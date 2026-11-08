#!/usr/bin/env python3
"""Validate Luanti's Arabic runtime and gettext catalogs without third-party packages."""

from __future__ import annotations

from collections import Counter
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
TEMPLATE = ROOT / "luanti/code/builtin/locale/template.txt"
SOURCE_TR = ROOT / "luanti/code/builtin/locale/__builtin.ar.tr"
BUNDLED_TR = ROOT / "luanti/5.17.0/builtin/locale/__builtin.ar.tr"
ENGINE_PO = ROOT / "luanti/code/po/ar/luanti.po"

# This is the current non-fuzzy, non-empty baseline. Fuzzy entries are not
# compiled by msgfmt and must never inflate the effective Arabic coverage.
MIN_ENGINE_TRANSLATIONS = 1575

TR_TOKENS = re.compile(r"@[0-9]+|@=|@n|<(?!empty>)[^<>]+>")
PO_TOKENS = re.compile(
    r"\$[0-9]+|@[0-9]+|@=|@n|%[-+#0-9.*hlLzjt]*[diouxXeEfFgGaAcspn%]"
)


def tr_entries(path: Path) -> tuple[dict[str, str], list[str]]:
    """Parse a Luanti .tr file, including its escaped equals/newline syntax."""
    entries: dict[str, str] = {}
    errors: list[str] = []

    for number, raw in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        if not raw or raw.startswith("#"):
            continue

        key: list[str] = []
        value: list[str] = []
        target = key
        separator_seen = False
        index = 0

        while index < len(raw):
            char = raw[index]
            if char == "@" and index + 1 < len(raw):
                escaped = raw[index + 1]
                if escaped == "=":
                    target.append("=")
                elif escaped == "n":
                    target.append("\n")
                else:
                    target.extend(("@", escaped))
                index += 2
                continue

            if char == "=" and not separator_seen:
                separator_seen = True
                target = value
                index += 1
                continue

            target.append(char)
            index += 1

        if not separator_seen:
            errors.append(
                f"{path.relative_to(ROOT)}:{number}: missing key/value separator"
            )
            continue

        decoded_key = "".join(key)
        decoded_value = "".join(value)
        if decoded_key in entries:
            errors.append(
                f"{path.relative_to(ROOT)}:{number}: duplicate key {decoded_key!r}"
            )
        entries[decoded_key] = decoded_value

    return entries, errors


def tokens(value: str, pattern: re.Pattern[str]) -> Counter[str]:
    return Counter(pattern.findall(value))


def parse_po(path: Path) -> tuple[list[dict[str, object]], list[str]]:
    """Parse the PO fields used by Luanti, including continuations and fuzzy flags."""
    entries: list[dict[str, object]] = []
    errors: list[str] = []
    current: dict[str, object] | None = None
    active_field: str | None = None
    pending_fuzzy = False

    def finish() -> None:
        nonlocal current, active_field
        if current is not None:
            entries.append(current)
        current = None
        active_field = None

    for number, line in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        if not line.strip():
            finish()
            continue

        if line.startswith("#"):
            if line.startswith("#,") and re.search(r"\bfuzzy\b", line):
                if current is None:
                    pending_fuzzy = True
                else:
                    current["fuzzy"] = True
            continue

        match = re.match(
            r"^(msgctxt|msgid_plural|msgid|msgstr(?:\[(\d+)\])?)\s+(.+)$",
            line,
        )
        if match:
            field, plural_index, quoted = match.groups()

            # A context may precede msgid in the same entry. A second msgid
            # without an intervening blank line starts the next entry.
            if field == "msgid" and current is not None and "msgid" in current:
                finish()

            if current is None:
                current = {
                    "msgid": "",
                    "msgstr": "",
                    "translations": {},
                    "fuzzy": pending_fuzzy,
                }
                pending_fuzzy = False

            try:
                decoded = json.loads(quoted)
            except json.JSONDecodeError as exc:
                errors.append(
                    f"{path.relative_to(ROOT)}:{number}: invalid PO string: {exc}"
                )
                continue

            if field == "msgctxt":
                current["msgctxt"] = decoded
            elif field == "msgid":
                current["msgid"] = decoded
            elif field == "msgid_plural":
                current["msgid_plural"] = decoded
            elif field == "msgstr":
                current["msgstr"] = decoded
            else:
                current["translations"][plural_index] = decoded

            active_field = field
            continue

        if line.lstrip().startswith('"') and current is not None and active_field:
            try:
                decoded = json.loads(line.strip())
            except json.JSONDecodeError as exc:
                errors.append(
                    f"{path.relative_to(ROOT)}:{number}: invalid continuation: {exc}"
                )
                continue

            if active_field == "msgctxt":
                current["msgctxt"] = current.get("msgctxt", "") + decoded
            elif active_field == "msgid":
                current["msgid"] += decoded
            elif active_field == "msgid_plural":
                current["msgid_plural"] = current.get("msgid_plural", "") + decoded
            elif active_field == "msgstr":
                current["msgstr"] += decoded
            elif active_field.startswith("msgstr["):
                plural_index = active_field[7:-1]
                current["translations"][plural_index] += decoded
            continue

        # Obsolete PO lines are comments beginning with #~ and were skipped
        # above. Anything else is a syntax error rather than silently ignored.
        errors.append(
            f"{path.relative_to(ROOT)}:{number}: unrecognized PO syntax: {line!r}"
        )

    finish()
    return entries, errors


def validate() -> int:
    errors: list[str] = []

    template, template_errors = tr_entries(TEMPLATE)
    source, source_errors = tr_entries(SOURCE_TR)
    bundled, bundled_errors = tr_entries(BUNDLED_TR)
    errors.extend(template_errors + source_errors + bundled_errors)

    if set(template) != set(source):
        missing = sorted(set(template) - set(source))
        extra = sorted(set(source) - set(template))
        errors.append(
            f"source __builtin.ar.tr keys differ from template: "
            f"missing={missing[:8]}, extra={extra[:8]}"
        )

    if set(template) != set(bundled):
        missing = sorted(set(template) - set(bundled))
        extra = sorted(set(bundled) - set(template))
        errors.append(
            f"bundled __builtin.ar.tr keys differ from template: "
            f"missing={missing[:8]}, extra={extra[:8]}"
        )

    if SOURCE_TR.read_bytes() != BUNDLED_TR.read_bytes():
        errors.append("source and bundled __builtin.ar.tr files are not byte-identical")

    empty_builtin = sorted(key for key in template if not source.get(key, "").strip())
    if empty_builtin:
        errors.append(
            f"{len(empty_builtin)} builtin translations are empty: {empty_builtin[:8]}"
        )

    builtin_placeholder_errors = []
    for key, value in source.items():
        if tokens(key, TR_TOKENS) != tokens(value, TR_TOKENS):
            builtin_placeholder_errors.append(key)
    if builtin_placeholder_errors:
        errors.append(
            "builtin placeholder mismatches: "
            f"{builtin_placeholder_errors[:8]}"
        )

    po_entries, po_errors = parse_po(ENGINE_PO)
    errors.extend(po_errors)

    header = next((entry for entry in po_entries if entry.get("msgid") == ""), {})
    metadata = str(header.get("msgstr", ""))
    if not re.search(r"(?m)^Language:\s*ar\s*$", metadata):
        errors.append("Arabic engine PO metadata must contain Language: ar")

    seen_ids: set[tuple[str, str]] = set()
    duplicate_ids: set[tuple[str, str]] = set()
    translated = 0
    untranslated = 0
    fuzzy = 0
    lang_code: str | None = None
    po_placeholder_errors: list[str] = []
    message_count = 0

    for entry in po_entries:
        msgid = str(entry.get("msgid", ""))
        if not msgid:
            continue

        message_count += 1
        identity = (str(entry.get("msgctxt", "")), msgid)
        if identity in seen_ids:
            duplicate_ids.add(identity)
        seen_ids.add(identity)

        if msgid == "LANG_CODE":
            lang_code = str(entry.get("msgstr", ""))

        if entry.get("fuzzy"):
            fuzzy += 1
            continue

        if entry.get("msgid_plural"):
            values = list(entry["translations"].values())
            expected_text = msgid + "\n" + str(entry["msgid_plural"])
        else:
            values = [str(entry.get("msgstr", ""))]
            expected_text = msgid

        if values and all(value.strip() for value in values):
            translated += 1
        else:
            untranslated += 1

        expected_tokens = tokens(expected_text, PO_TOKENS)
        for value in values:
            if not value.strip():
                continue
            if tokens(value, PO_TOKENS) != expected_tokens:
                po_placeholder_errors.append(msgid)
                break

    if duplicate_ids:
        errors.append(f"duplicate gettext entries: {list(duplicate_ids)[:8]}")
    if lang_code != "ar":
        errors.append(f'LANG_CODE must translate to "ar", got {lang_code!r}')
    if translated < MIN_ENGINE_TRANSLATIONS:
        errors.append(
            "effective Arabic translation count regressed: "
            f"{translated} < {MIN_ENGINE_TRANSLATIONS}"
        )
    if po_placeholder_errors:
        errors.append(
            "engine PO placeholder mismatches: "
            f"{po_placeholder_errors[:8]}"
        )

    print(
        f"Luanti builtin: {len(template)} template keys, {len(source)} Arabic keys, "
        f"{len(empty_builtin)} empty; packaged copy byte-identical."
    )
    print(
        f"Luanti gettext ar: {message_count} messages; {translated} effective "
        f"translations, {untranslated} untranslated, {fuzzy} fuzzy; "
        f"LANG_CODE={lang_code!r}."
    )

    if fuzzy:
        print(
            f"NOTICE: {fuzzy} fuzzy entries are excluded from effective coverage "
            "until reviewed."
        )

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        print(f"FAILED: {len(errors)} issue(s).", file=sys.stderr)
        return 1

    print(
        "PASS: builtin keys, translations, placeholders, bundle sync, "
        "PO metadata and non-fuzzy coverage floor."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(validate())

#!/usr/bin/env python3
"""Validate the Luanti Arabic runtime and gettext catalogs without third-party Python packages."""

from __future__ import annotations

from collections import Counter
import argparse
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
TEMPLATE = ROOT / "luanti/code/builtin/locale/template.txt"
SOURCE_TR = ROOT / "luanti/code/builtin/locale/__builtin.ar.tr"
BUNDLED_TR = ROOT / "luanti/5.17.0/builtin/locale/__builtin.ar.tr"
ENGINE_PO = ROOT / "luanti/code/po/ar/luanti.po"
MIN_ENGINE_TRANSLATIONS = 846

TR_TOKENS = re.compile(r"@[0-9]+|@=|@n|<(?!empty>)[^<>]+>")
PO_TOKENS = re.compile(
    r"\$[0-9]+|@[0-9]+|@=|@n|%[-+#0-9.*hlLzjt]*[diouxXeEfFgGaAcspn%]"
)


def tr_entries(path: Path) -> tuple[dict[str, str], list[str]]:
    """Read .tr lines using Luanti's @= and @n escape rules."""
    entries: dict[str, str] = {}
    errors: list[str] = []
    for number, raw in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        if not raw or raw.startswith("#"):
            continue
        key: list[str] = []
        value: list[str] = []
        target = key
        separator_seen = False
        i = 0
        while i < len(raw):
            char = raw[i]
            if char == "@":
                if i + 1 >= len(raw):
                    target.append("@")
                    i += 1
                    continue
                escaped = raw[i + 1]
                if escaped == "=":
                    target.append("=")
                elif escaped == "n":
                    target.append("\n")
                else:
                    target.extend(("@", escaped))
                i += 2
                continue
            if char == "=" and not separator_seen:
                separator_seen = True
                target = value
                i += 1
                continue
            target.append(char)
            i += 1
        if not separator_seen:
            errors.append(f"{path.relative_to(ROOT)}:{number}: missing key/value separator")
            continue
        k, v = "".join(key), "".join(value)
        if k in entries:
            errors.append(f"{path.relative_to(ROOT)}:{number}: duplicate key {k!r}")
        entries[k] = v
    return entries, errors


def tokens(value: str, pattern: re.Pattern[str]) -> Counter[str]:
    return Counter(pattern.findall(value))


def parse_po(path: Path) -> tuple[list[dict[str, object]], list[str]]:
    """Small strict PO reader sufficient for msgid/msgstr, plurals and continuations."""
    entries: list[dict[str, object]] = []
    errors: list[str] = []
    current: dict[str, object] | None = None
    active_field: str | None = None

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
            if current is not None and line.startswith("#,") and "fuzzy" in line:
                current["fuzzy"] = True
            continue

        field_match = re.match(r"^(msgctxt|msgid_plural|msgid|msgstr(?:\[\d+\])?)\s+(.*)$", line)
        if field_match:
            field, quoted = field_match.groups()
            if field == "msgid":
                if current is not None and "msgid" in current:
                    finish()
                if current is None:
                    current = {"msgid": "", "msgstr": "", "translations": {}, "fuzzy": False}
            elif current is None:
                errors.append(f"{path.relative_to(ROOT)}:{number}: {field} appears before msgid")
                continue
            try:
                decoded = json.loads(quoted)
            except json.JSONDecodeError as exc:
                errors.append(f"{path.relative_to(ROOT)}:{number}: invalid quoted PO string: {exc}")
                continue

            if field.startswith("msgstr["):
                current["translations"][field[7:-1]] = decoded
                active_field = field
            elif field == "msgstr":
                current["msgstr"] = decoded
                active_field = field
            elif field == "msgid":
                current["msgid"] = decoded
                active_field = field
            elif field == "msgid_plural":
                current["msgid_plural"] = decoded
                active_field = field
            else:
                current["msgctxt"] = decoded
                active_field = field
            continue

        if line.lstrip().startswith('"') and current is not None and active_field:
            try:
                decoded = json.loads(line.strip())
            except json.JSONDecodeError as exc:
                errors.append(f"{path.relative_to(ROOT)}:{number}: invalid continuation string: {exc}")
                continue
            if active_field == "msgid":
                current["msgid"] += decoded
            elif active_field == "msgid_plural":
                current["msgid_plural"] = current.get("msgid_plural", "") + decoded
            elif active_field == "msgstr":
                current["msgstr"] += decoded
            elif active_field.startswith("msgstr["):
                index = active_field[7:-1]
                current["translations"][index] += decoded
            elif active_field == "msgctxt":
                current["msgctxt"] = current.get("msgctxt", "") + decoded
            continue

        errors.append(f"{path.relative_to(ROOT)}:{number}: unrecognized PO syntax")
    finish()
    return entries, errors


def validate() -> int:
    errors: list[str] = []
    template, template_errors = tr_entries(TEMPLATE)
    source, source_errors = tr_entries(SOURCE_TR)
    bundled, bundled_errors = tr_entries(BUNDLED_TR)
    errors.extend(template_errors + source_errors + bundled_errors)

    if template != source:
        missing = sorted(set(template) - set(source))
        extra = sorted(set(source) - set(template))
        errors.append(f"source __builtin.ar.tr key mismatch: missing={missing[:8]}, extra={extra[:8]}")
    if template != bundled:
        missing = sorted(set(template) - set(bundled))
        extra = sorted(set(bundled) - set(template))
        errors.append(f"bundled __builtin.ar.tr key mismatch: missing={missing[:8]}, extra={extra[:8]}")
    if SOURCE_TR.read_bytes() != BUNDLED_TR.read_bytes():
        errors.append("source and bundled __builtin.ar.tr files are not byte-identical")

    empty_builtin = sorted(key for key in template if not source.get(key, "").strip())
    if empty_builtin:
        errors.append(f"{len(empty_builtin)} builtin translations are empty: {empty_builtin[:8]}")

    placeholder_mismatches = []
    for key, value in source.items():
        source_tokens = tokens(key, TR_TOKENS)
        target_tokens = tokens(value, TR_TOKENS)
        if source_tokens != target_tokens:
            placeholder_mismatches.append((key, dict(source_tokens), dict(target_tokens)))
    if placeholder_mismatches:
        errors.append(f"builtin placeholder mismatches: {placeholder_mismatches[:8]}")

    po_entries, po_errors = parse_po(ENGINE_PO)
    errors.extend(po_errors)
    duplicate_ids: set[tuple[str, str]] = set()
    seen_ids: set[tuple[str, str]] = set()
    translated = 0
    untranslated = 0
    fuzzy = 0
    lang_code = None
    po_placeholder_errors: list[tuple[str, dict[str, int], dict[str, int]]] = []

    for entry in po_entries:
        msgid = str(entry.get("msgid", ""))
        if not msgid:
            continue
        identity = (str(entry.get("msgctxt", "")), msgid)
        if identity in seen_ids:
            duplicate_ids.add(identity)
        seen_ids.add(identity)

        if entry.get("fuzzy"):
            fuzzy += 1
        if msgid == "LANG_CODE":
            lang_code = str(entry.get("msgstr", ""))

        translations = list(entry["translations"].values()) if entry.get("msgid_plural") else [str(entry.get("msgstr", ""))]
        if translations and all(value.strip() for value in translations):
            translated += 1
        else:
            untranslated += 1

        if not entry.get("fuzzy"):
            for value in translations:
                if not value.strip():
                    continue
                expected = tokens(msgid, PO_TOKENS)
                actual = tokens(value, PO_TOKENS)
                if expected != actual:
                    po_placeholder_errors.append((msgid, dict(expected), dict(actual)))

    if duplicate_ids:
        errors.append(f"duplicate gettext entries: {list(duplicate_ids)[:8]}")
    if lang_code != "ar":
        errors.append(f'LANG_CODE must translate to "ar", got {lang_code!r}')
    if translated < MIN_ENGINE_TRANSLATIONS:
        errors.append(
            f"engine Arabic translation count regressed: {translated} < {MIN_ENGINE_TRANSLATIONS}"
        )
    if po_placeholder_errors:
        errors.append(f"engine PO placeholder mismatches: {po_placeholder_errors[:8]}")

    print(
        f"Luanti builtin: {len(template)} source keys, {len(source)} Arabic keys, "
        f"{len(empty_builtin)} empty; packaged copy byte-identical."
    )
    print(
        f"Luanti gettext ar: {len(seen_ids)} messages, {translated} translated, "
        f"{untranslated} untranslated, {fuzzy} fuzzy; LANG_CODE={lang_code!r}."
    )
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        print(f"FAILED: {len(errors)} issue(s).", file=sys.stderr)
        return 1
    print("PASS: builtin keys, translations, placeholders, bundle sync, PO metadata and coverage floor.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.parse_args()
    return validate()


if __name__ == "__main__":
    raise SystemExit(main())

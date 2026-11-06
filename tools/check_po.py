#!/usr/bin/env python3
"""Validate Arabic PO files, required translations, and Luanti placeholders."""

from __future__ import annotations

from collections import Counter
from pathlib import Path
import re
import sys

import polib

ROOT = Path(__file__).resolve().parents[1]
PO_FILES = sorted((ROOT / "mods").rglob("*.ar.po"))
TOKEN_RE = re.compile(r"@[0-9]+")


def tokens(value: str) -> Counter[str]:
    return Counter(TOKEN_RE.findall(value))


def main() -> int:
    if not PO_FILES:
        print("ERROR: no *.ar.po files found under mods/", file=sys.stderr)
        return 1

    errors: list[str] = []
    checked = 0
    untranslated = 0

    for path in PO_FILES:
        try:
            po = polib.pofile(str(path), check_for_duplicates=True)
        except Exception as exc:
            errors.append(f"{path.relative_to(ROOT)}: cannot parse PO: {exc}")
            continue

        if po.metadata.get("Language") != "ar":
            errors.append(f"{path.relative_to(ROOT)}: header Language must be ar")

        for entry in po:
            if entry.obsolete or not entry.msgid or not entry.msgid.strip():
                continue
            checked += 1
            if "fuzzy" in entry.flags:
                errors.append(f"{path.relative_to(ROOT)}: fuzzy entry: {entry.msgid!r}")
                continue

            expected = tokens(entry.msgid)
            values = (
                list(entry.msgstr_plural.values())
                if entry.msgid_plural
                else [entry.msgstr]
            )
            if not values or any(not value.strip() for value in values):
                untranslated += 1
                errors.append(f"{path.relative_to(ROOT)}: empty translation: {entry.msgid!r}")
                continue

            for value in values:
                actual = tokens(value)
                if actual != expected:
                    errors.append(
                        f"{path.relative_to(ROOT)}: placeholder mismatch for {entry.msgid!r}; "
                        f"source={dict(expected)}, target={dict(actual)}"
                    )

    print(
        f"Checked {len(PO_FILES)} files and {checked} messages; "
        f"{untranslated} untranslated message(s)."
    )
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        print(f"FAILED: {len(errors)} issue(s).", file=sys.stderr)
        return 1

    print("PASS: PO syntax, Arabic metadata, complete translations, and placeholders.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Validate Mineclonia Arabic PO files and Luanti numbered placeholders."""

from __future__ import annotations

from collections import Counter
from pathlib import Path
import re
import sys

import polib

ROOT = Path(__file__).resolve().parents[1]
PO_FILES = sorted((ROOT / "mods").rglob("*.ar.po"))
PLACEHOLDER_RE = re.compile(r"(?<!@)@[1-9]")


def placeholders(value: str) -> Counter[str]:
    return Counter(PLACEHOLDER_RE.findall(value))


def main() -> int:
    if not PO_FILES:
        print("No Arabic PO files found under mods/.")
        return 0

    errors: list[str] = []
    untranslated = 0
    checked = 0

    for path in PO_FILES:
        try:
            po = polib.pofile(str(path), check_for_duplicates=True)
        except Exception as exc:  # polib raises several parse/validation errors
            errors.append(f"{path.relative_to(ROOT)}: cannot parse PO: {exc}")
            continue

        language = po.metadata.get("Language", "")
        if language != "ar":
            errors.append(
                f"{path.relative_to(ROOT)}: expected Language: ar, got {language!r}"
            )

        for entry in po:
            if entry.obsolete:
                continue
            checked += 1
            source_tokens = placeholders(entry.msgid)

            translated_values = (
                list(entry.msgstr_plural.values())
                if entry.msgid_plural
                else [entry.msgstr]
            )
            for translated in translated_values:
                if not translated:
                    untranslated += 1
                    continue
                if placeholders(translated) != source_tokens:
                    errors.append(
                        f"{path.relative_to(ROOT)}: placeholder mismatch for "
                        f"{entry.msgid!r}: source={dict(source_tokens)}, "
                        f"translation={dict(placeholders(translated))}"
                    )

    print(
        f"Checked {len(PO_FILES)} Arabic PO file(s), "
        f"{checked} message(s), {untranslated} untranslated form(s)."
    )
    if errors:
        print("\n".join(f"ERROR: {item}" for item in errors), file=sys.stderr)
        return 1

    print("PO parsing, language metadata, and numbered placeholders: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

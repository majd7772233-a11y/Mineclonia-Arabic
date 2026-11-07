#!/usr/bin/env python3
"""Luanti TR serializer/checker fallback.

This is NOT Wuzzy's LTT implementation. It writes and validates the legacy
Luanti translation-file syntax used by Mineclonia, while the official LTT
tools are temporarily unavailable. It intentionally supports only PO -> TR
and syntax checking; it does not replace the upstream maintenance workflow.
"""

from __future__ import annotations

import argparse
from collections import Counter
from pathlib import Path
import re
import sys

import polib

PLACEHOLDER = re.compile(r"@[1-9]")


def die(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")


def escape_tr(value: str, where: str) -> str:
    """Escape a UTF-8 text value for the Luanti .tr key=value format."""
    out: list[str] = []
    i = 0
    while i < len(value):
        char = value[i]
        if char == "\n":
            out.append("@n")
            i += 1
        elif char == "=":
            out.append("@=")
            i += 1
        elif char == "@":
            if i + 1 < len(value) and value[i + 1] in "123456789":
                out.append(char + value[i + 1])
                i += 2
            else:
                out.append("@@")
                i += 1
        elif char == "\r":
            # Normalize CRLF/CR to the Luanti newline escape.
            if i + 1 < len(value) and value[i + 1] == "\n":
                i += 1
            out.append("@n")
            i += 1
        else:
            out.append(char)
            i += 1
    result = "".join(out)
    if "\x00" in result:
        die(f"{where}: NUL bytes are not supported in TR strings")
    return result


def decode_tr(value: str, where: str) -> str:
    out: list[str] = []
    i = 0
    while i < len(value):
        char = value[i]
        if char != "@":
            out.append(char)
            i += 1
            continue
        if i + 1 >= len(value):
            die(f"{where}: dangling @ escape")
        nxt = value[i + 1]
        if nxt == "@":
            out.append("@")
        elif nxt == "=":
            out.append("=")
        elif nxt == "n":
            out.append("\n")
        elif nxt in "123456789":
            out.append("@" + nxt)
        else:
            die(f"{where}: invalid Luanti escape @{nxt}")
        i += 2
    return "".join(out)


def find_separator(line: str, where: str) -> int:
    i = 0
    while i < len(line):
        if line[i] == "@":
            if i + 1 >= len(line):
                die(f"{where}: dangling @ escape")
            i += 2
        elif line[i] == "=":
            return i
        else:
            i += 1
    die(f"{where}: translation line has no unescaped '='")
    return -1


def read_textdomain(template: Path) -> str:
    try:
        text = template.read_text(encoding="utf-8-sig")
    except Exception as exc:
        die(f"{template}: cannot read runtime template: {exc}")
    for line in text.splitlines():
        if line.startswith("# textdomain:"):
            domain = line.split(":", 1)[1].strip()
            if not domain:
                die(f"{template}: empty textdomain")
            return domain
    die(f"{template}: missing '# textdomain: <domain>' header")
    return ""


def convert_one(po_path: Path) -> Path:
    locale = po_path.parent.parent
    template = locale / "template.txt"
    if not template.is_file():
        die(f"{po_path}: missing matching template.txt")
    domain = read_textdomain(template)
    try:
        po = polib.pofile(str(po_path), check_for_duplicates=True)
    except Exception as exc:
        die(f"{po_path}: PO parser error: {exc}")
    if po.metadata.get("Language") != "ar":
        die(f"{po_path}: expected Language: ar")
    lines = [f"# textdomain: {domain}"]
    used: set[str] = set()
    for entry in po:
        if entry.obsolete or entry.msgid == "":
            continue
        if entry.msgctxt:
            die(f"{po_path}: msgctxt is not supported by this fallback converter")
        if entry.msgid_plural:
            die(f"{po_path}: TR does not support gettext plural entries: {entry.msgid!r}")
        if "fuzzy" in entry.flags:
            die(f"{po_path}: fuzzy translation: {entry.msgid!r}")
        if entry.msgid in used:
            die(f"{po_path}: duplicate msgid: {entry.msgid!r}")
        used.add(entry.msgid)
        if not entry.msgstr.strip() and not (entry.msgid.isspace() and entry.msgstr == entry.msgid):
            die(f"{po_path}: empty msgstr: {entry.msgid!r}")
        source = escape_tr(entry.msgid, f"{po_path}: source")
        translation = escape_tr(entry.msgstr, f"{po_path}: translation")
        if source.startswith("#"):
            die(f"{po_path}: source strings beginning with '#' cannot be represented safely in .tr")
        lines.append(f"{source}={translation}")
    output_name = po_path.name[:-3] + "tr"
    output_path = locale / output_name
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"CONVERTED {po_path} -> {output_path} ({len(used)} entries)")
    return output_path


def check_tr_file(path: Path) -> int:
    if path.parent.name != "locale":
        die(f"{path}: TR files must be stored in a locale directory")
    if not path.name.endswith(".ar.tr"):
        die(f"{path}: expected a lowercase .ar.tr suffix")
    try:
        raw = path.read_text(encoding="utf-8-sig")
    except Exception as exc:
        die(f"{path}: cannot read UTF-8 TR file: {exc}")
    lines = raw.splitlines()
    if not lines:
        die(f"{path}: empty file")
    if not lines[0].startswith("# textdomain:"):
        die(f"{path}: first line must declare '# textdomain: <domain>'")
    domain = lines[0].split(":", 1)[1].strip()
    if not domain:
        die(f"{path}: empty textdomain")
    seen: set[str] = set()
    count = 0
    for line_no, line in enumerate(lines[1:], 2):
        if not line or line.startswith("#"):
            continue
        where = f"{path}:{line_no}"
        sep = find_separator(line, where)
        source = decode_tr(line[:sep], where + " source")
        translated = decode_tr(line[sep + 1:], where + " translation")
        if source in seen:
            die(f"{where}: duplicate source string: {source!r}")
        seen.add(source)
        source_placeholders = PLACEHOLDER.findall(source)
        target_placeholders = PLACEHOLDER.findall(translated)
        if len(source_placeholders) != len(set(source_placeholders)):
            die(f"{where}: repeated placeholder in source string")
        if source_placeholders != sorted(source_placeholders, key=lambda x: int(x[1])):
            die(f"{where}: placeholders are out of numeric order")
        if Counter(source_placeholders) != Counter(target_placeholders):
            die(
                f"{where}: placeholder mismatch; "
                f"source={source_placeholders}, translation={target_placeholders}"
            )
        count += 1
    print(f"CHECKED {path}: domain={domain}, {count} entries")
    return count


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", type=Path, help="mods directory or a translation file")
    parser.add_argument("--po2tr", action="store_true", help="convert .po files in locale/poconvert to .ar.tr")
    parser.add_argument("-r", "--recursive", action="store_true", help="recursively scan subdirectories")
    parser.add_argument("--check", action="store_true", help="check .tr files; default mode if --po2tr is absent")
    args = parser.parse_args()

    root = args.path.resolve()
    if not root.exists():
        die(f"path does not exist: {root}")
    if args.po2tr:
        po_files = sorted(root.rglob("poconvert/*.ar.po")) if args.recursive else sorted(root.glob("*/locale/poconvert/*.ar.po"))
        if not po_files:
            die(f"no .ar.po files found under {root}")
        outputs = [convert_one(path) for path in po_files]
        print(f"Converted {len(outputs)} Arabic PO files to TR.")
    else:
        tr_files = sorted(root.rglob("*.ar.tr")) if args.recursive else sorted(root.glob("*/locale/*.ar.tr"))
        if not tr_files:
            die(f"no .ar.tr files found under {root}")
        total = sum(check_tr_file(path) for path in tr_files)
        print(f"Checked {len(tr_files)} Arabic TR files and {total} entries; 0 errors.")


if __name__ == "__main__":
    main()

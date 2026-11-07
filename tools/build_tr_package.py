#!/usr/bin/env python3
"""Build an installable Mineclonia Arabic TR overlay using the official MTT tools."""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile

import polib

PLACEHOLDER_RE = re.compile(r"@[1-9]")
ANY_NUMERIC_AT_RE = re.compile(r"@[0-9]+")


def fail(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")


def meaningful_entries(po: polib.POFile):
    return [
        entry for entry in po
        if not entry.obsolete and entry.msgid and entry.msgid.strip()
    ]


def decode_template_key(raw: str, where: str) -> str:
    """Decode the escapes used by Luanti template.txt and legacy TR files."""
    result: list[str] = []
    i = 0
    while i < len(raw):
        char = raw[i]
        if char != "@":
            result.append(char)
            i += 1
            continue
        if i + 1 >= len(raw):
            fail(f"{where}: dangling @ escape in template key")
        nxt = raw[i + 1]
        if nxt == "@":
            result.append("@")
        elif nxt == "=":
            result.append("=")
        elif nxt == "n":
            result.append("\n")
        elif nxt in "123456789":
            result.append("@" + nxt)
        else:
            fail(f"{where}: unsupported template escape @{nxt}")
        i += 2
    return "".join(result)


def find_template_separator(line: str, where: str) -> int:
    i = 0
    while i < len(line):
        if line[i] == "@":
            if i + 1 >= len(line):
                fail(f"{where}: dangling @ escape")
            i += 2
        elif line[i] == "=":
            return i
        else:
            i += 1
    fail(f"{where}: template entry has no unescaped '='")
    return -1


def parse_template_txt(path: Path) -> tuple[str, list[str]]:
    text = path.read_text(encoding="utf-8-sig")
    domain = ""
    keys: list[str] = []
    for line_no, line in enumerate(text.splitlines(), 1):
        if not line or line.startswith("#"):
            if line.startswith("# textdomain:"):
                domain = line.split(":", 1)[1].strip()
            continue
        where = f"{path}:{line_no}"
        separator = find_template_separator(line, where)
        key = decode_template_key(line[:separator], where)
        if not key:
            fail(f"{where}: empty source string")
        keys.append(key)
    if not domain:
        fail(f"{path}: missing '# textdomain: <domain>' header")
    return domain, keys


def check_placeholder_pair(source: str, translated: str, where: str) -> None:
    source_all = ANY_NUMERIC_AT_RE.findall(source)
    target_all = ANY_NUMERIC_AT_RE.findall(translated)
    invalid = [token for token in source_all if not PLACEHOLDER_RE.fullmatch(token)]
    if invalid:
        fail(f"{where}: unsupported Luanti placeholder(s): {invalid}")
    if Counter(source_all) != Counter(target_all):
        fail(
            f"{where}: placeholder mismatch; "
            f"source={source_all}, translation={target_all}"
        )


def run_checked(command: list[str], cwd: Path, description: str) -> str:
    print(f"$ {' '.join(command)}  (cwd={cwd})", flush=True)
    result = subprocess.run(
        command, cwd=cwd, text=True, encoding="utf-8",
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    if result.stdout:
        print(result.stdout, flush=True)
    if result.returncode != 0:
        fail(f"{description} failed with exit code {result.returncode}")
    return result.stdout


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--mtt-convert", required=True, type=Path)
    parser.add_argument("--mtt-check", required=True, type=Path)
    parser.add_argument("--source-commit", required=True)
    parser.add_argument("--expected-files", type=int, default=163)
    parser.add_argument("--expected-messages", type=int, default=4980)
    parser.add_argument("--target-release", default="39603 (owner-reported; exact source match unverified)")
    parser.add_argument("--luanti-version", default="5.17.0 (owner-reported)")
    parser.add_argument("--output", type=Path, default=Path("dist"))
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    mods_root = repo_root / "mods"
    source_root = args.source_root.resolve()
    if not source_root.is_dir():
        fail(f"source checkout not found: {source_root}")
    if not args.mtt_convert.is_file() or not args.mtt_check.is_file():
        fail("official MTT converter/checker scripts were not found")

    po_files = sorted(mods_root.rglob("*.ar.po"))
    if len(po_files) != args.expected_files:
        fail(f"expected {args.expected_files} Arabic PO files, found {len(po_files)}")

    stage_root = args.output.resolve() / "_stage"
    if stage_root.exists():
        shutil.rmtree(stage_root)
    stage_mods = stage_root / "mods"
    stage_mods.mkdir(parents=True, exist_ok=True)

    per_module: list[dict[str, object]] = []
    total_messages = 0
    seen_domains: dict[str, str] = {}

    for po_path in po_files:
        rel = po_path.relative_to(repo_root)
        source_locale = source_root / rel.parent
        pot_path = source_locale / "template.pot"
        txt_path = source_locale / "template.txt"
        if not pot_path.is_file():
            fail(f"{rel}: matching source template.pot is missing")
        if not txt_path.is_file():
            fail(f"{rel}: matching source template.txt is missing")

        try:
            po = polib.pofile(str(po_path), check_for_duplicates=True)
            pot = polib.pofile(str(pot_path), check_for_duplicates=True)
        except Exception as exc:
            fail(f"{rel}: PO/POT parsing failed: {exc}")

        if po.metadata.get("Language") != "ar":
            fail(f"{rel}: PO Language header must be ar")

        entries = meaningful_entries(po)
        source_entries = meaningful_entries(pot)
        if any(entry.msgctxt for entry in entries):
            fail(f"{rel}: msgctxt requires a separate textdomain mapping; none expected here")
        plural_entries = [entry.msgid for entry in entries if entry.msgid_plural]
        if plural_entries:
            fail(
                f"{rel}: TR does not support gettext plurals; "
                f"found {len(plural_entries)} plural entr(y/ies), e.g. {plural_entries[:3]}"
            )
        if any("fuzzy" in entry.flags for entry in entries):
            fail(f"{rel}: contains fuzzy entries")

        po_ids = [entry.msgid for entry in entries]
        pot_ids = [entry.msgid for entry in source_entries]
        if Counter(po_ids) != Counter(pot_ids):
            missing = list((Counter(pot_ids) - Counter(po_ids)).elements())[:5]
            extra = list((Counter(po_ids) - Counter(pot_ids)).elements())[:5]
            fail(f"{rel}: PO differs from pinned POT; missing={missing!r}; extra={extra!r}")

        domain, template_ids = parse_template_txt(txt_path)
        if Counter(template_ids) != Counter(pot_ids):
            missing = list((Counter(template_ids) - Counter(pot_ids)).elements())[:5]
            extra = list((Counter(pot_ids) - Counter(template_ids)).elements())[:5]
            fail(
                f"{rel}: template.txt does not match POT; "
                f"untranslated/computed={missing!r}; absent-from-template={extra!r}"
            )
        previous = seen_domains.get(domain)
        if previous and previous != str(rel.parent):
            fail(f"textdomain collision: {domain!r} belongs to both {previous} and {rel.parent}")
        seen_domains[domain] = str(rel.parent)

        for entry in entries:
            if not entry.msgstr.strip():
                fail(f"{rel}: empty translation for {entry.msgid!r}")
            check_placeholder_pair(entry.msgid, entry.msgstr, f"{rel}: {entry.msgid!r}")

        locale_stage = stage_mods / rel.parent
        locale_stage.mkdir(parents=True, exist_ok=True)
        shutil.copy2(txt_path, locale_stage / "template.txt")
        poconvert = locale_stage / "poconvert"
        poconvert.mkdir(exist_ok=True)
        shutil.copy2(po_path, poconvert / po_path.name)

        module = rel.parent.parent.name
        message_count = len(entries)
        total_messages += message_count
        per_module.append({
            "module": module,
            "path": str(rel.parent),
            "textdomain": domain,
            "messages": message_count,
        })
        print(f"VALID {module}: {message_count} messages, textdomain={domain}", flush=True)

    if total_messages != args.expected_messages:
        fail(f"expected {args.expected_messages} messages, found {total_messages}")

    # Official Mineclonia workflow: PO files live in locale/poconvert; MTT
    # writes the runtime .tr files into each locale directory.
    run_checked(
        [sys.executable, str(args.mtt_convert.resolve()), "--po2tr", "-r"],
        cwd=stage_mods,
        description="mtt_convert.py --po2tr -r",
    )
    run_checked(
        [sys.executable, str(args.mtt_check.resolve()), "-r"],
        cwd=stage_mods,
        description="mtt_check.py -r",
    )

    tr_files = sorted(stage_mods.rglob("*.ar.tr"))
    if len(tr_files) != args.expected_files:
        fail(f"expected {args.expected_files} generated .ar.tr files, found {len(tr_files)}")

    package_root = args.output.resolve() / "Mineclonia-Arabic-TR"
    if package_root.exists():
        shutil.rmtree(package_root)
    package_root.mkdir(parents=True, exist_ok=True)

    installed_messages = 0
    tr_manifest: list[dict[str, object]] = []
    for tr_path in tr_files:
        rel = tr_path.relative_to(stage_mods)
        text = tr_path.read_text(encoding="utf-8-sig")
        entries = [
            line for line in text.splitlines()
            if line and not line.startswith("#")
        ]
        installed_messages += len(entries)
        destination = package_root / "mods" / rel
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(tr_path, destination)
        tr_manifest.append({
            "path": str(Path("mods") / rel),
            "entries": len(entries),
            "sha256": sha256_file(tr_path),
        })

    if installed_messages != total_messages:
        fail(
            f"generated TR entry count mismatch: expected {total_messages}, "
            f"found {installed_messages}"
        )

    source_readme = source_root / "README.md"
    source_version = "unknown"
    if source_readme.is_file():
        version_match = re.search(
            r"(?m)^Version:\s*([^\r\n]+)", source_readme.read_text(encoding="utf-8")
        )
        if version_match:
            source_version = version_match.group(1).strip()

    install_text = f"""حزمة التعريب العربي لـ Mineclonia
==================================

المحتويات
---------
- ملفات تشغيل Luanti بصيغة .tr فقط.
- {len(tr_files)} ملف ترجمة موزعة على مجلدات mods/*/*/locale/.
- {total_messages} مدخلاً مترجماً.
- مصدر القوالب: ZenonSeth/mineclonia-mirror
- commit المصدر: {args.source_commit}
- إصدار Mineclonia الظاهر في README للمصدر: {source_version}
- إصدار اللعبة المحلي الذي أبلغ عنه صاحب المشروع: {args.target_release}
- إصدار Luanti المحلي الذي أبلغ عنه صاحب المشروع: {args.luanti_version}

التثبيت على Windows
-------------------
1. أغلق Luanti بالكامل.
2. افتح مجلد لعبة Mineclonia الذي يحتوي على game.conf.
3. استخرج محتويات هذا الأرشيف إلى ذلك المجلد، واسمح بدمج مجلد mods.
4. لا تستبدل مجلد mods بالكامل؛ الحزمة تضيف ملفات locale/*.ar.tr فقط.
5. افتح Luanti، واختر العربية من إعدادات اللغة، ثم أعد تشغيل اللعبة.

إزالة التعريب
-------------
احذف ملفات *.ar.tr التي أضافتها هذه الحزمة من مجلدات mods/*/locale/.

ملاحظات التوافق
---------------
هذه الحزمة مبنية من جميع قوالب template.pot وtemplate.txt في لقطة المصدر
المثبتة أعلاه. لم تتم مطابقة شجرة ملفات اللعبة المحلية مع تلك اللقطة بصورة
مباشرة، ولذلك لا يمكن ضمان التطابق التام مع رقم release المحلي حتى تُفحص
نسخة اللعبة نفسها. الملفات التي لا يقابلها textdomain داخل اللعبة لن تؤثر.

لم يتم اختبار العرض داخل عميل Luanti فعلياً ضمن عملية البناء هذه.
"""
    (package_root / "INSTALL_ARABIC.txt").write_text(install_text, encoding="utf-8")

    manifest = {
        "name": "Mineclonia Arabic runtime translation overlay",
        "language": "ar",
        "format": "Luanti legacy TR",
        "source_repository": "https://github.com/ZenonSeth/mineclonia-mirror",
        "source_commit": args.source_commit,
        "source_version": source_version,
        "target_game_release_owner_reported": args.target_release,
        "luanti_version_owner_reported": args.luanti_version,
        "po_files": len(po_files),
        "tr_files": len(tr_files),
        "translated_messages": total_messages,
        "conversion": "Wuzzy Minetest Translation Tools: mtt_convert.py --po2tr -r",
        "validation": "Wuzzy Minetest Translation Tools: mtt_check.py -r",
        "runtime_tested": False,
        "official_weblate_merged": False,
        "modules": per_module,
        "files": tr_manifest,
    }
    (package_root / "ARABIC_TRANSLATION_MANIFEST.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    args.output.resolve().mkdir(parents=True, exist_ok=True)
    zip_path = args.output.resolve() / f"Mineclonia-Arabic-TR-{args.source_commit[:8]}.zip"
    if zip_path.exists():
        zip_path.unlink()
    with ZipFile(zip_path, "w", compression=ZIP_DEFLATED, compresslevel=9) as archive:
        for path in sorted(package_root.rglob("*")):
            if path.is_file():
                archive.write(path, path.relative_to(package_root))

    checksum = sha256_file(zip_path)
    checksum_path = args.output.resolve() / (zip_path.name + ".sha256")
    checksum_path.write_text(f"{checksum}  {zip_path.name}\n", encoding="ascii")

    shutil.rmtree(stage_root, ignore_errors=True)
    print(json.dumps({
        "status": "success",
        "po_files": len(po_files),
        "tr_files": len(tr_files),
        "messages": total_messages,
        "source_version": source_version,
        "package": str(zip_path),
        "sha256": checksum,
    }, ensure_ascii=False, indent=2), flush=True)


if __name__ == "__main__":
    main()

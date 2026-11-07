#!/usr/bin/env python3
"""Validate the Arabic PO set against a pinned Mineclonia checkout and package runtime translations."""

from __future__ import annotations

import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import shutil
import tempfile
import zipfile

import polib

ROOT = Path(__file__).resolve().parents[1]
PO_ROOT = ROOT / "mods"
PLACEHOLDER_RE = re.compile(r"(?<!@)@[1-9]|%(?:\([^)]+\))?[sdif]")
EXPECTED_SOURCE_COMMIT = "4ba113d1ce7afbc5b480b5b0e41b6c0037c8d8da"


def tokens(value: str) -> Counter[str]:
    return Counter(PLACEHOLDER_RE.findall(value))


def active_entries(po: polib.POFile):
    return [entry for entry in po if not entry.obsolete and entry.msgid and entry.msgid.strip()]


def get_textdomain(path: Path) -> str:
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("# textdomain:"):
            return line.partition(":")[2].strip()
    return ""


def prepare(source_root: Path, plan_path: Path, source_commit: str) -> None:
    if source_commit != EXPECTED_SOURCE_COMMIT:
        raise SystemExit(f"Unexpected source commit: {source_commit}")

    po_files = sorted(PO_ROOT.rglob("*.ar.po"))
    if len(po_files) != 163:
        raise SystemExit(f"Expected 163 Arabic PO files, found {len(po_files)}")

    plan = {
        "source_commit": source_commit,
        "po_file_count": len(po_files),
        "message_count": 0,
        "stale_pot_keys": 0,
        "po_extra_keys": 0,
        "tr_files": [],
        "po_fallbacks": [],
    }
    errors: list[str] = []

    for po_path in po_files:
        relative = po_path.relative_to(ROOT)
        source_mod = source_root / relative.parent
        pot_path = source_mod / "template.pot"
        template_txt = source_mod / "template.txt"

        if not pot_path.is_file() or not template_txt.is_file():
            errors.append(f"{relative}: source POT or template.txt is missing")
            continue

        module_domain = po_path.name.removesuffix(".ar.po")
        source_domain = get_textdomain(template_txt)
        if not source_domain or source_domain != module_domain:
            errors.append(
                f"{relative}: PO filename domain {module_domain!r} "
                f"does not match template domain {source_domain!r}"
            )
            continue

        try:
            po = polib.pofile(str(po_path), encoding="utf-8", check_for_duplicates=True)
            pot = polib.pofile(str(pot_path), encoding="utf-8")
        except Exception as exc:
            errors.append(f"{relative}: PO/POT parse error: {exc}")
            continue

        if po.metadata.get("Language") != "ar":
            errors.append(f"{relative}: Language metadata must be ar")

        source_entries = active_entries(pot)
        translated_entries = active_entries(po)
        source_keys = {(e.msgctxt, e.msgid, e.msgid_plural) for e in source_entries}
        translated_keys = {(e.msgctxt, e.msgid, e.msgid_plural) for e in translated_entries}

        # Mineclonia's checked-in POT/template.txt can lag behind its current Lua
        # strings. A POT key is required only while its literal remains in Lua.
        lua_text = "\n".join(
            path.read_text(encoding="utf-8", errors="replace")
            for path in source_mod.rglob("*.lua")
        )

        def still_used(key):
            _, msgid, plural = key
            for value in (msgid, plural):
                if not value:
                    continue
                variants = (
                    value,
                    value.replace(chr(10), chr(92) + "n"),
                    value.replace('"', chr(92) + '"'),
                )
                if any(candidate and candidate in lua_text for candidate in variants):
                    return True
            return False

        missing_active = [key for key in source_keys - translated_keys if still_used(key)]
        if missing_active:
            errors.append(
                f"{relative}: {len(missing_active)} strings still present in Lua are missing from PO"
            )
            continue

        plan["stale_pot_keys"] += len(source_keys - translated_keys)
        plan["po_extra_keys"] += len(translated_keys - source_keys)

        needs_po = False
        for entry in translated_entries:
            if "fuzzy" in entry.flags:
                errors.append(f"{relative}: fuzzy entry {entry.msgid!r}")
                continue

            values = list(entry.msgstr_plural.values()) if entry.msgid_plural else [entry.msgstr]
            if not values or any(not value.strip() for value in values):
                errors.append(f"{relative}: empty translation for {entry.msgid!r}")
                continue

            reference = entry.msgid_plural if entry.msgid_plural else entry.msgid
            expected = tokens(reference)
            for value in values:
                if tokens(value) != expected:
                    errors.append(f"{relative}: placeholder mismatch for {entry.msgid!r}")

            if entry.msgid_plural or entry.msgctxt:
                needs_po = True

        plan["message_count"] += len(translated_entries)
        staged_po = source_mod / po_path.name
        if not staged_po.is_file():
            errors.append(f"{relative}: staged PO is missing from source checkout")
            continue

        if needs_po:
            plan["po_fallbacks"].append({
                "source_path": relative.as_posix(),
                "runtime_path": (relative.parent / f"{source_domain}.ar.po").as_posix(),
                "reason": "TR cannot preserve gettext plural forms or per-entry contexts",
            })
            staged_po.unlink()
        else:
            plan["tr_files"].append(relative.with_suffix(".tr").as_posix())

    if errors:
        raise SystemExit("SOURCE VALIDATION FAILED:\n" + "\n".join(errors[:120]))

    if len(plan["tr_files"]) + len(plan["po_fallbacks"]) != len(po_files):
        raise SystemExit("Runtime format plan does not cover all PO files")

    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(json.dumps(plan, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(
        f"PASS: {len(po_files)} PO files match pinned POT templates; "
        f"{plan['message_count']} messages checked; "
        f"{len(plan['tr_files'])} TR and {len(plan['po_fallbacks'])} PO runtime files; "
        f"{plan['stale_pot_keys']} stale POT keys retained, {plan['po_extra_keys']} PO-only keys preserved."
    )


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def package(source_root: Path, plan_path: Path, output_path: Path, source_commit: str) -> None:
    plan = json.loads(plan_path.read_text(encoding="utf-8"))
    if plan.get("source_commit") != source_commit:
        raise SystemExit("Plan/source commit mismatch")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="mineclonia-arabic-") as temp:
        stage = Path(temp)
        file_records = []

        for relative in plan["tr_files"]:
            source_file = source_root / relative
            if not source_file.is_file():
                raise SystemExit(f"Generated TR file is missing: {relative}")
            target = stage / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_file, target)
            file_records.append({
                "path": relative,
                "format": "tr",
                "size_bytes": target.stat().st_size,
                "sha256": digest(target),
            })

        for item in plan["po_fallbacks"]:
            source_file = source_root / item["source_path"]
            if not source_file.is_file():
                raise SystemExit(f"PO fallback is missing: {item['source_path']}")
            target = stage / item["runtime_path"]
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_file, target)
            file_records.append({
                "path": item["runtime_path"],
                "format": "po",
                "reason": item["reason"],
                "size_bytes": target.stat().st_size,
                "sha256": digest(target),
            })

        if len(file_records) != plan["po_file_count"]:
            raise SystemExit(
                f"Expected {plan['po_file_count']} runtime files, found {len(file_records)}"
            )

        install = f"""# حزمة التعريب العربي لـ Mineclonia

هذه الحزمة طبقة ترجمة فقط، ولا تحتوي على ملفات اللعبة أو ملفات العالم.

## التثبيت

1. أغلق Luanti.
2. فك ضغط الأرشيف في مجلد لعبة Mineclonia نفسه، بحيث يندمج مجلد mods الموجود في الأرشيف مع مجلد mods داخل اللعبة.
3. وافق على دمج المجلدات عند الطلب. لا تحذف ملفات اللعبة الموجودة ولا تستبدل ملفات Lua أو إعدادات اللعبة.
4. شغّل Luanti واختر العربية من إعدادات اللغة، ثم أعد تشغيل اللعبة.

## التوافق والمصدر

- مصدر قوالب الترجمة: Mineclonia mirror commit {source_commit}.
- نسخة Mineclonia الموصوفة في لقطة المصدر: 0.123.0.
- ملفات TR أُنتجت بواسطة أداة Mineclonia Translation Tools (mtt_convert.py --po2tr -r) ثم فُحصت بواسطة mtt_check.py -r.
- أي وحدة لا يمكن تمثيل صيغ جمعها أو سياقاتها بأمان في TR تُضمّن بملف PO الأصلي؛ يتطلب دعم PO محرك Luanti 5.10.0 أو أحدث.
- لم تُختبر الحزمة على جهاز المستخدم داخل اللعبة. يجب التحقق بصرياً من القوائم والأسماء والحوارات في نسخة اللعبة الفعلية.
- هذه الحزمة لا تعني اعتماد الترجمة أو دمجها في Weblate الرسمي.

راجع manifest.json لبصمات SHA-256 وقائمة الملفات.
"""
        (stage / "INSTALL-AR.md").write_text(install, encoding="utf-8")

        manifest = {
            "name": "Mineclonia Arabic translation overlay",
            "language": "ar",
            "source_commit": source_commit,
            "source_game_version": "0.123.0",
            "built_at_utc": datetime.now(timezone.utc).isoformat(),
            "po_source_files": plan["po_file_count"],
            "translated_messages": plan["message_count"],
            "runtime_files": len(file_records),
            "tr_files": len(plan["tr_files"]),
            "po_fallbacks": len(plan["po_fallbacks"]),
            "files": sorted(file_records, key=lambda x: x["path"]),
        }
        (stage / "manifest.json").write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

        if output_path.exists():
            output_path.unlink()
        with zipfile.ZipFile(output_path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
            for path in sorted(stage.rglob("*")):
                if path.is_file():
                    archive.write(path, path.relative_to(stage).as_posix())

    print(
        f"PACKAGE PASS: {output_path.name}; {len(file_records)} files "
        f"({len(plan['tr_files'])} TR, {len(plan['po_fallbacks'])} PO), "
        f"{plan['message_count']} translated messages."
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=("prepare", "package"))
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--plan", required=True, type=Path)
    parser.add_argument("--source-commit", default=EXPECTED_SOURCE_COMMIT)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    if args.mode == "prepare":
        prepare(args.source_root.resolve(), args.plan.resolve(), args.source_commit)
    else:
        if args.output is None:
            parser.error("--output is required in package mode")
        package(args.source_root.resolve(), args.plan.resolve(), args.output.resolve(), args.source_commit)


if __name__ == "__main__":
    main()

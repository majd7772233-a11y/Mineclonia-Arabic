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
        if not entry.obsolete and entry.msgid
    ]


def decode_template_key(raw: str, where: str) -> str:
    """Decode the escapes used by Luanti template.txt and legacy TR files."""
    result: list[str] = []
    i = 0
    while i < len(raw):
        char = raw[i]
        if char == "\\" and i + 1 < len(raw):
            nxt = raw[i + 1]
            if nxt == "n":
                result.append("\n")
                i += 2
                continue
            if nxt == "t":
                result.append("\t")
                i += 2
                continue
            if nxt == "r":
                result.append("\r")
                i += 2
                continue
            if nxt == "\\":
                result.append("\\")
                i += 2
                continue
            if nxt == "=":
                result.append("=")
                i += 2
                continue
            if nxt == "@":
                result.append("@")
                i += 2
                continue
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
        if line[i] == "\\" and i + 1 < len(line):
            i += 2
        elif line[i] == "@":
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


def parse_tr_file(path: Path) -> dict[str, str]:
    # The TR grammar allows @ followed by a physical newline as a newline
    # escape. Replace it with a sentinel before processing physical lines.
    raw = path.read_text(encoding="utf-8-sig")
    raw = raw.replace("@\r\n", "\u0000").replace("@\n", "\u0000")
    records: dict[str, str] = {}
    for line_no, line in enumerate(raw.splitlines(), 1):
        if not line or line.startswith("#"):
            continue
        where = f"{path}:{line_no}"
        separator = find_template_separator(line, where)
        source = decode_tr_value(line[:separator], where)
        translated = decode_tr_value(line[separator + 1:], where)
        if source in records:
            fail(f"{where}: duplicate source key in generated TR file: {source!r}")
        records[source] = translated
    return records


def decode_tr_value(raw: str, where: str) -> str:
    result: list[str] = []
    i = 0
    while i < len(raw):
        char = raw[i]
        if char == "\u0000":
            result.append("\n")
            i += 1
            continue
        if char == "\\" and i + 1 < len(raw):
            nxt = raw[i + 1]
            if nxt == "t":
                result.append("\t")
            elif nxt == "n":
                result.append("\n")
            elif nxt == "r":
                result.append("\r")
            elif nxt == "\\":
                result.append("\\")
            elif nxt == "=":
                result.append("=")
            elif nxt == "@":
                result.append("@")
            else:
                fail(f"{where}: unsupported backslash escape \\{nxt}")
            i += 2
            continue
        if char != "@":
            result.append(char)
            i += 1
            continue
        if i + 1 >= len(raw):
            fail(f"{where}: dangling @ escape")
        nxt = raw[i + 1]
        if nxt == "@":
            result.append("@")
        elif nxt == "=":
            result.append("=")
        elif nxt == "n":
            result.append("\n")
        elif nxt in "123456789":
            result.append("@" + nxt)
        elif nxt == "\u0000":
            result.append("\n")
        else:
            fail(f"{where}: unsupported TR escape @{nxt}")
        i += 2
    return "".join(result)


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
    source_po_entries = 0
    whitespace_passthrough_entries = 0
    runtime_messages = 0
    runtime_whitespace_keys = 0
    stale_po_entries: list[dict[str, object]] = []
    missing_runtime_entries: list[dict[str, str]] = []
    po_runtime_modules: list[dict[str, object]] = []
    po_runtime_catalogs: list[dict[str, str]] = []
    seen_domains: dict[str, str] = {}
    expected_catalogs: dict[str, dict[str, str]] = {}
    module_locale_paths: dict[str, Path] = {}
    module_domains: dict[str, str] = {}

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
        # Legacy TR cannot represent gettext contexts or plural forms. Keep the
        # whole module as a native Luanti PO catalog when either feature is used.
        use_native_po = any(entry.msgctxt or entry.msgid_plural for entry in entries)
        if any("fuzzy" in entry.flags for entry in entries):
            fail(f"{rel}: contains fuzzy entries")

        po_ids = [entry.msgid for entry in entries]
        pot_ids = [entry.msgid for entry in source_entries]
        domain, template_ids = parse_template_txt(txt_path)
        # template.txt is regenerated from the running game and is the
        # authoritative runtime inventory. A single gettext key may occur at
        # several call sites, but a TR catalog stores each source key once.
        required_keys = set(template_ids)
        po_keys = set(po_ids)
        missing_required = sorted(required_keys - po_keys)
        if missing_required:
            print(
                f"MISSING RUNTIME TRANSLATION KEYS IN {rel} ({len(missing_required)}):",
                flush=True,
            )
            for missing_key in missing_required:
                print("MISSING_RUNTIME\\t" + json.dumps(missing_key, ensure_ascii=False), flush=True)
                missing_runtime_entries.append({
                    "path": str(rel),
                    "key": missing_key,
                })
        extra_po = sorted(po_keys - required_keys)
        if extra_po:
            stale_po_entries.append({
                "path": str(rel),
                "count": len(extra_po),
                "examples": extra_po[:20],
            })
        runtime_messages += sum(1 for key in required_keys if not key.isspace())
        runtime_whitespace_keys += sum(1 for key in required_keys if key.isspace())
        previous = seen_domains.get(domain)
        if previous and previous != str(rel.parent):
            fail(f"textdomain collision: {domain!r} belongs to both {previous} and {rel.parent}")
        seen_domains[domain] = str(rel.parent)

        for entry in entries:
            if entry.msgid_plural:
                plural_values = list(entry.msgstr_plural.values())
                if len(plural_values) != 6 or any(not value.strip() for value in plural_values):
                    fail(
                        f"{rel}: Arabic plural entry {entry.msgid!r} must have six non-empty msgstr_plural forms"
                    )
                if Counter(re.findall(r"@[1-9]", entry.msgid)) != Counter(
                    re.findall(r"@[1-9]", entry.msgid_plural)
                ):
                    fail(f"{rel}: singular/plural placeholders differ for {entry.msgid!r}")
                for form_index, value in enumerate(plural_values):
                    check_placeholder_pair(
                        entry.msgid_plural, value,
                        f"{rel}: {entry.msgid!r} plural form {form_index}"
                    )
            else:
                if not entry.msgstr.strip() and not (entry.msgid.isspace() and entry.msgstr == entry.msgid):
                    fail(f"{rel}: empty translation for {entry.msgid!r}")
                check_placeholder_pair(entry.msgid, entry.msgstr, f"{rel}: {entry.msgid!r}")

        # Luanti Translation Tools expects a flat mods folder: each direct
        # child must be a mod root containing init.lua and mod.conf. Mineclonia
        # groups its mods by category, so make a temporary flat conversion tree
        # while preserving each module's original path for the install ZIP.
        module = rel.parent.parent.name
        if module in module_locale_paths:
            fail(f"duplicate module directory name in package staging: {module}")
        module_locale_paths[module] = rel.parent
        module_domains[module] = domain
        stage_module = stage_mods / module
        locale_stage = stage_module / "locale"
        locale_stage.mkdir(parents=True, exist_ok=True)
        source_mod = source_root / rel.parent.parent
        source_init = source_mod / "init.lua"
        if not source_init.is_file():
            fail(f"{rel}: source mod root has no init.lua required by Luanti Translation Tools")
        shutil.copy2(source_init, stage_module / "init.lua")
        source_mod_conf = source_mod / "mod.conf"
        if source_mod_conf.is_file():
            shutil.copy2(source_mod_conf, stage_module / "mod.conf")
        shutil.copy2(txt_path, locale_stage / "template.txt")

        source_po_entries += len(entries)
        whitespace_entries = [entry for entry in entries if entry.msgid.isspace()]
        message_entries = [entry for entry in entries if not entry.msgid.isspace()]
        whitespace_passthrough_entries += len(whitespace_entries)
        message_count = len(message_entries)
        total_messages += message_count
        per_module.append({
            "module": module,
            "path": str(rel.parent),
            "textdomain": domain,
            "messages": message_count,
            "runtime_format": "po" if use_native_po else "tr",
        })

        if use_native_po:
            # Do not put this PO in the converter staging tree: the converter
            # must not flatten its gettext plural forms. It is copied verbatim
            # into the original locale path after the TR modules are processed.
            po_runtime_modules.append({
                "module": module,
                "path": str(rel.parent),
                "runtime_path": str(Path("mods") / rel.parent.relative_to("mods") / po_path.name),
                "messages": message_count,
                "reason": "gettext plural forms or message contexts require native PO support",
            })
            po_runtime_catalogs.append({
                "path": str(rel.parent),
                "runtime_path": str(Path("mods") / rel.parent.relative_to("mods") / po_path.name),
                "source_sha256": sha256_file(po_path),
            })
        else:
            poconvert = locale_stage / "poconvert"
            poconvert.mkdir(exist_ok=True)
            # A source key made only of whitespace is a formatting token, not
            # visible text. Luanti's TR converter collapses it to an empty key;
            # leave it to the engine's original source-string fallback instead.
            po_for_tr = polib.pofile(str(po_path), check_for_duplicates=True)
            for staged_entry in list(po_for_tr):
                if not staged_entry.obsolete and staged_entry.msgid and staged_entry.msgid.isspace():
                    po_for_tr.remove(staged_entry)
            po_for_tr.save(str(poconvert / po_path.name))
            expected_catalogs[module] = {
                entry.msgid: entry.msgstr for entry in message_entries
            }

        print(
            f"VALID {module}: {message_count} messages, textdomain={domain}, "
            f"runtime_format={'PO' if use_native_po else 'TR'}",
            flush=True,
        )

    if missing_runtime_entries:
        print(
            f"RUNTIME COVERAGE SUMMARY: {len(missing_runtime_entries)} missing unique key(s) "
            f"across {len({item['path'] for item in missing_runtime_entries})} module(s).",
            flush=True,
        )
        for item in missing_runtime_entries:
            print(
                "MISSING_RUNTIME_SUMMARY\\t"
                + item["path"]
                + "\\t"
                + json.dumps(item["key"], ensure_ascii=False),
                flush=True,
            )
        fail("Arabic PO catalogs do not cover every generated runtime template string")

    if total_messages < args.expected_messages:
        fail(
            f"expected at least {args.expected_messages} translated PO messages, "
            f"found {total_messages}"
        )

    # Mineclonia's documented layout: PO files live in locale/poconvert;
    # the selected converter writes Arabic .tr files into each locale folder.
    using_local_fallback = args.mtt_convert.name == "ltt_tr_compat.py"
    if using_local_fallback:
        converter_description = "project-local Luanti TR compatibility fallback (not upstream LTT)"
        validator_description = "project-local recursive TR syntax/placeholder checker"
        converter_command_name = "ltt_tr_compat.py"
    else:
        converter_description = "Wuzzy Luanti Translation Tools"
        validator_description = "Wuzzy Luanti Translation Tools"
        converter_command_name = "ltt_convert.py"

    run_checked(
        [sys.executable, str(args.mtt_convert.resolve()), str(stage_mods), "--po2tr", "-r"],
        cwd=stage_mods.parent,
        description=f"{converter_command_name} <mods> --po2tr -r",
    )
    run_checked(
        [sys.executable, str(args.mtt_check.resolve()), str(stage_mods)],
        cwd=stage_mods.parent,
        description=f"{'ltt_tr_compat.py' if using_local_fallback else 'ltt_check.py'} <mods>",
    )

    # Normalize LTT's doubled prefix to the canonical <textdomain>.ar.tr filename.
    generated_tr_files = sorted(stage_mods.rglob("*.ar.tr"))
    for generated_path in generated_tr_files:
        module = generated_path.parent.parent.name
        domain = module_domains.get(module)
        if not domain:
            fail(f"generated TR has unknown module directory: {generated_path}")
        canonical_name = f"{domain}.ar.tr"
        if generated_path.name == canonical_name:
            continue
        duplicated_name = f"{domain}.{canonical_name}"
        if generated_path.name != duplicated_name:
            fail(
                f"{generated_path}: unexpected generated filename; "
                f"expected {canonical_name!r} or {duplicated_name!r}"
            )
        canonical_path = generated_path.with_name(canonical_name)
        if canonical_path.exists():
            fail(f"cannot normalize {generated_path}: {canonical_path} already exists")
        generated_path.rename(canonical_path)
        print(f"NORMALIZED TR filename: {generated_path.name} -> {canonical_name}", flush=True)

    tr_files = sorted(stage_mods.rglob("*.ar.tr"))
    expected_tr_count = len(expected_catalogs)
    if len(tr_files) != expected_tr_count:
        fail(f"expected {expected_tr_count} generated .ar.tr files, found {len(tr_files)}")
    if len(po_runtime_modules) + len(tr_files) != len(po_files):
        fail(
            f"runtime file coverage mismatch: {len(tr_files)} TR + "
            f"{len(po_runtime_modules)} native PO != {len(po_files)} modules"
        )

    package_root = args.output.resolve() / "Mineclonia-Arabic-TR"
    if package_root.exists():
        shutil.rmtree(package_root)
    package_root.mkdir(parents=True, exist_ok=True)

    for helper_name in ("ENABLE_ARABIC_LANGUAGE.ps1", "ENABLE_ARABIC_LANGUAGE.bat"):
        helper_source = repo_root / "tools" / helper_name
        if not helper_source.is_file():
            fail(f"missing Windows language-menu helper: {helper_source}")
        shutil.copy2(helper_source, package_root / helper_name)

    installed_messages = 0
    tr_manifest: list[dict[str, object]] = []
    seen_tr_paths: set[str] = set()
    for tr_path in tr_files:
        rel = tr_path.relative_to(stage_mods)
        module = rel.parts[0]
        expected = expected_catalogs.get(module)
        if expected is None:
            fail(f"unexpected generated TR file: {tr_path}")
        actual = parse_tr_file(tr_path)
        if actual != expected:
            missing = sorted(set(expected) - set(actual))[:3]
            extra = sorted(set(actual) - set(expected))[:3]
            different = [
                key for key in expected.keys() & actual.keys()
                if expected[key] != actual[key]
            ][:3]
            fail(
                f"{tr_path}: generated TR differs from PO; "
                f"missing={missing!r}; extra={extra!r}; translation_mismatch={different!r}"
            )
        installed_messages += len(actual)
        seen_tr_paths.add(module)
        original_locale = module_locale_paths.get(module)
        if original_locale is None:
            fail(f"generated TR belongs to unknown staged module: {module}")
        destination = package_root / "mods" / original_locale.relative_to("mods") / tr_path.name
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(tr_path, destination)
        tr_manifest.append({
            "path": str(Path("mods") / original_locale.relative_to("mods") / tr_path.name),
            "entries": len(actual),
            "sha256": sha256_file(tr_path),
        })
    if seen_tr_paths != set(expected_catalogs):
        missing_paths = sorted(set(expected_catalogs) - seen_tr_paths)
        fail(f"missing generated TR files for: {missing_paths[:10]}")

    po_runtime_messages = sum(int(item["messages"]) for item in po_runtime_modules)
    if installed_messages + po_runtime_messages != total_messages:
        fail(
            f"runtime message count mismatch: expected {total_messages}, "
            f"found {installed_messages} TR + {po_runtime_messages} PO"
        )

    po_manifest: list[dict[str, object]] = []
    for item in po_runtime_catalogs:
        source_file = repo_root / item["path"] / Path(item["runtime_path"]).name
        if not source_file.is_file():
            fail(f"native PO fallback disappeared before packaging: {source_file}")
        relative = Path(item["runtime_path"])
        destination = package_root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_file, destination)
        po_manifest.append({
            "path": str(relative),
            "format": "gettext-po",
            "reason": "preserves Arabic gettext plural forms; requires Luanti 5.10.0+",
            "messages": next(
                int(module["messages"])
                for module in po_runtime_modules
                if str(module["runtime_path"]) == str(relative)
            ),
            "sha256": sha256_file(destination),
        })

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
- ملفات تشغيل Luanti: {len(tr_files)} ملفاً بصيغة .ar.tr و{len(po_runtime_modules)} ملفاً بصيغة .ar.po الأصلية.
- أداة التحويل لملفات TR: {converter_description}.
- إجمالي الوحدات المغطاة: {len(tr_files) + len(po_runtime_modules)} من {len(po_files)}.
- إجمالي النصوص المترجمة: {total_messages}.
- مفاتيح التنسيق المكونة من مسافات فقط: {whitespace_passthrough_entries}؛ تُترك لمحرك اللعبة كما هي ولا تتحول إلى مفاتيح TR فارغة.
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
4. لا تستبدل مجلد mods بالكامل؛ اسمح بدمج ملفات locale فقط.
5. تحتوي الحزمة على ملف PO أصلي للوحدة ذات صيغ الجمع الست؛ يتطلب تحميله Luanti 5.10.0 أو أحدث، وإصدارك المبلغ عنه 5.17.0 يفي بذلك.
6. شغّل ENABLE_ARABIC_LANGUAGE.bat الموجود بجانب مجلد mods. اختر جذر تثبيت Luanti الذي يحتوي على bin وbuiltin (في شجرة جهازك: C:\\Users\\Magd\\AppData\\Local\\luanti\\5.17.0). لا تختر مجلد Mineclonia. يقبل السكربت أيضاً اختيار builtin أو bin ويحدد الجذر تلقائياً.
7. في إصدار Windows المحمول RUN_IN_PLACE سيكتشف السكربت وجود minetest.conf.example ويضبط language = ar في ملف minetest.conf بجذر التثبيت، بجانب bin؛ وهذا يطابق سلوك المحرك الذي يتجاهل متغيرات مسار المستخدم في وضع RUN_IN_PLACE. أما التثبيت غير المحمول فيحترم LUANTI_USER_PATH أو MINETEST_USER_PATH، ثم يستخدم %APPDATA%\\Minetest\\minetest.conf عند غيابها.
8. لن تحتاج إلى اختيار arbe أو العربية يدوياً من القائمة؛ أغلق Luanti ثم افتحه مجدداً، ويقرأ المحرك اللغة ar عند بدء التشغيل. يحتفظ السكربت ببقية إعداداتك، وينشئ نسخة احتياطية من كل ملف سيغيره داخل %LOCALAPPDATA%\\Mineclonia-Arabic\\LuantiBackups.

إزالة التعريب
-------------
احذف ملفات *.ar.tr وملفات *.ar.po الخاصة بهذه الحزمة من مجلدات mods/*/locale/.

ملاحظات التوافق
---------------
هذه الحزمة مبنية من قوالب template.pot وكتالوجات template.txt التي أُعيد
استخراجها بتشغيل لقطة المصدر المثبتة أعلاه في خادم Luanti. جرى التحقق من
تغطية مفاتيح runtime ومطابقة مخرجات TR مع ترجمات PO. وحدات gettext ذات صيغ
الجمع تُوزّع كملفات PO أصلية، لا تُختزل إلى ترجمة TR واحدة.

لم تتم مطابقة شجرة ملفات اللعبة المحلية مع لقطة المصدر بصورة مباشرة، ولذلك
لا يمكن ضمان التطابق التام مع release=39603 حتى تُفحص نسخة اللعبة نفسها.
ولم يتم اختبار العرض داخل عميل Luanti فعلياً ضمن عملية البناء هذه. يدعم المشغّل الآن تفعيل locale ar عبر قائمة المحرك وإعداد المستخدم المحفوظ؛ لكنه لا يضيف تشكيل الحروف أو تخطيط RTL الكامل إلى محرك Luanti.
"""
    (package_root / "INSTALL_ARABIC.txt").write_text(install_text, encoding="utf-8")

    manifest = {
        "name": "Mineclonia Arabic runtime translation overlay",
        "language": "ar",
        "format": "Luanti TR plus native gettext PO fallback",
        "source_repository": "https://github.com/ZenonSeth/mineclonia-mirror",
        "source_commit": args.source_commit,
        "source_version": source_version,
        "target_game_release_owner_reported": args.target_release,
        "luanti_version_owner_reported": args.luanti_version,
        "po_files": len(po_files),
        "source_po_entries_including_whitespace_tokens": source_po_entries,
        "whitespace_only_source_entries_left_to_engine_fallback": whitespace_passthrough_entries,
        "tr_files": len(tr_files),
        "native_po_runtime_files": len(po_runtime_modules),
        "native_po_runtime_modules": po_runtime_modules,
        "translated_messages_in_po_and_tr": total_messages,
        "messages_in_tr": installed_messages,
        "messages_in_native_po": po_runtime_messages,
        "runtime_template_messages": runtime_messages,
        "runtime_whitespace_only_keys_left_to_engine_fallback": runtime_whitespace_keys,
        "extra_po_entries_not_present_in_current_template_txt": sum(x["count"] for x in stale_po_entries),
        "extra_po_entries_report": stale_po_entries,
        "converter_implementation": converter_description,
        "converter_is_official_ltt": not using_local_fallback,
        "conversion": f"{converter_command_name} <mods> --po2tr -r",
        "validation": validator_description,
        "runtime_tested": False,
        "official_weblate_merged": False,
        "language_menu_patch": {
            "included_script": "ENABLE_ARABIC_LANGUAGE.ps1",
            "launcher": "ENABLE_ARABIC_LANGUAGE.bat",
            "targets": [
                "builtin/settingtypes.txt",
                "builtin/common/settings/dlg_settings.lua",
                "minetest.conf (RUN_IN_PLACE install root or resolved Windows user path)",
            ],
            "purpose": "Expose canonical locale ar in Luanti 5.17.0 and persist language = ar in the actual user config. Detects RUN_IN_PLACE from minetest.conf.example and writes to the install root because Luanti bypasses setSystemPaths() in that compile-time mode; non-portable builds honor LUANTI_USER_PATH and MINETEST_USER_PATH before the APPDATA fallback. Windows PowerShell 5.1 tests cover root/builtin/bin selection, malformed arbe repair, RUN_IN_PLACE precedence, config persistence, preservation, and idempotency; no RTL engine support is added.",
        },
        "tr_filename_normalization": "textdomain.ar.tr",
        "modules": per_module,
        "files": tr_manifest + po_manifest,
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
        "converter_is_official_ltt": not using_local_fallback,
        "converter_implementation": converter_description,
        "po_files": len(po_files),
        "source_po_entries_including_whitespace_tokens": source_po_entries,
        "whitespace_only_source_entries_left_to_engine_fallback": whitespace_passthrough_entries,
        "tr_files": len(tr_files),
        "native_po_runtime_files": len(po_runtime_modules),
        "messages": total_messages,
        "messages_in_tr": installed_messages,
        "messages_in_native_po": po_runtime_messages,
        "runtime_template_messages": runtime_messages,
        "runtime_whitespace_only_keys_left_to_engine_fallback": runtime_whitespace_keys,
        "stale_po_entries": sum(x["count"] for x in stale_po_entries),
        "source_version": source_version,
        "package": str(zip_path),
        "sha256": checksum,
    }, ensure_ascii=False, indent=2), flush=True)


if __name__ == "__main__":
    main()

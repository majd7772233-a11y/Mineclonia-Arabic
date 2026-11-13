# البنية التقنية

## 1. مسار ترجمة Mineclonia
1. ملفات PO العربية في mods/**/locale هي مصادر الترجمة.
2. template.pot يحدد مفاتيح المصدر؛ بصمات القوالب في SOURCE_TEMPLATES.md.
3. GLOSSARY.md مرجع المصطلحات.
4. tools/build_tr_package.py وtools/check_po.py وtools/ltt_tr_compat.py أدوات فحص وتحويل.
5. ملفات التشغيل تكون عادة .ar.tr؛ مكون mcl_biome_dispatch له احتياج PO لصيغ الجمع موثق في SOURCE_VERSION.md.

لا تغيّر msgid أو placeholders أو بنية الرسائل متعددة الأسطر.

## 2. تخطيط النص العربي

تدفق العمل:

UTF-8 / wide text
-> UTF-32 logical code points
-> Raqm layout
-> FriBidi paragraph direction / BiDi runs
-> HarfBuzz OpenType shaping
-> FreeType font faces and glyph metrics
-> visual glyph order + logical clusters
-> rendering / dimensions / cursor stops / selection geometry

لا تعكس النص المنطقي يدوياً. يتم تشكيل النص في Raqm مع الحفاظ على فهارس المصدر المنطقية. الترتيب المرئي يختلف عن الترتيب المنطقي خصوصاً في النص المختلط.

### الخطوط والفهارس
- الخط المرفق NotoSansArabic-Regular.ttf موجود في luanti/code/fonts وluanti/5.17.0/fonts.
- CGUITTFont يحمل الخط عبر FreeType وينشئ Raqm layouts.
- يمكن لعنقود glyph أن يأتي من face مختلف عبر fallback.
- Linux غالباً wchar_t 32-bit؛ Windows wchar_t 16-bit ويحتاج تحويل UTF-16/surrogate pairs إلى UTF-32.
- لا يجوز للمؤشر أو الحذف تقسيم surrogate pair أو grapheme أو shaping cluster.
- grapheme boundary وshaping-cluster boundary مفهومان مختلفان.

### font_text_layout.h
يساعد في استخراج shaping cluster boundaries، التحقق من cursor stops، توسيع التحديد، حساب visual selection intervals متعددة، تصنيف Unicode whitespace/punctuation دون locale، وحدود الكلمات والحركة المرئية للمؤشر.

### CGUITTFont.cpp/.h
مسؤول عن تحويل الفهارس wide/UTF-32، إنشاء Raqm layout، قياس النص، تحديد موضع المؤشر، التنقل المرئي، visual word navigation، selection geometry ورسم glyphs مع offsets/advances/clusters وfallback fonts.

### StaticText
في src/irrlicht_changes/static_text.cpp يتم كشف اتجاه الفقرة عبر Raqm فقط لخط EGFT_CUSTOM. لا تحوّل bitmap font إلى CGUITTFont. النص يظل بترتيبه المنطقي وBiDi يعاد ترتيبه أثناء shaping/drawing.

### CGUIEditBox
في irr/src/CGUIEditBox.cpp يوجد تنقل بصري بالكلمة، حذف بالكلمة، hit-testing إلى أقرب caret stop مسموح، selection intervals، وتكامل مع copy/cut/paste. يجب الحفاظ على المؤشر والتحديد عند رفض الإدخال.

### editbox_text_edit.h
يفصل بناء النص المرشح عن تعديل widget: يقص cursor/mark indexes، يحدد نطاق الاستبدال، يعالج overwrite دون ابتلاع CR/LF، ويفحص Max قبل قبول التغيير.

## 3. builtin SHA-256
CMake يولد manifest keys بشرطات slash. Windows filesystem قد يعيد backslashes. normalizeBuiltinFileSha256Path() في src/builtin_files.h يوحدها قبل lookup، ويستخدم في script/cpp_api/s_security.cpp. testBuiltinFileSha256Manifest يختبر entries والمسارات. لا تخف التحذير؛ اختبر exe وbuiltin المتطابقين.

## 4. قائمة اللغة وLua
شجرة بناء المصدر luanti/code/builtin، والشجرة المرجعية luanti/5.17.0/builtin. خطأ القائمة الأصلي كان فاصلة مفقودة بعد العربية قبل be؛ الفاصلة موجودة حالياً في النسختين. tools/validate_lua_syntax.py يقرأ كل Lua من الشجرتين لكنه يحتوي regex مخصصاً مع double escaping يحتاج إصلاحاً.

## 5. CMake/vcpkg
vcpkg.json يثبت الاعتماديات. Linux triplet x64-linux static، ويستخدم raqm.pc مع pkg-config --static حتى لا تسقط تبعيات HarfBuzz/FriBidi/FreeType. Windows يستخدم x64-windows. تفضيل imported targets يحافظ على transitive link dependencies. ENABLE_SOUND=ON صريح. ZLIB يُكتشف بـ find_package(ZLIB MODULE REQUIRED) لإنشاء ZLIB::ZLIB المتوافق مع static vcpkg.

## 6. ما يحتاج اختباراً يدوياً
فتح قوائم العميل وإعدادات اللغة، عدم ظهور Lua parser error، اختفاء SHA warnings مع exe/builtin من حزمة واحدة، clipboard round-trip، mouse drag وShift selection، clipping/shadows/centering/alignment، scroll، fallback/emoji/supplementary Unicode.

## 7. Windows portable path model

For Windows RUN_IN_PLACE ZIP builds, path_share remains the application/share root containing builtin, fonts, textures, locale and binaries. path_user is now derived as path_share/Minetest. The engine's existing path_user consumers therefore place worlds, user-installed games, config, logs and other writable profile data below that directory; path_cache is path_user/cache.

The first-launch migration copies the legacy %APPDATA%/Minetest tree into a staging directory beside the target and activates it only after the copy succeeds. The old profile remains untouched. Existing portable data is not merged with AppData automatically. Windows RUN_IN_PLACE config discovery is restricted to path_user/minetest.conf; parent-level legacy config discovery remains only on non-Windows RUN_IN_PLACE builds.

This change is intentionally Windows-specific. Other platforms keep their prior RUN_IN_PLACE path behavior. The Windows workflow's package probe is intended to prove migration and prevent accidental writes back to AppData.

### Portable package smoke-test boundary

The Windows package probe runs against isolated copies of the installed tree. One scenario verifies first-launch copy from synthetic AppData, a second startup, migrated settings/world/game/cache content, cache-tag location/signature, an unchanged legacy profile, and no leftover staging folder. The package path contains spaces. A second scenario starts with an existing `Minetest` directory and proves that legacy AppData is neither merged nor allowed to overwrite existing portable files.

The probe starts the executable with the valid option syntax `--worldlist path` (two separate arguments). Run #112 showed that `--worldlist=path` is rejected as an unknown command-line parameter before the application initializes; the subsequent missing-config assertion was therefore a consequence of the invalid invocation, not proof of a migration defect. The workflow does not parse stdout from the GUI client linked with `/SUBSYSTEM:WINDOWS`; it validates the files and contents produced after a valid command. TestFileSys also exercises `getAvailableWorlds()` and `getAvailableGameIds()` with a temporary `path_user` containing spaces.

Run #114 reached world discovery and listed the synthetic world beneath the portable profile twice, but the package assertion found no root-level `minetest.conf`. The Windows workflow now logs the synthetic source inventory and the portable destination inventory when required paths are missing. A direct 16th TestFileSys case exercises `fs::CopyDir()` with a root-level config file and nested world file, both under paths with spaces, to distinguish a directory-copy defect from a startup/migration fixture issue. The next CI result must identify which file set reached the destination before any production migration change is considered.

These automated checks do not prove ContentDB, screenshots, settings saves, or removable-drive behavior on the user's real Windows installation.


### Windows portable path normalization and staging recovery — pending CI

The installed Windows executable can derive `path_share` through an executable location containing `bin/..`. The portable path helper now resolves an existing share directory with `fs::AbsolutePath()` before deriving `path_user`, retaining the original value as a fallback if canonicalization is unavailable.

First-launch migration keeps its staging folder next to the final profile for a same-volume rename, but names each attempt `Minetest.migration-tmp-<pid>` and adds a numeric suffix if that path exists. It never deletes or overwrites a pre-existing legacy staging path. A staging folder created by the current attempt is cleaned if its copy or rename activation fails; rename errors include the CRT error code/message. The Windows CI probe seeds a known old fixed-name staging directory and checks preservation plus cleanup of the current attempt's uniquely named staging directory. This correction is pending a full CI run.

### Run #115: trace before production migration changes

The latest package smoke (Run #115) showed an apparent discrepancy: two valid world-list launches printed a world beneath the portable profile, but the subsequent recursive file check found no portable files although four synthetic AppData source files were confirmed. The next workflow invocation adds `--trace` before path initialization so its stderr includes info/verbose/trace events from migration and CopyFileEx. The FileSys CopyDir regression now pre-creates the destination root to match the production staging setup. PowerShell and .NET inventories are reported independently. Do not change production copy logic until these observations identify the actual failure.


### Run #116 — migration and staging recovery verified

The successful Windows CI trace shows the production migration copying each seeded profile file with CopyFileEx into a process-specific staging sibling, then activating it as the package-root `Minetest` profile. This is tested from a package path containing spaces. A second launch reuses the portable profile; an injected stale `Minetest.migration-tmp` directory remains untouched while the PID-specific stage is cleaned. Legacy AppData files are hash-identical before/after migration. A second package scenario with an existing portable profile confirms that legacy AppData is neither merged nor allowed to overwrite existing portable files.

Run #116 also passed direct C++ world/game discovery and CopyDir regression checks under paths with spaces, TestFileSys 16/16, and Arabic layout 17/17 on both Windows and Linux. Real GUI behavior and all user-data routes still require Phase 4 tests on the user's Windows installation.


## Run #117 regression and restored migration invariants

The tested Run #116 implementation normalized the Windows executable-derived share path with `fs::AbsolutePath()` before forming the writable profile. This removes the non-canonical `bin/..` component before filesystem operations such as directory activation. Its migration helper used a unique `Minetest.migration-tmp-<pid>` sibling (adding a suffix on collision), preserving stale directories and avoiding a fixed-name conflict. Run #117's later HEAD dropped both safeguards and the package test reverted to parsing GUI-subsystem stdout.

The corrective batch restores the exact Run #116 source and workflow files. The test gate again verifies the normalization and CopyDir behavior directly; the package test uses trace and file-system evidence instead of captured stdout. Require successful Run #118 before treating the restored HEAD as validated.


## Run #118 — portable migration restored and CI-verified

The restored source/workflow baseline passed Run #118 on Linux and Windows. Both hosts passed Arabic layout 17/17 and FileSys 16/16. The Windows trace records four successful CopyFileEx calls to a PID-specific stage and activation of the portable Minetest profile. Repeated startup, AppData immutability, stale staging preservation, PID-stage cleanup and existing-profile/no-merge all passed. This is automated package evidence; GUI feature/storage use still requires actual Windows acceptance.

## Mixed-script vertical alignment and fallback-aware line boxes

The default GUI font chain uses Arimo for Latin/digits and Noto Sans Arabic as the Arabic fallback. Raqm assigns glyphs from both faces within one shaped line. The glyphs must use one shared baseline calculated from the maximum ascender across the full fallback chain; using each face's own ascender made Arabic glyphs sit below adjacent Latin and numeric glyphs.

CGUITTFont now uses fallback-aware line metrics for both drawing and getDimension. The line box covers the maximum nominal face height and the combined maximum ascender/deepest descender, with one pixel of clearance. Font metric initialization also uses the same width=0,height=size FreeType request as shaping and glyph rasterization. The signed vertical Raqm offset is rounded symmetrically.

The 18th Arabic layout regression uses Arimo + Noto and a line mixing Latin, Arabic and digits, then checks actual raster bounds against the shared line box. It does not alter logical source ordering, advances, clusters or BiDi resolution. Widget-specific clipping may still require inspection if a fixed-size formspec element supplies a smaller clip rectangle than its measured text.

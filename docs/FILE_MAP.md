# خريطة ملفات المستودع

## المستوى الأعلى

| المسار | المسؤولية |
|---|---|
| AGENTS.md | قواعد العمل والمساهمة |
| README.md | مقدمة المشروع |
| PROGRESS.md | تقدم الترجمة وتحديث حالة المحرك |
| SOURCE_VERSION.md | مصدر Mineclonia وقيود المطابقة |
| SOURCE_TEMPLATES.md | قوالب PO وبصمات Git blob |
| GLOSSARY.md | المصطلحات العربية الموحدة |
| mods/ | ملفات PO العربية |
| luanti/ | مصدر المحرك والمرجع والموارد |
| tools/ | محولات وفحوص وأدوات |
| .github/workflows/ | GitHub Actions |
| docs/ | حالة المشروع والبنية والتسليم والسجل |

## وثائق المشروع

| الملف | دوره |
|---|---|
| PROJECT_OVERVIEW.md | نطاق المشروع ومساراه |
| PROJECT_STATE.md | المرجع التنفيذي للحالة الحالية |
| ARCHITECTURE.md | تدفق النص العربي والتشكيل |
| FILE_MAP.md | هذه الخريطة |
| BUILD_TESTING.md | أوامر الفحص والبناء والحزم |
| ROADMAP.md | ترتيب العمل |
| DOCUMENTATION_RULES.md | سياسة التوثيق |
| NEXT_SESSION_PROMPT.md | رسالة الاستئناف |
| ARABIC_ENGINE_HANDOFF.md | تسليم مختصر |
| ARABIC_ENGINE_SUPPORT.md | تاريخ الدعم التقني |
| ARABIC_ENGINE_WORKLOG.md | سجل العمل append-only |

## شجرة العمل الأساسية

mods/<CATEGORY>/<MOD>/locale/<MOD>.ar.po

luanti/5.17.0/
- builtin/: نسخة مرجعية من builtin.
- fonts/: الخطوط المرفقة.
- bin/: ملفات تنفيذية/مكتبات مرفقة؛ لا تفترض أنها تطابق HEAD الحالي.
- ARABIC_TEXT_FIX.md: سجل مشكلة العرض السابقة.

luanti/code/
- builtin/: builtin الذي يدخل بناء المحرك.
- fonts/: خطوط البناء.
- po/ar/luanti.po: كتالوج gettext العربي.
- src/irrlicht_changes/CGUITTFont.cpp/.h
- src/irrlicht_changes/font_text_layout.h
- src/irrlicht_changes/editbox_text_edit.h
- src/irrlicht_changes/static_text.cpp/.h
- src/unittest/test_arabic_text_layout.cpp
- src/builtin_files.h
- src/script/cpp_api/s_security.cpp
- src/CMakeLists.txt
- irr/src/CGUIEditBox.cpp
- irr/src/CMakeLists.txt
- vcpkg.json

tools/
- validate_luanti_arabic.py
- validate_lua_syntax.py
- validate_arabic_test_source.py
- build_tr_package.py
- check_po.py
- ltt_tr_compat.py
- ENABLE_ARABIC_LANGUAGE.bat/.ps1
- tests/

.github/workflows/
- build-arabic-package.yml
- build-luanti-arabic-engine.yml
- validate-translations.yml

## تحديد مصدر التعديل
- تعديل المحرك يبدأ من luanti/code.
- تعديل builtin القائمة يجب أن يراجع الشجرتين.
- لا تعتبر luanti/5.17.0/bin/luanti.exe بناء HEAD الحالي ما لم يثبت تطابقه.
- ترجمات mods لا تتبع دورة بناء محرك Luanti.

## Windows portable profile implementation

| File | Responsibility |
|---|---|
| luanti/code/src/porting.h | Portable writable profile path API |
| luanti/code/src/porting.cpp | Normalized Windows portable profile path, first-launch AppData copy and collision-safe process-specific staging |
| luanti/code/src/main.cpp | Windows config path restricted to the portable profile |
| luanti/code/src/unittest/test_filesys.cpp | 16-case portable path normalization, world/game discovery and CopyDir root/nested file checks under paths with spaces and pre-created destination |
| .github/workflows/build-luanti-arabic-engine.yml | TestFileSys 16-case gates; Windows migration/second-start, path-with-spaces, AppData hashes, stale-stage preservation and no-merge probes (Run #116 passed) |
| luanti/code/README.md | Windows ZIP path layout reference |


## Run #116 verified portable-profile evidence

The Windows package smoke and both platform test suites passed Run #116. The remaining real-device acceptance is tracked in `docs/ROADMAP.md` and `docs/ARABIC_ENGINE_HANDOFF.md`.


## Run #117 rollback / restoration note

The Windows portable-path implementation, 16-case TestFileSys suite, and trace-based package workflow are restored from the successful Run #116 tested HEAD. The exact restored files are recorded in the corrective commit:
- `luanti/code/src/porting.cpp`
- `luanti/code/src/porting.h`
- `luanti/code/src/unittest/test_filesys.cpp`
- `.github/workflows/build-luanti-arabic-engine.yml`

Run #118 is required to validate the new branch head again.


## Run #118 verified Windows package

- Tested source/workflow commit: `8896f0f3eaa7ca19a97bd0cfcfcfac5a05bf7192`.
- CI Run: `43385926666` (all jobs passed).
- Windows artifact: `14018204334`; inner ZIP integrity passed with 533 entries.
- Directories and key payload: `bin/luanti.exe`, builtin, Noto Sans Arabic and its OFL license.
- Remaining manual acceptance and test steps: `docs/ROADMAP.md`, `docs/PROJECT_STATE.md`, and `docs/ARABIC_ENGINE_HANDOFF.md`.

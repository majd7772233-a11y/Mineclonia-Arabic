# البناء والاختبار والتحقق

## Workflow

يتكون .github/workflows/build-luanti-arabic-engine.yml من أربع Jobs:
- workflow-sanity.
- catalog-validation.
- linux: x64-linux وNinja وvcpkg static.
- windows: x64-windows وVisual Studio 17 2022 وMSVC.

يحافظ Workflow على cancel-in-progress=true. لا تدفع commit جديداً أثناء تشغيل CI. يجب أن تبقى actions/cache/restore@v4 وactions/cache/save@v4 في كل منصة مع vcpkg baseline المثبت.

## الفحوص السريعة

من جذر المستودع:

    python tools/validate_luanti_arabic.py
    python -m pip install --disable-pip-version-check --no-cache-dir luaparser==4.2.0
    python tools/validate_lua_syntax.py
    python tools/validate_arabic_test_source.py
    msgfmt --check --check-format -o /tmp/luanti-ar.mo luanti/code/po/ar/luanti.po

Lua validator يفحص جميع ملفات Lua في builtin source وbundled trees ويتحقق من فاصلة خيار العربية قبل be. فحص regex وحده لا يعوض Lua parser.

## Preflight للـWorkflow

- YAML parsing باستخدام PyYAML أو أداة مناسبة.
- actionlint إن توفر.
- bash -n لكل run blocks الخاصة بـBash.
- مراجعة PowerShell syntax وexit codes وشروط Select-String.
- التأكد من أربع Jobs فقط مرة واحدة.
- التأكد من cache restore/save لكل منصة وبقاء cancel-in-progress=true.
- مراجعة الشروط المتسلسلة Configure/Build/Tests/Package ورفع Artifact.

في preflight بتاريخ 2026-11-12: YAML وbash -n وregex نجحت. actionlint وPowerShell parser وmsgfmt لم تكن متاحة. تعذر تنزيل luaparser==4.2.0 بسبب DNS؛ يجب أن ينفذ catalog-validation اختبار Lua في CI. لا تعني هذه القيود نجاح الفحوص غير المنفذة.

## Linux

الإعداد:
- triplet x64-linux.
- CMake 4.4.4 وNinja.
- baseline ddf0518c31a259dcc359fd4c7bcb1ce8a27e9855.
- ENABLE_SOUND=ON وRUN_IN_PLACE=ON.

    cmake --build build --parallel 2
    ./bin/luanti --run-unittests --test-module TestArabicTextLayout

المطلوب:
Module TestArabicTextLayout passed (0 failures / 17 tests)

ثم cmake --install، والتحقق من bin/luanti ومن builtin/common/settings/dlg_settings.lua ومطابقته للمصدر، ومن NotoSansArabic-Regular.ttf وOFL license. افحص ldd وتأكد من عدم وجود مكتبات runtime بحالة not found، ثم أنشئ tar.gz وارفع Artifact.

## Windows

- runner windows-2022.
- Visual Studio 17 2022 x64.
- triplet x64-windows.
- CMake 4.4.4، ENABLE_SOUND=ON، RUN_IN_PLACE=ON.

    cmake --build build --config Release --parallel 2
    bin/Release/luanti.exe --run-unittests --test-module TestArabicTextLayout

المطلوب:
Module TestArabicTextLayout passed (0 failures / 17 tests)

ثم cmake --install. تحقق من bin/luanti.exe، وقارن SHA-256 للـbuiltin المصدر والمعبأ، وتأكد من فاصلة خيار العربية وخط Noto وترخيصه. تحقق من DLLs الخاصة بـRaqm وHarfBuzz وFriBidi وOpenAL وOgg وVorbis قبل ZIP والرفع.

## CMake/vcpkg

- حافظ على Linux Raqm static link closure باستخدام raqm.pc وpkg-config --static.
- find_package(ZLIB MODULE REQUIRED) لإنشاء ZLIB::ZLIB.
- لا تخلط plain وPRIVATE signatures في target_link_libraries لهدف luanti.
- imported targets: Vorbis::vorbisfile وVorbis::vorbis وOgg::ogg وOpenAL::OpenAL وfmt::fmt.
- لا تعطل الصوت أو cURL لإخفاء أخطاء الربط.

## قراءة CI

سجل HEAD SHA، وكل Jobs وlogs، ونتائج configure/build/test/package، وعدد الاختبارات، وArtifact IDs وSHA/digest. Run108 ID 43201985097 على SHA 1c5bab319be89eb2aeba40f6626c1333dd495a7a انتهى failure لكنه لم يحوِ Jobs أو Artifacts أو check-runs؛ لا تنسبه إلى فشل compiler أو tests. لا ترفع درجة الدليل: configure ليس build، وbuild ليس test، وtest ليس package أو runtime.


## Run #109 — verified CI baseline (2026-11-12)

Run ID 43212062926, HEAD 69c62784baf96e3456239d5b954055be5e855501, completed/success. All four jobs passed: workflow sanity, catalog/source validation, Linux x64 build/tests/package, and Windows x64 MSVC build/tests/package.

Validation job:
- 245/245 builtin keys, no empty strings, source/bundle byte-identical.
- 1,575/1,575 effective gettext translations, 0 untranslated, 0 fuzzy, LANG_CODE=ar.
- `msgfmt --check --check-format`: passed.
- `luaparser==4.2.0`: 210 builtin Lua files parsed across both trees.
- Arabic test-source validator: 17 declarations, 17 registrations, 17 definitions.

Linux job 131162246258: vcpkg bootstrap/cache restore/install and CMake configure passed; cache save was skipped on cache hit. Client build passed. TestArabicTextLayout passed with 0 failures / 17 tests. Install/package checks passed: executable, source-matched builtin, Noto font/license, and no unresolved runtime dependencies from `ldd`. Artifact ID 13941099252, 22,718,197 bytes, SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.

Windows job 131162246670: vcpkg bootstrap/cache restore/install and CMake configure passed; cache save was skipped on cache hit. MSVC client build passed. TestArabicTextLayout step passed its exact 17-test zero-failure gate. Install/package checks passed: executable, source/package builtin SHA equality and comma check, Noto font/license, and Raqm/HarfBuzz/FriBidi/OpenAL/Ogg/Vorbis runtime DLL presence. Artifact ID 13941743588, 20,672,848 bytes, SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.

This is CI desktop build/test/package evidence, not user-side GUI evidence. The full Windows artifact must still be launched to verify the main menu, Settings > Language, and SHA warning behavior. Interactive edit-box and rendering tests remain open.


## Post-download artifact verification — Run #109

The GitHub artifacts were downloaded and unpacked independently after the CI run:
- Outer Linux artifact ZIP ID 13941099252: digest `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`, matching the upload action.
- Outer Windows artifact ZIP ID 13941743588: digest `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`, matching the upload action.
- Both outer ZIPs passed integrity checks and contained their inner package archive.

Linux inner package:
- `file bin/luanti`: ELF 64-bit x86-64.
- `ldd bin/luanti`: no `not found` dependencies.
- `./bin/luanti --version`: exit code 0; Luanti 5.17.0, Release, RUN_IN_PLACE=1.
- Arabic language-label Lua, `__builtin.ar.tr`, Noto Sans Arabic Regular and OFL license present.
- `./bin/luanti --run-unittests --test-module TestArabicTextLayout` run from the extracted package: 17 PASS, 0 failures, exact expected summary.

Windows inner package:
- Confirmed `bin/luanti.exe`, Raqm/HarfBuzz/FriBidi/FreeType, OpenAL/Ogg/Vorbis DLLs, language selector Lua file, Noto Sans Arabic and OFL license.
- This was not a Windows execution; launch and GUI checks still need the user's Windows environment.

A successful extracted Linux artifact test is still not a substitute for the user's GUI test. Do not claim Windows runtime or Lua menu startup success without a Windows run/log.

## Windows portable profile regression coverage

The build-tree TestFileSys module has 16 cases and is required on both Linux and Windows:
`Module TestFileSys passed (0 failures / 16 tests)`.

Portable-profile C++ coverage checks the path builder with whitespace and temporarily assigns `porting::path_user` to a path containing spaces. It creates a fixture world and user-installed game beneath that profile, then directly verifies that `getAvailableWorlds()` and `getAvailableGameIds()` discover them. This avoids depending on console output from the Windows GUI executable.

The additional `testCopyDir()` case calls `fs::CopyDir()` for a source profile whose path contains spaces. It pre-creates the destination root first, exactly as first-launch migration pre-creates its staging directory, then verifies exact contents for both a root-level `minetest.conf` and a nested `worlds/world with spaces/world.mt`.

The Windows package smoke test runs against isolated copies of the install tree and launches its startup probes with `--trace`, capturing combined output so path initialization, CopyFileEx operations and staging activation can be diagnosed:
1. The package copy is placed under a directory containing spaces.
2. A synthetic `%APPDATA%/Minetest` profile is created with a config, world, game and cache marker.
3. The package executable is launched twice using the valid two-argument syntax `--worldlist path`; the workflow checks process exit status, then verifies migrated file existence and contents under the portable `Minetest` directory.
4. The check validates the CACHEDIR.TAG signature/location, hashes the legacy AppData files before/after, and checks that the migration staging directory is gone.
5. A second package copy begins with an existing `Minetest` directory plus a distinct AppData profile. The test checks that the portable config and marker remain unchanged, legacy-only files are not merged, and the source AppData remains intact.

Never use `--worldlist=path`: Luanti's option parser expects an option value as a separate argument (`--worldlist <value>`). Run #112 rejected the equals form before initialization; the subsequent missing-file assertion exposed that invalid invocation. The test also deliberately does not parse `stdout` from the GUI-subsystem executable, which is linked with `/SUBSYSTEM:WINDOWS`.

The Run #115 Windows package log showed the synthetic world path under the portable profile twice, but all required files were then reported absent and the recursive PowerShell file inventory was empty. The exact cause remains unknown. The trace-enabled invocation and .NET/PowerShell inventories are the next evidence gate; do not change migration code until they identify the failure boundary.

These expanded checks are pending the next complete Linux/Windows CI run. They do not replace real Windows GUI/user acceptance.



## Windows portable profile — Run #110 and smoke-test correction

Run #110 (43231076763, commit 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427) verified:
- Linux build, Arabic layout 17/17, TestFileSys 14/14, package install and upload all passed.
- Windows build, Arabic layout 17/17, TestFileSys 14/14 and install/runtime DLL checks passed.
- Windows package smoke failed during the duplicate TestFileSys invocation against the copied install package; its native process output was not captured in the log. The preceding `--worldlist=path` process returned success. Do not infer an unproven migration defect.
- Linux artifact 13950892008 (22,717,628 bytes), SHA-256 ac9c301921e1443b0bd89383a1b2a0d58bbeb0def9e5a404a293ebdb12eaee1b. Windows artifact was not uploaded.

The Run #110 follow-up used first/second `--worldlist=path` output as part of the package assertion. Run #111 showed that parsing this GUI-subsystem process output is invalid; the script stopped before verifying the copied profile. The replacement checks on-disk content and no-merge behavior, and adds a direct C++ world/game discovery test.


## Run #113 migration/staging regression correction — pending CI

Run #113 passed both C++ test modules (Arabic layout 17/17 and FileSys 15/15) on Linux and Windows, and Linux packaging/upload. Windows package smoke reached profile initialization but failed when the fixed `Minetest.migration-tmp` sibling already existed at the executable-derived `bin/..` path; no portable config was activated.

The current corrective batch adds:
- a TestFileSys assertion that `getPortableUserPath()` resolves an existing share path containing `bin/..` and spaces to its absolute package root;
- process-specific migration staging names with collision suffixes, preserving old staging directories;
- a Windows package smoke test that starts with a clean install copy and a synthetic stale fixed-name staging directory containing a marker, verifies the marker survives migration, and verifies no process-specific staging directory remains afterward;
- a clean existing-profile probe, plus assertions for no-merge behavior and unchanged legacy AppData.

The migration implementation now logs the actual CRT rename error code and message when activation fails. These changes have not yet been compiled or tested; the next complete CI must rerun both platforms and the Windows smoke.


## Run #116 — final CI evidence for the Windows portable profile

Run #116 (43376273868, HEAD b0c486db3a0960c7779c2f341198fe267356f61b) passed all four jobs. On both Linux and Windows, the C++ suites passed:
- TestArabicTextLayout: 17 tests, 0 failures.
- TestFileSys: 16 tests, 0 failures.

The Windows package smoke test passed:
- process startup using `--trace --worldlist path` (valid separate option/value arguments);
- CopyFileEx migration of four seeded AppData files into a process-specific staging directory and successful activation as package-root `Minetest`;
- first and second launch, world discovery and path-with-spaces;
- exact config/world/game/cache contents and CACHEDIR.TAG signature;
- legacy AppData path/content hashes unchanged;
- stale fixed-name staging directory preserved and the new process stage removed;
- a second package with an existing `Minetest` directory did not merge or overwrite the portable profile.

Both package uploads completed. Windows Artifact 14015073206 (20,682,182 bytes; SHA-256 66e58c0a28de9b99215fb348a70f80a77a7f186875b6b97646119e348bf32ecc); Linux Artifact 14014987601 (22,731,184 bytes; SHA-256 d9ee80d7af0a291bc83479870598634b530f68b301818c903efdd83e6c7428d9).

The Windows ZIP therefore clears the automated package gate. Actual GUI/user acceptance on Windows remains outstanding, including Content tab download, world creation, settings/log/screenshot/texture/cache output paths, AppData immutability during use, and a non-system/removable drive.


## Run #117 — regression in the Windows package test

Run #117 (43382276907) passed Windows configure/build, TestArabicTextLayout 17/17 and the then-current TestFileSys 14/14. The package test failed because it again parsed captured stdout from `luanti.exe --worldlist=path`. The GUI-subsystem process produced no captured console text, so the first world-list assertion failed before the workflow checked the profile files. Linux build/tests/package/upload passed; there was no Windows artifact.

The latest change is restoring the Run #116 tested versions of `porting.cpp`, `porting.h`, `test_filesys.cpp`, and the workflow:
- TestFileSys returns to 16 cases, including the `bin/..` path normalization regression and direct CopyDir root/nested file test.
- The path helper normalizes the share directory before deriving the portable profile.
- Migration uses process-specific staging paths with collision suffixes and retains stale staging folders.
- The Windows package job uses `--trace --worldlist path`, inspects on-disk files and file contents, compares AppData hashes, and tests stale staging and existing-profile/no-merge.

This is the exact file set from successful Run #116. Run #118 must rebuild/test both systems and repeat the Windows integration before this branch head is accepted.



## Run #118 — restored source and package gates passed

Run #118 (43385926666) tested `8896f0f3eaa7ca19a97bd0cfcfcfac5a05bf7192` and completed successfully. Both Linux and Windows passed configure/build, TestArabicTextLayout 17/17, TestFileSys 16/16, package installation and upload. The Windows trace confirmed first-run CopyFileEx migration into a process-specific stage, profile activation, second-launch reuse, AppData content preservation, stale-stage preservation, PID-stage cleanup, and existing-profile/no-merge. The package also passed path-with-spaces checks.

Windows artifact 14018204334: 20,682,129 bytes; SHA-256 `88277ca613a04ffa255bedbc538229a260f32b94bc5bcb3558cc2402d45f8571`. Inner Windows ZIP: 20,792,171 bytes; SHA-256 `36062ddb228fa9fec9bb864036012d4ac6a93e75c7e50f031b26a462baeebb39`; ZIP integrity passed with 533 entries. Linux artifact 14018973226: 22,731,333 bytes; SHA-256 `9072268c9b3a87bb75a5a01bd0c09fd713733e637c2ad01e9a12575ce7f38c6f`.

The remaining Phase 4 acceptance is manual Windows 10 GUI and storage use: launch the ZIP, Content tab download, new world, saved settings, logs/screenshots/texture/cache, no new AppData writes, and a non-system/removable drive if practical.

## Mixed-font baseline and line-height regression — pending CI

TestArabicTextLayout now contains 18 cases. The added mixed-font test loads the real Arimo primary face and Noto Sans Arabic fallback, shapes the line "Luanti مرحبا 123", verifies that both faces supplied glyphs and checks the rasterized glyph bounds against the shared line box. A separate synthetic-metrics case verifies the maximum-ascender/deepest-descender calculation.

Both Linux and Windows workflow summaries now require:
`Module TestArabicTextLayout passed (0 failures / 18 tests)`.

The production draw path and getDimension use one baseline for every face in the fallback chain. Line height covers the greatest nominal font height and the combined maximum ascender/deepest descender, plus one pixel of clearance. This avoids the old per-face y placement and reduces bottom clipping caused by measuring only the primary Latin face.

These checks are pending the next full Linux/Windows build and package CI. The actual client must still be visually retested for short labels, inventory text, loading/progress messages and mixed Arabic/English/digit lines.

## Run #119 — mixed-font baseline CI accepted

Both platform jobs passed. TestArabicTextLayout is now 18/18 and TestFileSys remains 16/16 on Windows and Linux. The Windows package smoke also passed migration and profile preservation checks. Artifact IDs, sizes and SHA-256 digests are recorded in the Run #119 section of PROJECT_STATE.md.

Visual acceptance remains pending on the user's Windows installation: recheck small one-line labels, inventory text, progress/loading messages and mixed Arabic/English/digit strings using the fresh ZIP. CI verifies the metrics contract and package, not every fixed-size GUI clipping rectangle.

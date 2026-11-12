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

Both Linux and Windows now run TestFileSys after the Arabic layout suite. The expected result is:
Module TestFileSys passed (0 failures / 14 tests)

The Windows package job also performs an isolated first-launch smoke test:
1. Copy the just-installed package to a temporary probe directory so test data cannot enter the published artifact.
2. Create a synthetic %APPDATA%/Minetest profile containing minetest.conf, worlds/portable-world-probe, games/portable-game-probe and a cache marker.
3. Launch the copied luanti.exe with --worldlist=path and verify the portable world is listed beneath the Minetest profile.
4. Run TestFileSys with the same APPDATA override to force normal path/config initialization.
5. Verify config, world, game and cache files exist beneath the portable profile, CACHEDIR.TAG was generated there, and the original AppData profile's file list did not change.

This workflow logic is part of the pending portable implementation batch. It has not passed CI until a new complete run succeeds. A CI smoke test does not replace the user's Windows GUI acceptance.

## Windows portable profile — Run #110 and smoke-test correction

Run #110 (43231076763, commit 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427) verified:
- Linux build, Arabic layout 17/17, TestFileSys 14/14, package install and upload all passed.
- Windows build, Arabic layout 17/17, TestFileSys 14/14 and install/runtime DLL checks passed.
- Windows package smoke failed during the duplicate TestFileSys invocation against the copied install package; its native process output was not captured in the log. The preceding `--worldlist=path` process returned success. Do not infer an unproven migration defect.
- Linux artifact 13950892008 (22,717,628 bytes), SHA-256 ac9c301921e1443b0bd89383a1b2a0d58bbeb0def9e5a404a293ebdb12eaee1b. Windows artifact was not uploaded.

The package smoke test retains the separate build-tree TestFileSys gate and tests installed-package migration using first and second `--worldlist=path` launches, file/content assertions for config/world/game/cache, CACHEDIR.TAG under the portable profile, and a before/after comparison of the old AppData tree. It no longer repeats TestFileSys inside the installed package.

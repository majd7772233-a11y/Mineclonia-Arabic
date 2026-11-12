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

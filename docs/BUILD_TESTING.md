# البناء والاختبار والتحقق

## قواعد قبل البناء
اقرأ AGENTS.md وPROJECT_STATE.md. أصلح workflow الحالي قبل تشغيله. حافظ على actions/cache/restore@v4 وactions/cache/save@v4 على Linux وWindows. اجمع الإصلاحات ثم نفذ CI واحداً. لا تدفع أثناء run مهم لأن cancel-in-progress=true.

## الفحوص السريعة
من جذر المستودع:

    python tools/validate_luanti_arabic.py

    python -m pip install --disable-pip-version-check --no-cache-dir luaparser==4.2.0
    python tools/validate_lua_syntax.py

    python tools/validate_arabic_test_source.py

PO formatting:

    msgfmt --check --check-format -o /tmp/luanti-ar.mo luanti/code/po/ar/luanti.po

Lua validator يجب أن يعدل regex قبل الاعتماد عليه. Arabic source validator يجب أن يؤكد 17 declaration و17 registration و17 definition.

## Linux
الإعدادات:
- triplet x64-linux.
- CMake 4.4.4.
- Ninja.
- vcpkg baseline ddf0518c31a259dcc359fd4c7bcb1ce8a27e9855.
- cache folder .vcpkg-binary-cache.

بعد تجهيز vcpkg:

    cmake -S . -B build -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
      -DVCPKG_TARGET_TRIPLET=x64-linux \
      -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_CLIENT=ON \
      -DBUILD_SERVER=OFF \
      -DBUILD_UNITTESTS=ON \
      -DBUILD_DOCUMENTATION=OFF \
      -DENABLE_SOUND=ON \
      -DENABLE_POSTGRESQL=OFF \
      -DENABLE_CURSES=OFF \
      -DRUN_IN_PLACE=ON

    cmake --build build --parallel 2
    ./bin/luanti --run-unittests --test-module TestArabicTextLayout

النتيجة المطلوبة:
Module TestArabicTextLayout passed (0 failures / 17 tests)

بعدها cmake --install build --prefix إلى مجلد package مؤقت. تحقق من executable وbuiltin/common/settings/dlg_settings.lua ومطابقته للمصدر، وخط Noto Sans Arabic والترخيص، ثم أنشئ tar.gz وارفع Artifact.

## Windows
- runner windows-2022.
- Visual Studio 17 2022 x64.
- triplet x64-windows.
- CMake 4.4.4.
- cache restore/save.

PowerShell configuration الأساسية بعد إعداد vcpkg:

    $toolchain = Join-Path $env:VCPKG_ROOT "scripts/buildsystems/vcpkg.cmake"
    $cmakeArgs = @(
      "-S", ".", "-B", "build",
      "-G", "Visual Studio 17 2022", "-A", "x64",
      "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
      "-DVCPKG_TARGET_TRIPLET=x64-windows",
      "-DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON",
      "-DBUILD_CLIENT=ON", "-DBUILD_SERVER=OFF",
      "-DBUILD_UNITTESTS=ON", "-DBUILD_DOCUMENTATION=OFF",
      "-DENABLE_SOUND=ON", "-DENABLE_POSTGRESQL=OFF",
      "-DENABLE_CURSES=OFF", "-DRUN_IN_PLACE=ON"
    )
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    cmake --build build --config Release --parallel 2
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    & "bin/Release/luanti.exe" --run-unittests --test-module TestArabicTextLayout
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

يجب أن يشترط workflow summary 17/17. بعد ذلك cmake --install build --config Release --prefix. تحقق من bin/luanti.exe، Lua file hash comparison مع المصدر، comma-terminated Arabic option، Noto font/license، وDLLs الخاصة بـRaqm/HarfBuzz/FriBidi/OpenAL/Ogg/Vorbis. أنشئ ZIP وارفعه فقط بعد كل الفحوص.

**لا تنسخ PowerShell Select-String الحالي**: الشرط مبتور ويحتاج إعادة كتابة كاملة.

## CMake/vcpkg نقاط حساسة
- Linux static Raqm link closure من raqm.pc مع pkg-config --static.
- find_package(ZLIB MODULE REQUIRED) لإنشاء ZLIB::ZLIB.
- لا تخلط plain وkeyword target_link_libraries على الهدف luanti.
- Vorbis targets: Vorbis::vorbisfile وVorbis::vorbis وOgg::ogg.
- OpenAL::OpenAL وfmt::fmt.
- حافظ على dependencies العابرة للربط.

## قراءة نتائج CI
لكل تشغيل افحص run status/conclusion، كل jobs، أول خطأ أصلي، عدد الاختبارات والـsummary، artifacts IDs وSHA/digest، والـhead SHA المبني. لا تعتبر نجاح validation وحده أو نجاح منصة واحدة دليلاً على نجاح المشروع.

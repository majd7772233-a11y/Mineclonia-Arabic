# الحالة التنفيذية الحالية للمشروع

آخر تدقيق: 2026-11-12
المستودع: majd7772233-a11y/Mineclonia-Arabic
فرع العمل الوحيد: work/arabic-engine-support
رأس تنفيذ إصلاح Workflow وLua validator: 69c62784baf96e3456239d5b954055be5e855501
رأس main المفحوص: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (لم يُعدّل).
الأب المباشر لدفعة الإصلاح: 1c5bab319be89eb2aeba40f6626c1333dd495a7a.
هذا التحديث توثيقي فقط، مبني فوق رأس التنفيذ أعلاه؛ اقرأ HEAD النهائي من GitHub بعد تسجيله.

## 1. الخلاصة التنفيذية

المستودع يحتوي مسارين منفصلين:
- تعريب Mineclonia: 163 ملف PO و5,018 ترجمة غير فارغة ومفتاح تنسيقي واحد. حزمة v7 Pre-release تاريخية ولم تُدمج في Weblate.
- دعم العربية الأصلي داخل Luanti 5.17.0 باستخدام Raqm وHarfBuzz وFriBidi وFreeType وNoto Sans Arabic. أصبحت أول دفعة Desktop build/test/package ناجحة معاً على Linux وWindows. ما زال اختبار GUI والتفاعل على عميل فعلي غير مكتمل.

## 2. Run #109 — الدليل الكامل

- Run number: 109
- Run ID: 43212062926
- HEAD SHA: 69c62784baf96e3456239d5b954055be5e855501
- الحدث: push
- بدأ: 2026-11-12T11:46:47Z
- اكتمل: 2026-11-12T11:57:15Z
- الحالة: completed / success
- رابط التشغيل: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926

### نتائج الوظائف الأربع

| الوظيفة | Job ID | النتيجة |
|---|---:|---|
| Verify GitHub Actions job scheduling | 131161916735 | success |
| Validate Arabic engine catalogs and sources | 131161957086 | success |
| Linux x64 build, tests, and install package | 131162246258 | success |
| Windows x64 MSVC build, tests, and install package | 131162246670 | success |

### Catalog / Lua / PO / C++ source
- builtin: 245 template keys و245 Arabic keys، صفر فارغ، والنسخة المعبأة byte-identical.
- Luanti gettext: 1,575 رسالة، 1,575 فعالة، 0 غير مترجمة، 0 fuzzy، وLANG_CODE='ar'.
- msgfmt --check --check-format: success.
- Lua parser: 210 ملفات builtin تم تحليلها بنجاح عبر شجرتي المصدر والنسخة المرجعية.
- validate_arabic_test_source.py: 17 اختباراً معلناً ومسجلاً ومُعرّفاً مرة واحدة.

### Linux
- vcpkg bootstrap/cache restore/install: success؛ save cache تخطى لأن cache hit كان true.
- CMake Configure: success.
- Client Build: success.
- TestArabicTextLayout: 17/17، 0 failures. سجل الوظيفة يحتوي 17 سطر PASS والملخص الحرفي:
  `Module TestArabicTextLayout passed (0 failures / 17 tests)`.
- cmake --install: success.
- تم فحص وجود executable وbuiltin؛ ملف dlg_settings.lua المعبأ مطابق للمصدر بـcmp.
- تم فحص Noto Sans Arabic والترخيص؛ ldd لم يُظهر dependency بحالة not found.
- Package/Artifact upload: success.

**Linux Artifact**
- الاسم: luanti-arabic-linux-x64
- ID: 13941099252
- الحجم: 22,718,197 bytes
- SHA-256: `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`
- الرابط: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941099252

### Windows
- vcpkg bootstrap/cache restore/install: success؛ save cache تخطى لأن cache hit كان true.
- CMake Configure: success.
- MSVC Client Build: success.
- خطوة TestArabicTextLayout: success. PowerShell step لا تعتبر الاختبار ناجحاً إلا بعد exit code صفر والعثور على النص الحرفي `Module TestArabicTextLayout passed (0 failures / 17 tests)`.
- cmake --install: success.
- تم فحص تطابق SHA-256 للـbuiltin المصدر والمعبأ، والفاصلة بعد خيار العربية قبل be، ووجود exe وخط Noto والترخيص وDLLs الخاصة بـRaqm/HarfBuzz/FriBidi/OpenAL/Ogg/Vorbis.
- ZIP/Artifact upload: success.

**Windows Artifact**
- الاسم: luanti-arabic-windows-x64
- ID: 13941743588
- الحجم: 20,672,848 bytes
- SHA-256: `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`
- الرابط: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941743588

لم يُرفع artifact جديد أثناء Run #109 إلا الحزمتان أعلاه. لا تستخدم artifacts الأقدم لتمثيل HEAD 69c62784baf96e3456239d5b954055be5e855501.

## 3. إصلاحات دفعة التنفيذ

Commit التنفيذ 69c62784baf96e3456239d5b954055be5e855501 (Repair Arabic engine CI workflow and Lua validator) أصلح بنية Workflow إلى أربع Jobs فريدة، وأزال package fragments/Windows jobs المكررة، وأكمل شروط Bash/PowerShell. حافظ على cancel-in-progress=true وعلى cache restore/save في النظامين. وصُحح double escaping في regex الخاص بـtools/validate_lua_syntax.py. لم تُخفّض assertions ولم يُحذف اختبار ولم يتغير C++ production shaping في هذه الدفعة.

## 4. الحالة غير المنجزة

- لم يُختبر تشغيل Windows GUI على جهاز المستخدم.
- لم يُثبت بعد اختفاء Lua menu error أو SHA warnings في تشغيل فعلي من الحزمة الجديدة.
- لم تكتمل اختبارات EditBox للحافظة/السحب/Shift/اختصارات الكلمات على عميل فعلي.
- لم تكتمل اختبارات عرض clipping/shadows/centering/alignment/scrolling وemoji/supplementary Unicode.
- Android مؤجل بعد تثبيت Desktop.

## 5. نقطة العمل التالية

1. استخدم حزمة Windows الحديثة أعلاه كاملة، لا تستبدل exe وحده في تثبيت قديم.
2. افتح القائمة الرئيسية ثم Settings > Language وتحقق من خيار العربية.
3. راقب سجل التشغيل بحثاً عن Lua parser errors وNo SHA256 known warnings؛ أرسل السجل إذا ظهر شيء.
4. نفذ/وثّق اختبارات clipboard copy/cut/paste وmouse drag وShift selection والتنقل والحذف بالنص المختلط.
5. تابع مصفوفة Rendering GUI، ثم Android بعد استقرار Desktop.
6. أي تعديل جديد يجب أن يضم تحديث Markdown في commit نفسه، وألا يدفع أثناء CI لأن cancel-in-progress=true.

لا يُعد دعم العربية مكتملاً بعد؛ نجاح Desktop build و17/17 والحزمتين إنجاز مهم لكنه لا يعوض تجربة GUI والتفاعل على عميل فعلي.


## 6. فحص مستقل للحزم بعد تنزيل Artifacts — 2026-11-12

تم تنزيل الـArtifactين من Run #109 إلى بيئة الفحص، ثم فك أرشيف GitHub الخارجي والأرشيف الداخلي.

### مطابقة Artifact SHA-256

- Linux Artifact 13941099252: بصمة الملف المنزّل `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`، مطابقة تماماً لبصمة upload action في سجل Run #109. الأرشيف الخارجي ZIP سليم، ويحتوي tar.gz داخلياً.
- Windows Artifact 13941743588: بصمة الملف المنزّل `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`، مطابقة تماماً لبصمة upload action في سجل Run #109. الأرشيف الخارجي ZIP سليم، ويحتوي ZIP الحزمة المحمولة داخلياً.
- لم يفشل اختبار سلامة أعضاء ZIP الخارجي؛ `ZipFile.testzip()` أعاد `None` لكليهما.

### Linux extracted package

- `bin/luanti`: ELF 64-bit x86-64.
- `ldd bin/luanti`: لا توجد مكتبة runtime بحالة `not found`.
- تشغيل `./bin/luanti --version` من جذر الحزمة نجح exit code 0، وأظهر Luanti 5.17.0 / Release / RUN_IN_PLACE=1 / USE_CURL=1 / USE_GETTEXT=1 / USE_SOUND=1.
- `builtin/common/settings/dlg_settings.lua` موجود ويحوي السطر `ar = "العربية [ar]",`.
- `builtin/locale/__builtin.ar.tr` موجود.
- `fonts/NotoSansArabic-Regular.ttf` موجود (844,676 bytes)، و`fonts/NotoSansArabic-OFL.txt` موجود (4,382 bytes).
- شغّلت الاختبارات من النسخة المستخرجة نفسها: `./bin/luanti --run-unittests --test-module TestArabicTextLayout`.
- النتيجة الفعلية: 17 سطراً PASS، والملخص `Module TestArabicTextLayout passed (0 failures / 17 tests)`، ثم `Unit Test Results: PASSED` و`0 / 17 failed tests`.

### Windows extracted package

تم فك ZIP وفحص وجود الملفات التالية وأحجامها:
- `bin/luanti.exe` (9,920,000 bytes).
- DLLs: `raqm-0.dll`، `harfbuzz.dll`، `fribidi-0.dll`، `freetype.dll`، `OpenAL32.dll`، `ogg.dll`، `vorbis.dll`، `vorbisfile.dll`.
- `builtin/common/settings/dlg_settings.lua` ويحوي خيار العربية مع الفاصلة.
- `fonts/NotoSansArabic-Regular.ttf` و`fonts/NotoSansArabic-OFL.txt`.

هذا الفحص يثبت سلامة تنزيل Artifact وحضور الملفات المحددة، لكنه **لا** يثبت تشغيل exe على Windows أو فتح GUI؛ لم يُنفّذ تشغيل Windows محلياً.

## 7. نقطة التوقف الحالية

1. الاختبار المستقل للحزمة Linux و17/17 اكتمل.
2. الاختبار المستقل للحزمة Windows يقتصر على سلامة الأرشيف وفحص محتويات الملفات؛ التشغيل الفعلي لم يُختبر.
3. الخطوة الخارجية التالية هي تشغيل حزمة Windows كاملة على جهاز المستخدم، فتح القائمة ثم Settings > Language، وجمع log جديد للتحقق من Lua parser وSHA warnings.
4. بعد ذلك تستكمل اختبارات clipboard/drag/Shift/keyboard والـrendering. Android يظل مؤجلاً.

هذا التحديث توثيقي فقط، مبني على تنفيذ 69c62784baf96e3456239d5b954055be5e855501؛ HEAD الوثائقي السابق كان 596c9dd10e98f37057f87a92873d77a21ebc4a95. اقرأ HEAD النهائي مباشرة من GitHub بعد هذا التحديث.

# الحالة التنفيذية الحالية للمشروع

آخر تدقيق: 2026-11-12
المستودع: majd7772233-a11y/Mineclonia-Arabic
فرع العمل الوحيد: work/arabic-engine-support
رأس بداية دفعة الإصلاح: 1c5bab319be89eb2aeba40f6626c1333dd495a7a
main الذي تم فحصه: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (لم يُعدّل).

## 1. GitHub وRun 108

- Run #108، ID 43201985097، event push، head SHA 1c5bab319be89eb2aeba40f6626c1333dd495a7a، completed/failure عند 2026-11-12T10:37:19Z.
- Jobs API: total_count=0. Artifacts API: total_count=0. check-runs: total_count=0.
- Combined status أعاد pending بلا statuses؛ لا توجد job logs تشرح الفشل. لا يمكن تصنيفه كفشل Compilation أو Tests أو Packaging.
- آخر commit موثق: 1c5bab319be89eb2aeba40f6626c1333dd495a7a — Document authoritative project state and continuation workflow.
- آخر commit برمجي سابق: 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2 — Remove duplicate Arabic tests and verify packaged builtin integrity.
- لا يوجد بناء حديث أو اختبار فعلي أو Artifact مطابق لرأس الفرع حتى الآن.

## 2. المساران المستقلان

### تعريب Mineclonia
163 ملف PO، و5,018 ترجمة غير فارغة ومفتاح تنسيقي واحد. المصدر والقوالب في SOURCE_VERSION.md وSOURCE_TEMPLATES.md. حزمة v7 Pre-release تاريخية، غير مدمجة في Weblate.

### دعم العربية الأصلي داخل Luanti 5.17.0
يعتمد Raqm وHarfBuzz وFriBidi وFreeType وNoto Sans Arabic. يغطي المصدر shaping وBiDi/RTL والتحويلات النصية ومؤشر/تحديد/editbox. هذا المسار غير مكتمل حتى نجاح Linux وWindows build و17/17 والـpackage والتحقق الفعلي.

## 3. ما تم فحصه مباشرة في المصدر

- Workflow القديم طوله 748 سطراً، ويحتوي grep مقتبساً بشكل غير مكتمل، وشرط PowerShell مبتوراً، ومقاطع Package يتيمة ومكررة، وتعريف Windows jobs مكرر.
- الفاصلة بعد ar موجودة فعلياً في نسختي dlg_settings.lua؛ لم يتم تعديل مصدر Lua السليم.
- validate_lua_syntax.py احتوى double escaping للأقواس المربعة؛ صُحح النمط إلى escaping واحد.
- test_arabic_text_layout.cpp: 17 declarations و17 registrations و17 definitions بلا أسماء مكررة.
- normalizeBuiltinFileSha256Path موجود ويُستخدم قبل البحث في g_builtin_file_sha256_map داخل safeLoadFile.
- لم يتم تعديل C++ tests أو assertions أو CMake أو كود الإنتاج في دفعة الإصلاح الحالية.

## 4. Workflow والإثبات الساكن

أعيد تنظيم Workflow بأربع Jobs فقط: workflow-sanity، catalog-validation، linux، windows. حُفظ cancel-in-progress=true وكاش vcpkg restore/save لكل منصة وbaseline الأصلي وإعداد ENABLE_SOUND=ON. يرفض الاختبار أي ملخص غير 0 failures / 17 tests. يتحقق Linux من ldd، والنسختان من ملف builtin والخط والترخيص، وWindows من DLLs.

Preflight محلي للنسخة المرشحة:
- YAML parsing عبر PyYAML: PASS.
- bash -n لجميع Bash run blocks: PASS.
- بنية Jobs الأربع وعدم التكرار: PASS.
- زوجا actions/cache/restore@v4 وactions/cache/save@v4: محفوظان.
- regex خيار العربية يطابق السطرين الحاليين ويرفض غياب الفاصلة: PASS.
- actionlint وPowerShell parser وmsgfmt غير متاحة محلياً.
- تعذر تثبيت luaparser==4.2.0 بسبب DNS؛ لم يُشغل Lua parser محلياً.
- لم يُجر CMake configure/build أو C++ tests أو package install.

هذه فحوص ساكنة وليست دليلاً على CI أو بناء ناجح.

## 5. نتائج CI التاريخية

| Run | SHA | الأدلة |
|---|---|---|
| #102 / 42774549330 | 9bdd876eea9ab8087906b4d067d76fd587e84fc8 | Windows build و16/16 وpackage نجحت؛ Linux build نجح و4/16 shaping tests فشلت |
| #105 / 42782985200 | 9eda41ba6285afb6ef0891a4493ac9cf35b32d99 | Windows build و16/16 وpackage نجحت؛ Linux build نجح و4/16 فشلت |
| #106 / 42798177083 | 3a6fe0e7e179e01c3562869cb590c6cdb6d14131 | Catalog نجح؛ compilation فشل على Linux وWindows بسبب duplicate test definitions |
| #107 / 42805300100 | 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2 | failure، jobs=0 وartifacts=0 |
| #108 / 43201985097 | 1c5bab319be89eb2aeba40f6626c1333dd495a7a | failure، jobs=0 وartifacts=0 وcheck-runs=0؛ لا logs |

لا تستخدم Artifacts التاريخية على أنها تمثل HEAD الحالي.

## 6. نقطة التوقف التالية

1. تأكد من الرأس الجديد على GitHub ومن ثبات main.
2. افحص Run الناتج عن دفعة الإصلاح على SHA المطابق.
3. تحقق من catalog/Lua/PO ثم configure/build وملخص 17/17 وpackage على المنصتين.
4. استخرج السبب الأصلي من أي Job فاشل واقرأ سياق الكود قبل الإصلاح.
5. لا تدفع commit أثناء Run بسبب cancel-in-progress=true.
6. بعد Windows Artifact حديث، يختبر المستخدم القائمة واللغة وتحذيرات SHA مع الحزمة الكاملة.
7. تظل clipboard/drag/Shift/rendering وAndroid أعمالاً لاحقة.

لا تعديل على main ولا فروع جديدة ولا Force Push.

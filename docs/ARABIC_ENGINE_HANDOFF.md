# تسليم دعم العربية في محرك Luanti — نقطة الاستئناف الرسمية

التاريخ: 2026-11-12
Repository: majd7772233-a11y/Mineclonia-Arabic
Branch الوحيد: work/arabic-engine-support
رأس البداية: 1c5bab319be89eb2aeba40f6626c1333dd495a7a
main الذي تم فحصه: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (لم يُعدّل).

## ما تم التحقق منه

- Run #108، ID 43201985097، على SHA 1c5bab319be89eb2aeba40f6626c1333dd495a7a: completed/failure، لكن API أعاد Jobs=0 وArtifacts=0 وcheck-runs=0؛ لا توجد job logs تحدد سبباً مباشراً.
- تمت قراءة Workflow كاملاً: تضمن grep غير مغلق، وPowerShell condition مبتوراً، وpackage tails مكررة، وWindows jobs مكررة.
- كلا ملفي dlg_settings.lua يحتويان فاصلة العربية قبل be.
- regex داخل validate_lua_syntax.py كان يهرب الأقواس المربعة مرتين؛ تم تصحيحه.
- C++ source يحوي 17 declarations و17 registrations و17 definitions بلا تكرار.
- normalizeBuiltinFileSha256Path موجود ويستخدم قبل manifest lookup في safeLoadFile.

## إصلاح Workflow والفاحص

Workflow أصبح بأربع Jobs: workflow-sanity وcatalog-validation وlinux وwindows. تم الحفاظ على cancel-in-progress وvcpkg cache restore/save للمنصتين وENABLE_SOUND=ON. هناك شرط صريح لنتيجة 17/17 قبل التغليف، والتحقق من builtin/font/license والاعتماديات.

Preflight: YAML parse وbash -n وregex نجحت. actionlint وPowerShell parser وmsgfmt غير متاحة محلياً. تعذر تثبيت luaparser بسبب DNS؛ يجب أن يثبت CI فحص Lua قبل البناء.

## غير مثبت

- لا CMake configure أو client build حديث.
- لا اختبارات C++ فعلية 17/17 على Linux أو Windows.
- لا install package أو Artifact حديث.
- لا اختبار GUI من المستخدم ولا تحقق من SHA warnings على جهازه.

## الخطوة التالية

بعد وصول دفعة الإصلاح إلى GitHub:
1. افحص HEAD وmain مباشرة.
2. افحص Run الجديد المطابق لـHEAD، لا Run108.
3. راجع كل Jobs وlogs، وأول خطوة فاشلة، ثم Catalog/Lua/PO وConfigure/Build/17 tests/Package.
4. لا تدفع أثناء تشغيل CI لأن cancel-in-progress=true.
5. أصلح السبب الجذري في دفعة متماسكة مع تحديث Markdown في نفس commit.
6. بعد Artifact Windows حديث، اختبر الحزمة الكاملة والقائمة وSettings > Language وسجل SHA.
7. تابع clipboard/drag/Shift/rendering، ثم Android بعد استقرار Desktop.

لا دمج إلى main ولا فروع جديدة ولا Force Push.

# رسالة استئناف محدثة — Mineclonia-Arabic

تابع المستودع majd7772233-a11y/Mineclonia-Arabic من GitHub HEAD الحالي فقط. الفرع المسموح الوحيد work/arabic-engine-support. ممنوع تعديل main أو إنشاء فروع أو Force Push.

## الفحص الأول
- اقرأ رأسي work/arabic-engine-support وmain من GitHub مباشرة.
- أعد قراءة وثائق المشروع الإلزامية كاملة، ثم تحقق من العدد الحالي لملفات Markdown (كان 66).
- راجع Run108 ID 43201985097 على SHA 1c5bab319be89eb2aeba40f6626c1333dd495a7a: completed/failure، لكن Jobs=0 وArtifacts=0 وcheck-runs=0، ولا توجد logs لوظيفة.

## حالة المصدر
- آخر commit برمجي سابق: 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2.
- آخر commit توثيق سابق: 1c5bab319be89eb2aeba40f6626c1333dd495a7a.
- اختبارات C++: 17 declarations و17 registrations و17 definitions بلا تكرار.
- فاصلة العربية قبل be موجودة في نسختي dlg_settings.lua.
- normalizeBuiltinFileSha256Path موجود ويستخدم قبل SHA manifest lookup.
- Lua validator regex كان double-escaped للأقواس المربعة، وتم تصحيحه في دفعة الإصلاح الحالية.

## Workflow
أعيد تنظيمه بأربع Jobs: workflow-sanity، catalog-validation، linux، windows. حُفظ vcpkg cache restore/save على المنصتين وcancel-in-progress=true وENABLE_SOUND=ON. يشترط ملخص 0 failures / 17 tests قبل package. يتحقق من Lua builtin/font/license والـruntime dependencies.

## Preflight مسجل
- YAML parsing: PASS.
- bash -n لكل كتل Bash: PASS.
- Jobs الأربع مرة واحدة، cache restore/save مرتين لكل action: PASS.
- regex يطابق النص الفعلي ويرفض غياب الفاصلة: PASS.
- actionlint وPowerShell parser وmsgfmt غير متاحة محلياً.
- تعذر تنزيل luaparser==4.2.0 بسبب DNS؛ Lua parsing الكامل يجب أن يحدث في CI.
- لا build أو C++ test أو package محلي حتى الآن.

## المطلوب تالياً
افحص Run الجديد على SHA الإصلاح تحديداً. تحقق من Catalog/Lua/PO ثم CMake configure/build والاختبارات 17/17 على كل منصة والحزم. لا تنسب Run108 إلى compilation لأنه لم يحتو Jobs. عند أي فشل، اقرأ السجل والكود المحيط به وأصلح السبب الجذري، ثم وثّق في commit نفسه. لا تدفع أثناء تشغيل CI.

Definition of Done: Validators ناجحة، Linux وWindows build ناجحان، 17/17 على كليهما، packages verified مع Artifact IDs وSHA-256، ثم تجربة GUI فعلية من المستخدم. تبقى clipboard/drag/Shift/rendering أعمالاً لاحقة.

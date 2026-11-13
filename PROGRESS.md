# تقدم التعريب

> ## نقطة حالة محرك Luanti — 2026-11-13 · Run #115 (Windows copy/trace diagnosis pending)
>
> Branch: `work/arabic-engine-support`; tested HEAD: `dab127e225c2083f37255ed4d829d207506aad4d`; `main` remains `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.
>
> Run #115 اجتاز scheduling/catalog validation. اجتاز Linux configure/build واختبار العربية 17/17 وTestFileSys 16/16 وpackage/upload. Artifact Linux: 13979992497، الحجم 22,730,451 bytes، SHA-256 `6f00827ab6f3de6dca56f195e857881116993271838d03c279d6ec4b0685c579`.
>
> اجتاز Windows configure/build واختبار العربية 17/17 وTestFileSys 16/16. شغّل فحص الحزمة `--worldlist path` مرتين وأظهر مسار العالم تحت `Minetest/worlds`، لكن فحص ما بعد التشغيل لم يجد الملفات الخمسة المطلوبة وكانت قائمة ملفات الوجهة فارغة. مصدر AppData التجريبي احتوى الملفات الأربعة كما هو متوقع؛ لم يُرفع Windows Artifact.
>
> التصحيح التالي يطابق اختبار `CopyDir` مع staging موجود مسبقاً، ويفعّل `--trace` أثناء تشغيل الحزمة لكشف مسارات التهيئة وعمليات CopyFileEx وتفعيل rename، ويقارن جرد PowerShell و.NET للمجلد الوجهة.
>
> ما زلنا في Phase 3 — اختبارات الحزمة والتوثيق. لا تبدأ Phase 4 الفعلية أو إصلاح الخطوط/UI قبل نجاح Windows smoke ورفع Windows Artifact.
>
> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).

> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).

# تقدم التعريب


> ## نقطة حالة محرك Luanti — 2026-11-12 · Run #113 (Windows staging collision correction pending)
>
> Branch: `work/arabic-engine-support`; tested HEAD: `e0b1fc5bf517f216523f5a53d3d6b049673432d0`; `main` remains `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.
>
> Run #113 نجح في scheduling/catalog validation. Linux: configure/build، اختبار العربية 17/17، TestFileSys 15/15، package/install/upload كلها نجحت. Artifact Linux: 13957134160، الحجم 22,723,769 bytes، SHA-256 `9b4178bba4ac2e284e51b85b170a3311b5d8f4adb753cd9323be6f67eb290ee6`.
>
> Windows: configure/build واختبار العربية 17/17 وTestFileSys 15/15 نجحت. فشل package smoke بعد الوصول لتهيئة المسارات لأن staging الثابت `Minetest.migration-tmp` كان موجوداً بجانب مسار `bin/..`؛ لم يُفعّل ملف المستخدم ولم يُرفع Windows Artifact. لا يثبت سجل التشغيل مصدر المجلد القديم.
>
> التصحيح الحالي يطبّع مسار الحزمة إلى مسار مطلق، ويستخدم staging فريداً لكل عملية مع suffix عند التصادم ويحافظ على المجلد القديم، ويضيف اختبار C++ لمسار `bin/..` وفحص Windows لبقاء marker وعدم ترك staging جديد.
>
> لا تعتبر قابلية النقل مثبتة قبل نجاح CI جديد ورفع Windows Artifact ثم الاختبار اليدوي على Windows. أبقِ إصلاح الخطوط/UI مؤجلاً.
>
> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).
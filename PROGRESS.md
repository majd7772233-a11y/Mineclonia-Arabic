# تقدم التعريب

> ## نقطة حالة محرك Luanti — 2026-11-12 · Run #114 (Windows root-config copy diagnosis pending)
>
> Branch: `work/arabic-engine-support`; tested HEAD: `334f6fba00c9a093b2e5511a2987f45aab91296e`; `main` remains `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.
>
> Run #114 نجح في scheduling/catalog validation. Linux: configure/build، اختبار العربية 17/17، TestFileSys 15/15، package/install/upload كلها نجحت. Artifact Linux: 13976243943، الحجم 22,728,834 bytes، SHA-256 `5966c9a2f7be474973eaa12881970ae71778b76a8aee77b6549f8d933a5ef1bb`.
>
> Windows: configure/build واختبار العربية 17/17 وTestFileSys 15/15 نجحت. أمر `--worldlist path` أظهر العالم التجريبي مرتين تحت `Minetest/worlds`، لكن فحص الملفات لم يجد `Minetest/minetest.conf`؛ لم يُرفع Windows Artifact. لا يحدد سجل Run #114 إن كان الملف غير موجود في مصدر AppData التجريبي أم لم يُنسخ.
>
> الإصلاح التالي يضيف اختبار `fs::CopyDir` مباشر لملف إعداد جذري وعالم متداخل داخل مسارات بها مسافات، ويرفع TestFileSys إلى 16/16، ويطبع قائمة ملفات المصدر والوجهة عند الفشل. لا تغيّر كود الترحيل قبل معرفة مكان ضياع الملف.
>
> ما زلنا في Phase 3 — اختبارات الحزمة والتوثيق. تبقى Phase 4 بحاجة لاختبار ZIP على Windows حقيقي. أبقِ إصلاح الخطوط/UI مؤجلاً.
>
> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).

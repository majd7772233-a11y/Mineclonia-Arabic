# تقدم التعريب

> ## نقطة حالة محرك Luanti — 2026-11-13 · Run #116 ناجح؛ Phase 4 الفعلية قيد الاختبار
>
> Branch: `work/arabic-engine-support`; tested HEAD: `b0c486db3a0960c7779c2f341198fe267356f61b`; `main` remains `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.
>
> Run #116 (ID 43376273868) اجتاز Jobs الأربعة: scheduling، catalog validation، Windows x64، Linux x64. اجتاز Windows وLinux CMake/build وTestArabicTextLayout 17/17 وTestFileSys 16/16، كما نجح install/package/upload على النظامين.
>
> أثبت سجل Windows `--trace` نسخ ملفات AppData الأربعة بواسطة CopyFileEx إلى staging خاص بالعملية، وتفعيل `Minetest`، ثم استخدامه في التشغيل الثاني. نجحت فحوص محتوى config/world/game/cache وCACHEDIR.TAG، تطابق SHA-256 لمصدر AppData قبل/بعد، الحفاظ على staging القديم، تنظيف staging الجديد، المسارات ذات المسافات، واختبار عدم الدمج عند وجود Minetest مسبقاً.
>
> Windows Artifact: 14015073206، 20,682,182 bytes، SHA-256 `66e58c0a28de9b99215fb348a70f80a77a7f186875b6b97646119e348bf32ecc`. Linux Artifact: 14014987601، 22,731,184 bytes، SHA-256 `d9ee80d7af0a291bc83479870598634b530f68b301818c903efdd83e6c7428d9`.
>
> اكتملت Phases 1–3 والاختبارات الآلية الأربع في Phase 4. بقي تشغيل الحزمة واختبار GUI وتدفقات التخزين الفعلية على Windows 10، واختبار قرص غير النظام إن أمكن. لا تبدأ إصلاح الخطوط/UI قبل تسجيل نتائج هذه الخطوة.
>
> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).

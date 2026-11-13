# تقدم التعريب

> ## نقطة حالة محرك Luanti — 2026-11-13 · Run #118 ناجح؛ قبول Windows الفعلي هو التالي
>
> Branch: `work/arabic-engine-support`; tested source commit: `8896f0f3eaa7ca19a97bd0cfcfcfac5a05bf7192`; protected `main`: `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d` (unchanged).
>
> Run #118 (ID 43385926666) اجتاز Jobs الأربعة: scheduling، catalog validation، Windows x64، Linux x64. اجتاز Windows وLinux configure/build وTestArabicTextLayout 17/17 وTestFileSys 16/16 وinstall/package/upload.
>
> أثبت سجل Windows نسخ ملفات AppData الأربعة بواسطة CopyFileEx إلى staging خاص بالعملية، وتفعيل مجلد `Minetest`، واستخدامه في التشغيل الثاني. نجحت فحوص المحتوى وCACHEDIR.TAG، تطابق SHA-256 لمصدر AppData قبل/بعد، الحفاظ على staging القديم، تنظيف staging الجديد، مسارات المسافات، وعدم الدمج فوق مجلد محمول موجود.
>
> Windows Artifact: 14018204334، الحجم 20,682,129 bytes، SHA-256 `88277ca613a04ffa255bedbc538229a260f32b94bc5bcb3558cc2402d45f8571`. ZIP الداخلي: 20,792,171 bytes، SHA-256 `36062ddb228fa9fec9bb864036012d4ac6a93e75c7e50f031b26a462baeebb39`، سلامة ZIP مؤكدة (533 entries). Linux Artifact: 14018973226، 22,731,333 bytes، SHA-256 `9072268c9b3a87bb75a5a01bd0c09fd713733e637c2ad01e9a12575ce7f38c6f`.
>
> اكتملت Phases 1–3 والاختبارات الآلية في Phase 4. بقي قبول Windows 10 الفعلي: GUI، تنزيل Content tab، إنشاء عالم، حفظ config، فحص logs/screenshots/texture/cache، عدم الكتابة الجديدة إلى AppData، وتجربة قرص غير النظام إن أمكن. لا تبدأ الخطوط/UI قبل تسجيل ذلك.
>
> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).

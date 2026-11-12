# تقدم التعريب


> ## نقطة حالة محرك Luanti — 2026-11-12 · Run #112 (Windows CLI syntax correction pending)
>
> Branch: `work/arabic-engine-support`; tested HEAD: `cc9ff808542bafd0d5849203684798d74eaf2fcf`; `main` remains `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.
>
> Run #112 نجح في scheduling/catalog validation. Linux: configure/build، اختبار العربية 17/17، TestFileSys 15/15، package/install/upload كلها نجحت. Artifact Linux: 13957061595، الحجم 22,727,482 bytes، SHA-256 `660232c9ddeae37fabe2e9a2264be3fa0d0493145b96c8da8cfd76a0306a756f`.
>
> Windows: configure/build واختبار العربية 17/17 وTestFileSys 15/15 نجحت. فشل package smoke لأن الأمر استخدم `--worldlist=path`؛ المحرك يتوقع `--worldlist path` كوسيطين منفصلين. رفض محلل الأوامر الصيغة، ولم تبدأ تهيئة المسارات أو عملية ترحيل AppData. لم يُرفع Windows Artifact.
>
> التصحيح التالي يغيّر استدعاءات التشغيل الثلاثة إلى `--worldlist path` مع الإبقاء على فحوص الملفات والبصمات وعدم الدمج. لا تعتبر قابلية النقل مثبتة قبل نجاح Windows smoke ورفع الحزمة، ثم الاختبار اليدوي على Windows. أبقِ إصلاح الخطوط/UI مؤجلاً.
>
> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).

# تقدم التعريب

> ## نقطة حالة محرك Luanti — 2026-11-13 · Run #117 تعثر؛ استعادة نسخة Run #116
>
> Branch: `work/arabic-engine-support`; HEAD قبل التصحيح: `efbf5dc3b1d73b99df3ec1a14a30666540d729ca`; `main`: `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.
>
> Run #116 (ID 43376273868) نجح على Linux وWindows: بناء، TestArabicTextLayout 17/17، TestFileSys 16/16، اختبار الحزمة ورفع Artifactين. أثبت trace Windows ترحيل AppData، تفعيل Minetest، التشغيل الثاني، عدم تغيير المصدر، استعادة staging، وعدم الدمج فوق ملف محمول موجود.
>
> Run #117 (ID 43382276907) على HEAD أحدث نجح في Linux بالكامل، وفي Windows نجح البناء واختبار العربية 17/17 وTestFileSys 14/14، ثم فشل اختبار الحزمة عند مطابقة stdout الفارغ من تطبيق GUI. توقف قبل فحص ملفات القرص، فلا يثبت فشل CopyDir. لا يوجد Windows Artifact من هذا التشغيل. Linux Artifact 14017277623، 22,717,242 bytes، SHA-256 `0f6e51221dbf526c6898d4897f4bf729e7569a5540bf1c8d1f6e75e874a2fc8c`.
>
> مقارنة HEAD بـRun #116 كشفت إزالة تطبيع `bin/..`، وإرجاع staging ثابت، وحذف اختبارات CopyDir/path (16 إلى 14)، وإرجاع stdout assertion. التصحيح الحالي يستعيد الملفات الأربعة من رأس Run #116 الناجح؛ Run #118 مطلوب لإعادة القبول.
>
> لا تبدأ Phase 4 الفعلية أو الخطوط/UI قبل نجاح Run #118 وإنتاج Windows Artifact مطابق للرأس الحالي. بعده اختبر GUI وContent tab والعوالم والإعدادات ومسارات التخزين وAppData على Windows 10.
>
> التفاصيل في [خارطة الطريق](docs/ROADMAP.md)، [حالة المشروع](docs/PROJECT_STATE.md)، [سجل العمل](docs/ARABIC_ENGINE_WORKLOG.md)، و[رسالة الاستئناف](docs/NEXT_SESSION_PROMPT.md).

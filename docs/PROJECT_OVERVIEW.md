# نظرة عامة على المشروع

المستودع: majd7772233-a11y/Mineclonia-Arabic
الفرع النشط: work/arabic-engine-support
التوثيق والتواصل: العربية.

## مسارا المشروع

### تعريب Mineclonia
ملفات PO العربية هي مصادر الترجمة القابلة للمراجعة، منظمة حسب مكونات اللعبة. القوالب الإنجليزية مرجع مفاتيح msgid، والمعجم الموحد في GLOSSARY.md.

الحالة المسجلة:
- 163 ملف PO.
- 5,018 نصاً مترجماً غير فارغ، إضافة إلى مفتاح تنسيقي واحد مكون من مسافة فقط.
- قوالب المصدر من ZenonSeth/mineclonia-mirror عند commit 4ba113d1ce7afbc5b480b5b0e41b6c0037c8d8da.
- SOURCE_VERSION.md وSOURCE_TEMPLATES.md يشرحان المصدر وبصمات القوالب.
- حزمة v7 المسجلة Pre-release؛ ليست إثباتاً على دمج الترجمة في Weblate أو مطابقتها النهائية مع game release=39603 المحلي.

### دعم العربية الأصلي في Luanti
يُطوّر في نسخة Luanti 5.17.0 المضمنة في luanti/code. الهدف تشكيل العربية واتجاه RTL/BiDi وسلوك المؤشر والتحديد والتحرير داخل العميل، وليس مجرد إضافة ترجمات.

المكونات: Raqm وHarfBuzz وFriBidi وFreeType وخط Noto Sans Arabic، مع تغييرات CGUITTFont وStaticText وCGUIEditBox واختبارات TestArabicTextLayout.

نجاح الترجمة لا يثبت نجاح المحرك. وبالمثل، نجاح اختبار وحدة لا يثبت العرض البصري أو نجاح كامل upstream suite.

## الإصدارات والمراجع
- مصدر المحرك: luanti/code.
- الشجرة المرجعية المرفقة: luanti/5.17.0.
- خط UI: NotoSansArabic-Regular.ttf، الحجم المسجل 844,676 بايت.
- vcpkg baseline: ddf0518c31a259dcc359fd4c7bcb1ce8a27e9855.
- manifest: luanti/code/vcpkg.json.

وجود EXE داخل luanti/5.17.0/bin لا يثبت أنه بُني من HEAD الحالي. وجود PO لا يثبت أن ملفات runtime تعمل في كل إصدارات اللعبة. راجع PROJECT_STATE.md للحالة التنفيذية.

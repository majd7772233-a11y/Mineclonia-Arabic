# مصدر اللعبة وإصداراتها

## نسخة المستخدم المستهدفة

بحسب ملف `game.conf` الذي قدمه صاحب المشروع:

- `title = Mineclonia`
- `textdomain = mcl_game_meta`
- `min_minetest_version = 5.10`
- `release = 39603`

القيمة `39603` مسجلة هنا كقيمة `release` من ملف المستخدم، وليست رقم إصدار Luanti.

## مصدر قوالب الترجمة

- المستودع الأساسي لمشروع Mineclonia مستضاف على Codeberg: https://codeberg.org/mineclonia/mineclonia
- مرآة GitHub المستخدمة للدفعة الحالية: https://github.com/ZenonSeth/mineclonia-mirror
- تصف صفحة المرآة نفسها بأنها مرآة للمستودع الرسمي على Codeberg.
- commit المصدر المستخدم في دفعة الزراعة والطعام والصيد: `4ba113d1ce7afbc5b480b5b0e41b6c0037c8d8da`
- القوالب الجديدة وبصمات Git blob الخاصة بها محفوظة في `SOURCE_TEMPLATES.md`.
- استُخدمت لقطة أقدم `24521bf6c99ddfc82bf52a60420a23902282c2a0` للدفعات السابقة، وتظهر تفاصيلها في سجل المصدر.

لم يكن الوصول المباشر إلى Codeberg متاحاً أثناء تنفيذ هذه الدفعة؛ لذلك استخدمت مرآة GitHub التي توضح علاقتها بالمستودع الأصلي. لا يعني ذلك أن كل قالب يطابق تلقائياً نسخة اللعبة المحلية `release=39603`. ستتم مقارنة الملفات مع نسخة اللعبة المحلية قبل إصدار حزمة تثبيت.

## ملاحظة مهمة حول PO وTR

محرك Luanti يدعم ملفات Gettext بصيغة `.po` ابتداءً من 5.10.0، لكن دليل سير عمل Mineclonia في المستودع يذكر أن ملفات PO تُستخدم في سير Weblate وأن ملفات TR هي ملفات التشغيل التي يستهلكها Mineclonia ضمن سير الصيانة الموثق.

لذلك:
- ملفات PO هي المصدر القابل للمراجعة والتحديث في هذا المستودع.
- لا نعتبر حزمة التعريب جاهزة للتثبيت حتى نتحقق عملياً من طريقة تحميل PO في إصدار Mineclonia المستهدف، أو نولّد ملفات TR بالطريقة الرسمية عند الحاجة.
- لا نوزّع ملفات تشغيل غير مختبرة على أنها حزمة نهائية.

## ملفات المصدر الرئيسية

- https://github.com/mineclonia-mirror/mineclonia/blob/main/DEV_TRANSLATION_WORKFLOW.md
- https://docs.luanti.org/for-creators/translation/mods-and-games/
- https://api.luanti.org/translations/

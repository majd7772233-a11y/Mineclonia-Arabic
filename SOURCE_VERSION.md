# مصدر اللعبة وإصداراتها

## نسخة المستخدم المستهدفة

بحسب ملف `game.conf` الذي قدمه صاحب المشروع:

- `title = Mineclonia`
- `textdomain = mcl_game_meta`
- `min_minetest_version = 5.10`
- `release = 39603`

القيمة `39603` مسجلة هنا كقيمة `release` من ملف المستخدم، وليست رقم إصدار Luanti.

## مصدر القوالب عند بدء التعريب

- المستودع: https://github.com/mineclonia-mirror/mineclonia
- الفرع: `main`
- لقطة المصدر: `24521bf6c99ddfc82bf52a60420a23902282c2a0`
- ملف القالب الأول: `mods/CORE/mcl_init/locale/template.pot`
- Blob SHA للقالب الأول: `2eb9b575a89a28a47c78be0e84f6cc56241afc40`

هذه لقطة مرجعية بدأنا منها، وليست ادعاءً بأننا طابقنا بعد كل ملف في نسخة المستخدم الثنائية مع المصدر حرفياً. عند تجهيز إصدار التثبيت سنقارن قوالب المكونات مع نسخة اللعبة المستهدفة ونوثّق أي اختلاف.

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

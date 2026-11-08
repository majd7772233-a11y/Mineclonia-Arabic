# تعريب Mineclonia

مشروع تعريب لعبة **Mineclonia** إلى العربية، مع الحفاظ على توافق ملفات الترجمة مع بنية اللعبة ومحرك Luanti.

## حالة المشروع

- اللغة المستهدفة: العربية (ar).
- ملفات الترجمة المصدرية: 163 ملف PO، تحتوي على 5,018 ترجمة غير فارغة ومفتاح تنسيقي واحد من مسافة فقط.
- التغطية: جميع وحدات لقطة المصدر المثبتة، مع فحص كتالوجات runtime التي استُخرجت بتشغيل Mineclonia على Luanti server.
- لقطة المصدر: ZenonSeth/mineclonia-mirror عند commit 4ba113d1ce7afbc5b480b5b0e41b6c0037c8d8da (إصدار README للمصدر: 0.123.0).
- الحزمة المولّدة: 162 ملف تشغيل .ar.tr وملف PO واحد لصيغ الجمع الست في mcl_biome_dispatch.
- إصدار اللعبة المحلي الذي أبلغ عنه صاحب المشروع: release=39603؛ لم تتم مطابقة شجرته مع لقطة المصدر بصورة مباشرة.
- نُشر إصدار تجريبي قابل للتثبيت، لكنه لم يُختبر بعد داخل عميل Luanti فعلياً ولم يُدمج في Weblate الرسمي.

### مسار تعريب محرك Luanti — تحديث 2026-11-08

يجري تطوير دعم العربية الأصلي للمحرك في الفرع `work/arabic-engine-support`، ولم يُدمج بعد في `main`، كما أن حزمة Mineclonia التجريبية v7 أعلاه لا تتضمن بناء المحرك الجديد.

- اكتمل إنشاء ملف واجهة Luanti المدمجة `__builtin.ar.tr` بعدد **245 مفتاحاً**، مع نسخة مطابقة في مصدر المحرك والحزمة المجهزة.
- كتالوج gettext العربي للمحرك `luanti/code/po/ar/luanti.po` يحتوي حالياً على **1,575 ترجمة من أصل 1,575 رسالة (100%)**؛ لا توجد رسائل فارغة أو إدخالات `fuzzy` بعد مراجعة جميع الإدخالات السابقة.
- يستخدم مسار تشكيل النصوص `libraqm` مع HarfBuzz وFriBidi وFreeType لتخطيط النص العربي والمختلط، مع الاحتفاظ بالنص المنطقي الأصلي.
- لا يُعد هذا المسار مكتملاً بعد: لا تزال ترجمة إعدادات المحرك ناقصة، كما أن بناء العميل واختبارات C++ وفحص العرض الفعلي للنصوص العربية على Linux وWindows لم تُعتمد بعد. راجع [خطة دعم المحرك](docs/ARABIC_ENGINE_SUPPORT.md) وسجل [PROGRESS.md](PROGRESS.md).

## تنظيم المستودع

تُحفظ ملفات الترجمة تحت مسارات المكونات المطابقة لمسارات Mineclonia، حتى يمكن دمجها أو نسخها دون تخمين:

```
mods/<CATEGORY>/<MOD>/locale/<MOD>.ar.po
```

ملفات `.po` هي ملفات الترجمة المصدرية. ويجب عدم افتراض أنها وحدها تكفي لتوزيعة Mineclonia؛ فدليل سير العمل الخاص باللعبة يصف تحويل ترجمات Weblate من PO إلى TR عند تجهيز ملفات اللعبة. لذلك سنحتفظ بمصدر PO، ونولّد/نختبر ملفات التشغيل وفق سير العمل المتوافق مع إصدار اللعبة المستهدف قبل إصدار حزمة تثبيت.

## قواعد الترجمة

1. لا نغيّر `msgid` أو سياق الرسالة `msgctxt`.
2. نحافظ على العناصر النائبة مثل `@1` و`@2` و`%s` وترتيبها.
3. نحافظ على فواصل الأسطر وعلامات الهروب والمعنى البرمجي.
4. نستخدم مصطلحات عربية موحّدة كما هو موثق في [GLOSSARY.md](GLOSSARY.md).
5. لا نترجم أسماء الأوامر والمعرّفات والرموز البرمجية إلا إذا كانت نصوص عرض صريحة.
6. أي نص غير مفهوم سياقياً يُعلَّم للمراجعة بدلاً من تخمين ترجمته.

## حزمة التثبيت

الحالة: **Pre-release للتجربة**.

- [تنزيل حزمة ملفات التشغيل v7](https://github.com/majd7772233-a11y/Mineclonia-Arabic/releases/download/mineclonia-arabic-source-4ba113d1-language-code-bridge-v7/Mineclonia-Arabic-TR-4ba113d1.zip)
- [صفحة إصدار v7 وملاحظاته](https://github.com/majd7772233-a11y/Mineclonia-Arabic/releases/tag/mineclonia-arabic-source-4ba113d1-language-code-bridge-v7)
- [ملف SHA-256](https://github.com/majd7772233-a11y/Mineclonia-Arabic/releases/download/mineclonia-arabic-source-4ba113d1-language-code-bridge-v7/Mineclonia-Arabic-TR-4ba113d1.zip.sha256)

الحزمة تحتوي 162 ملف تشغيل بأسماء textdomain.ar.tr الصحيحة وملف PO واحداً لصيغ الجمع، إضافة إلى مشغّل Windows وجسر اللغة الخاص بمحرك Luanti. استخرج محتويات ZIP كاملة إلى مجلد Mineclonia الذي يحتوي على game.conf، واسمح بدمج مجلد mods دون استبداله بالكامل. بعد ذلك شغّل ENABLE_ARABIC_LANGUAGE.bat الموجود بجانب mods، واختر جذر تثبيت Luanti الذي يحتوي على bin وbuiltin (وليس مجلد Mineclonia). يضيف المشغّل أيضاً `locale/ar/LC_MESSAGES/luanti.mo` إلى جذر Luanti؛ هذا الملف يعرّف `LANG_CODE = ar` حتى يعلن العميل اللغة العربية للخادم ويستلم ملفات Mineclonia العربية. كما يضبط `language = ar` في ملف إعداد Luanti الفعلي مع نسخ احتياطية.

الحزمة مبنية من لقطة مصدر Mineclonia 0.123.0 المثبتة في SOURCE_VERSION.md. الإصدار v7 يعالج سبب عدم وصول ملفات الترجمة: Luanti 5.17.0 يرسل رمز لغة العميل عبر ترجمة gettext للمفتاح `LANG_CODE`؛ وإذا لم يوجد ملف `luanti.mo` للعربية، يرسل رمزاً فارغاً ولا يرسل الخادم ملفات `.ar.tr`. لذلك يثبّت المشغّل كتالوج MO صغيراً يربط `LANG_CODE` بـ `ar`. في وضع Windows المحمول `RUN_IN_PLACE` يكتب إعدادات اللغة إلى `minetest.conf` بجذر التثبيت بجانب `bin`؛ أما التثبيت غير المحمول فيحترم `LUANTI_USER_PATH` أو `MINETEST_USER_PATH` ثم يستخدم `%APPDATA%\\Minetest`. اختبارات Windows PowerShell 5.1 تغطي مسار الإعداد وجسر MO ونسخ الملف الاحتياطي والحفاظ على بقية الإعدادات. هذا الجسر يفعّل ترجمة Mineclonia لكنه لا يترجم واجهة قوائم Luanti نفسها، ولا يضيف تشكيل الحروف أو تخطيط RTL الكامل. لم تتم مطابقة شجرة اللعبة المحلية release=39603 مع لقطة المصدر مباشرة ولم يُختبر العرض داخل عميل Luanti فعلياً؛ لذلك تظل الحزمة Pre-release للتجربة، وليست اعتماداً upstream أو دمجاً في Weblate. راجع [SOURCE_VERSION.md](SOURCE_VERSION.md) و[PROGRESS.md](PROGRESS.md).

## المراجع

- [مستودع Mineclonia](https://codeberg.org/mineclonia/mineclonia)
- [دليل الترجمة الخاص بـ Mineclonia](https://github.com/mineclonia-mirror/mineclonia/blob/main/DEV_TRANSLATION_WORKFLOW.md)
- [توثيق Luanti لترجمة الألعاب والإضافات](https://docs.luanti.org/for-creators/translation/mods-and-games/)
- [توثيق Luanti لملفات الترجمة](https://api.luanti.org/translations/)


### ملاحظة عن قائمة لغة Luanti

في Luanti 5.17.0 كانت `ar` محجوبة من قائمة اللغات في `builtin/settingtypes.txt` و`builtin/common/settings/dlg_settings.lua`، كما كانت ضمن `src/unsupported_language_list.txt` بسبب محدودية دعم اتجاه الكتابة RTL. لذلك لا يكفي وضع ملفات Mineclonia `.tr` داخل اللعبة. يتضمن الإصدار الجديد مشغّل `ENABLE_ARABIC_LANGUAGE.bat` لإظهار الخيار في الواجهة مع نسخ احتياطية. هذا لا يضيف دعم RTL كاملاً لمحرك Luanti؛ وقد تبقى بعض قوائم المحرك باللغة الإنجليزية أو يظهر ترتيب العربية غير مثالي.

مرجع قوالب الإنجليزية الاحتياطي: [mineclonia-en/mcl_core/mods](https://github.com/majd7772233-a11y/gpts/tree/main/mineclonia-en/mcl_core/mods).

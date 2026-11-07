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

- [تنزيل حزمة ملفات التشغيل](https://github.com/majd7772233-a11y/Mineclonia-Arabic/releases/download/mineclonia-arabic-source-4ba113d1/Mineclonia-Arabic-TR-4ba113d1.zip)
- [صفحة الإصدار وملاحظاته](https://github.com/majd7772233-a11y/Mineclonia-Arabic/releases/tag/mineclonia-arabic-source-4ba113d1)
- [ملف SHA-256](https://github.com/majd7772233-a11y/Mineclonia-Arabic/releases/download/mineclonia-arabic-source-4ba113d1/Mineclonia-Arabic-TR-4ba113d1.zip.sha256)

الحزمة تحتوي 162 ملف تشغيل .ar.tr وملف PO واحداً لصيغ الجمع، مع دليل تثبيت وManifest. أغلق Luanti، ثم فك الأرشيف في مجلد Mineclonia الذي يحتوي على game.conf وادمج مجلد mods؛ لا تستبدل مجلد mods بالكامل ولا تستبدل ملفات Lua.

الحزمة بُنيت من لقطة مصدر Mineclonia 0.123.0 المثبتة في SOURCE_VERSION.md. رقم release=39603 الذي أبلغت عنه لم يُطابق بعد مع شجرة المصدر، كما أن عرض العربية لم يُختبر داخل عميل Luanti فعلي. لذلك هذا Pre-release مرشح للتثبيت والاختبار، وليس اعتماداً upstream أو دمجاً في Weblate. راجع [SOURCE_VERSION.md](SOURCE_VERSION.md) و[PROGRESS.md](PROGRESS.md).

## المراجع

- [مستودع Mineclonia](https://codeberg.org/mineclonia/mineclonia)
- [دليل الترجمة الخاص بـ Mineclonia](https://github.com/mineclonia-mirror/mineclonia/blob/main/DEV_TRANSLATION_WORKFLOW.md)
- [توثيق Luanti لترجمة الألعاب والإضافات](https://docs.luanti.org/for-creators/translation/mods-and-games/)
- [توثيق Luanti لملفات الترجمة](https://api.luanti.org/translations/)

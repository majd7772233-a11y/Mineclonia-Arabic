# تعريب Mineclonia

مشروع تعريب لعبة **Mineclonia** إلى العربية، مع الحفاظ على توافق ملفات الترجمة مع بنية اللعبة ومحرك Luanti.

## حالة المشروع

- اللغة المستهدفة: العربية (`ar`).
- اللعبة المرجعية التي أبلغ عنها صاحب المشروع: `Mineclonia release=39603`.
- مصدر القوالب الأولي: [mineclonia-mirror/mineclonia](https://github.com/mineclonia-mirror/mineclonia).
- لقطة المصدر المستخدمة عند بدء المشروع: `24521bf6c99ddfc82bf52a60420a23902282c2a0`.
- بدأ العمل بملف `mcl_init`، وتُضاف بقية المكونات تدريجياً.\n- سجل القوالب وGit blob SHA لكل مصدر: [SOURCE_TEMPLATES.md](SOURCE_TEMPLATES.md).
- لم يُعلن عن اكتمال التعريب أو اختباره داخل اللعبة بعد.

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

<badge>Pre-release للتجربة</badge>

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

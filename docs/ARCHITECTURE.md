# البنية التقنية

## 1. مسار ترجمة Mineclonia
1. ملفات PO العربية في mods/**/locale هي مصادر الترجمة.
2. template.pot يحدد مفاتيح المصدر؛ بصمات القوالب في SOURCE_TEMPLATES.md.
3. GLOSSARY.md مرجع المصطلحات.
4. tools/build_tr_package.py وtools/check_po.py وtools/ltt_tr_compat.py أدوات فحص وتحويل.
5. ملفات التشغيل تكون عادة .ar.tr؛ مكون mcl_biome_dispatch له احتياج PO لصيغ الجمع موثق في SOURCE_VERSION.md.

لا تغيّر msgid أو placeholders أو بنية الرسائل متعددة الأسطر.

## 2. تخطيط النص العربي

تدفق العمل:

UTF-8 / wide text
-> UTF-32 logical code points
-> Raqm layout
-> FriBidi paragraph direction / BiDi runs
-> HarfBuzz OpenType shaping
-> FreeType font faces and glyph metrics
-> visual glyph order + logical clusters
-> rendering / dimensions / cursor stops / selection geometry

لا تعكس النص المنطقي يدوياً. يتم تشكيل النص في Raqm مع الحفاظ على فهارس المصدر المنطقية. الترتيب المرئي يختلف عن الترتيب المنطقي خصوصاً في النص المختلط.

### الخطوط والفهارس
- الخط المرفق NotoSansArabic-Regular.ttf موجود في luanti/code/fonts وluanti/5.17.0/fonts.
- CGUITTFont يحمل الخط عبر FreeType وينشئ Raqm layouts.
- يمكن لعنقود glyph أن يأتي من face مختلف عبر fallback.
- Linux غالباً wchar_t 32-bit؛ Windows wchar_t 16-bit ويحتاج تحويل UTF-16/surrogate pairs إلى UTF-32.
- لا يجوز للمؤشر أو الحذف تقسيم surrogate pair أو grapheme أو shaping cluster.
- grapheme boundary وshaping-cluster boundary مفهومان مختلفان.

### font_text_layout.h
يساعد في استخراج shaping cluster boundaries، التحقق من cursor stops، توسيع التحديد، حساب visual selection intervals متعددة، تصنيف Unicode whitespace/punctuation دون locale، وحدود الكلمات والحركة المرئية للمؤشر.

### CGUITTFont.cpp/.h
مسؤول عن تحويل الفهارس wide/UTF-32، إنشاء Raqm layout، قياس النص، تحديد موضع المؤشر، التنقل المرئي، visual word navigation، selection geometry ورسم glyphs مع offsets/advances/clusters وfallback fonts.

### StaticText
في src/irrlicht_changes/static_text.cpp يتم كشف اتجاه الفقرة عبر Raqm فقط لخط EGFT_CUSTOM. لا تحوّل bitmap font إلى CGUITTFont. النص يظل بترتيبه المنطقي وBiDi يعاد ترتيبه أثناء shaping/drawing.

### CGUIEditBox
في irr/src/CGUIEditBox.cpp يوجد تنقل بصري بالكلمة، حذف بالكلمة، hit-testing إلى أقرب caret stop مسموح، selection intervals، وتكامل مع copy/cut/paste. يجب الحفاظ على المؤشر والتحديد عند رفض الإدخال.

### editbox_text_edit.h
يفصل بناء النص المرشح عن تعديل widget: يقص cursor/mark indexes، يحدد نطاق الاستبدال، يعالج overwrite دون ابتلاع CR/LF، ويفحص Max قبل قبول التغيير.

## 3. builtin SHA-256
CMake يولد manifest keys بشرطات slash. Windows filesystem قد يعيد backslashes. normalizeBuiltinFileSha256Path() في src/builtin_files.h يوحدها قبل lookup، ويستخدم في script/cpp_api/s_security.cpp. testBuiltinFileSha256Manifest يختبر entries والمسارات. لا تخف التحذير؛ اختبر exe وbuiltin المتطابقين.

## 4. قائمة اللغة وLua
شجرة بناء المصدر luanti/code/builtin، والشجرة المرجعية luanti/5.17.0/builtin. خطأ القائمة الأصلي كان فاصلة مفقودة بعد العربية قبل be؛ الفاصلة موجودة حالياً في النسختين. tools/validate_lua_syntax.py يقرأ كل Lua من الشجرتين لكنه يحتوي regex مخصصاً مع double escaping يحتاج إصلاحاً.

## 5. CMake/vcpkg
vcpkg.json يثبت الاعتماديات. Linux triplet x64-linux static، ويستخدم raqm.pc مع pkg-config --static حتى لا تسقط تبعيات HarfBuzz/FriBidi/FreeType. Windows يستخدم x64-windows. تفضيل imported targets يحافظ على transitive link dependencies. ENABLE_SOUND=ON صريح. ZLIB يُكتشف بـ find_package(ZLIB MODULE REQUIRED) لإنشاء ZLIB::ZLIB المتوافق مع static vcpkg.

## 6. ما يحتاج اختباراً يدوياً
فتح قوائم العميل وإعدادات اللغة، عدم ظهور Lua parser error، اختفاء SHA warnings مع exe/builtin من حزمة واحدة، clipboard round-trip، mouse drag وShift selection، clipping/shadows/centering/alignment، scroll، fallback/emoji/supplementary Unicode.

## 7. Windows portable path model

For Windows RUN_IN_PLACE ZIP builds, path_share remains the application/share root containing builtin, fonts, textures, locale and binaries. path_user is now derived as path_share/Minetest. The engine's existing path_user consumers therefore place worlds, user-installed games, config, logs and other writable profile data below that directory; path_cache is path_user/cache.

The first-launch migration copies the legacy %APPDATA%/Minetest tree into a staging directory beside the target and activates it only after the copy succeeds. The old profile remains untouched. Existing portable data is not merged with AppData automatically. Windows RUN_IN_PLACE config discovery is restricted to path_user/minetest.conf; parent-level legacy config discovery remains only on non-Windows RUN_IN_PLACE builds.

This change is intentionally Windows-specific. Other platforms keep their prior RUN_IN_PLACE path behavior. The Windows workflow's package probe is intended to prove migration and prevent accidental writes back to AppData.

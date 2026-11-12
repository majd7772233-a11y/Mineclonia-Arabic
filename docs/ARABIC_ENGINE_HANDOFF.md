# تسليم دعم العربية في محرك Luanti — نقطة الاستئناف الرسمية

التاريخ: 2026-11-12
Repository: majd7772233-a11y/Mineclonia-Arabic
Branch: work/arabic-engine-support فقط
رأس الكود قبل دفعة التوثيق: 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2
main الذي تم فحصه: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (لم يُعدّل).

## اقرأ قبل العمل
AGENTS.md، docs/PROJECT_STATE.md، docs/NEXT_SESSION_PROMPT.md، docs/ROADMAP.md، docs/BUILD_TESTING.md، docs/ARCHITECTURE.md، docs/FILE_MAP.md، docs/DOCUMENTATION_RULES.md، docs/ARABIC_ENGINE_WORKLOG.md، docs/ARABIC_ENGINE_SUPPORT.md، README.md، PROGRESS.md، SOURCE_VERSION.md، SOURCE_TEMPLATES.md، GLOSSARY.md.

## الملخص
- تعريب Mineclonia: 163 PO files و5,018 نصاً مترجماً + مفتاح تنسيقي.
- Native Luanti Arabic: Raqm + HarfBuzz + FriBidi + FreeType + Noto Sans Arabic.
- النص المنطقي محفوظ؛ shaping/BiDi ينتج ترتيب glyph مرئياً.
- editbox: cursor clusters، visual word navigation/deletion، selection intervals، hit testing، max/overwrite/paste protections.
- builtin SHA path normalization موجود.
- خطأ الفاصلة في Lua مصحح بالمصدر في الشجرتين، لكن regex validator يحتاج إصلاحاً.

## نقطة التوقف
Run106 ID 42798177083 فشل في Build client على Linux/Windows بسبب duplicate definitions:
- testLamAlefPreservesSeparateClusters
- testArabicAllahLigatureCluster

Commit 6e8511e حذف التكرارات وأضاف validate_arabic_test_source.py. الفحص النصي الحالي وجد 17/17/17 دون duplicate، لكن لم يثبت build بعد.

Run107 ID 42805300100 failure، jobs=0 وartifacts=0 في API، ولا توجد job logs.

## العوائق الفورية
1. .github/workflows/build-luanti-arabic-engine.yml:
   - line 187 Linux grep غير مغلق.
   - line 332 PowerShell Select-String مبتور.
   - line 539 orphan -Quiet)).
   - مقاطع package مكررة وتعريف windows job ثاني عند line 591.
2. tools/validate_lua_syntax.py: regex يستخدم double escaping للأقواس المربعة؛ أصلحه واختبره على الشجرتين.
3. لا يوجد CI حديث يثبت إزالة C++ duplicates.
4. لا يوجد Artifact حديث من HEAD الحالي ولا اختبار GUI جديد من المستخدم.

## قواعد لا تنسها
- لا تلمس main ولا تنشئ branches.
- حافظ على vcpkg cache restore/save.
- CI مكلف؛ اجمع الإصلاحات ثم شغل مرة واحدة.
- لا تخف SHA warnings؛ تحقق من package matching exe/builtin.
- اشترط 17/17 على النظامين.
- تحديث Markdown لكل تعديل وفي commit نفسه إلزامي.

ابدأ بقراءة PROJECT_STATE.md وNEXT_SESSION_PROMPT.md. التفاصيل الكاملة والأوامر في BUILD_TESTING.md.

# رسالة جاهزة للمحادثة الجديدة

انسخ النص التالي كاملاً إلى محادثة جديدة. المطلوب من المساعد أن يبدأ بقراءة GitHub والوثائق ثم يكمل العمل دون الاعتماد على ذاكرة المحادثة السابقة.

---

أريدك أن تواصل مشروع Mineclonia-Arabic من آخر حالة موجودة على GitHub. هذه جلسة استئناف رسمية؛ لا تعتمد على محادثة سابقة.

## المستودع والفرع
- Repository: majd7772233-a11y/Mineclonia-Arabic
- Branch الوحيد المسموح: work/arabic-engine-support
- ممنوع تعديل main أو الدمج إليه أو إعادة ضبطه أو الدفع إليه.
- ممنوع إنشاء فروع إضافية.
- رأس الكود قبل دفعة التوثيق: 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2
- main الذي تم فحصه: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d
- اقرأ HEAD الحالي من GitHub؛ يجب أن يكون هناك commit توثيق فوق رأس الكود. لا تعمل على الرأس القديم إذا كان هناك HEAD أحدث.

## أول خطوة إلزامية: قراءة الوثائق
اقرأ هذه الملفات كاملة قبل لمس الكود:
1. AGENTS.md
2. docs/PROJECT_STATE.md
3. docs/NEXT_SESSION_PROMPT.md
4. docs/ARABIC_ENGINE_HANDOFF.md
5. docs/ROADMAP.md
6. docs/BUILD_TESTING.md
7. docs/ARCHITECTURE.md
8. docs/FILE_MAP.md
9. docs/DOCUMENTATION_RULES.md
10. docs/ARABIC_ENGINE_WORKLOG.md
11. docs/ARABIC_ENGINE_SUPPORT.md
12. README.md وPROGRESS.md وSOURCE_VERSION.md وSOURCE_TEMPLATES.md وGLOSSARY.md وluanti/5.17.0/ARABIC_TEXT_FIX.md.

بعدها اعرض قائمة بكل ملفات Markdown المتتبعة، واقرأ التوثيق الآخر المرتبط بالكود. ثم افحص branch HEAD وmain والـcommit الأخير والفرق. لا تبدأ أي تغيير قبل القراءة.

## قاعدة التوثيق الصارمة
بعد كل تعديل صغير أو كبير، حدث Markdown المناسب فوراً وفي commit نفسه. الحد الأدنى: PROJECT_STATE وWORKLOG وHANDOFF وSUPPORT وPROGRESS، وREADME عند تغير الحالة. حدّث BUILD_TESTING/ARCHITECTURE/FILE_MAP/ROADMAP حسب التغيير. لا تؤجل التوثيق إلى commit لاحق.

## حالة المشروع
المشروع مساران:
1. تعريب Mineclonia: 163 PO files، 5,018 ترجمة غير فارغة ومفتاح تنسيقي واحد؛ المصدر والقوالب في SOURCE_VERSION وSOURCE_TEMPLATES؛ حزمة v7 هي Pre-release تاريخية وليست دمج Weblate.
2. دعم العربية الأصلي في Luanti 5.17.0: Raqm + HarfBuzz + FriBidi + FreeType + Noto Sans Arabic، BiDi وRTL ومؤشرات وتحديد وتحرير cluster-safe وSHA builtin normalization.

آخر commit كود:
6e8511ef4f7b437e0ce7c6769a3405ba68c077b2 — Remove duplicate Arabic tests and verify packaged builtin integrity.

## آخر نتائج CI
Run #105 ID 42782985200:
- Windows build و16/16 tests والحزمة نجحوا.
- Linux build مرّ لكن Arabic test module فشل 4/16.
- Artifact Windows ID 13747836271 قديم ولا يمثل HEAD الحالي.

Run #106 ID 42798177083، SHA 3a6fe0e7e179e01c3562869cb590c6cdb6d14131:
- catalog validation passed.
- Linux وWindows Build client فشلا بسبب duplicate definitions في test_arabic_text_layout.cpp:
  testLamAlefPreservesSeparateClusters
  testArabicAllahLigatureCluster
- لم تصل الاختبارات أو الحزم إلى التنفيذ.

Run #107 ID 42805300100، SHA 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2:
- conclusion=failure.
- GitHub API jobs endpoint أعاد 0 jobs وartifacts endpoint أعاد 0.
- لا توجد job logs متاحة.
- workflow الحالي في المصدر فيه مقاطع مكسورة ومكررة.

## أول عائق: إصلاح workflow
الملف .github/workflows/build-luanti-arabic-engine.yml، 748 سطراً:
- line 187 Linux grep pattern غير مغلق.
- line 332 PowerShell Select-String شرط مبتور.
- line 539 orphan fragment يبدأ -Quiet)).
- lines 539–589 ذيول package مكررة ومختلطة.
- line 591 تعريف windows job ثانٍ مكرر.

أعد بناءه بعناية مع jobs واحدة:
workflow-sanity، catalog validation، Linux، Windows.
أصلح كل Bash/PowerShell quote/if blocks. حافظ على vcpkg cache restore/save على المنصتين. استخدم YAML parser/actionlint متاحاً قبل push. لا تطلق CI على workflow غير صالح.

## ثاني عائق: Lua validator regex
tools/validate_lua_syntax.py يحتوي regex يهرب الأقواس مرتين. النمط لا يطابق النص:
    ar = "العربية [ar]",
    be = "Беларуская [be]",

صححه إلى escaping واحد للأقواس ثم اختبره على الملفين:
- luanti/code/builtin/common/settings/dlg_settings.lua
- luanti/5.17.0/builtin/common/settings/dlg_settings.lua

خطأ Lua الأصلي كان فاصلة مفقودة بعد خيار العربية؛ الفاصلة موجودة الآن في الشجرتين منذ commit 9bdd876eea9ab8087906b4d067d76fd587e84fc8. لا تخلط بين إصلاح المصدر وخطأ regex في الفاحص.

## ثالث عائق: إثبات إزالة duplicate C++ tests
commit 6e8511e حذف النسخ المكررة وأضاف tools/validate_arabic_test_source.py. الفحص النصي الحالي وجد:
17 declarations، 17 registrations، 17 definitions، دون duplicates.
هذا ليس compilation proof. يجب بناء Linux وWindows بعد إصلاح CI.

## SHA warnings
المستخدم أبلغ عن No SHA256 known for builtin file. السبب اختلاف path separators بين manifest slash ومسارات Windows backslash. تمت إضافة normalizeBuiltinFileSha256Path() في src/builtin_files.h واستخدامها في s_security.cpp، وunit test لها.
لا تخف التحذير. بعد Artifact حديث يجب تجربة exe وbuiltin من الحزمة نفسها. لا تدّع أن التحذير اختفى على جهاز المستخدم حتى يختبر ذلك.

## اختبار العربية
المطلوب 17 tests، والsummary:
Module TestArabicTextLayout passed (0 failures / 17 tests)

الخط المرفق:
- بب ينتج أشكال base سياقية وعلامتي dot-below، أربعة glyphs صحيحة.
- لا لا تملك ligature في الخط؛ glyphs منفصلان.
- اللّٰه تملك ligature uniFDF2 واحدة.
لا تضعف assertions لإرضاء CI.

## ترتيب العمل
1. اقرأ الوثائق وكل Markdown المتعلق بالمشروع.
2. افحص GitHub HEAD وmain.
3. أصلح workflow وLua regex وأي خطأ ثابت آخر في دفعة واحدة.
4. شغل catalog, PO msgfmt, Lua parser, Arabic test source validator, YAML validation.
5. راجع diff كاملاً.
6. حدث جميع docs في commit نفسه.
7. شغل CI واحداً بعد preflight مع الحفاظ على الكاش.
8. عالج الأخطاء من السجلات.
9. اشترط 17/17 على Linux وWindows، ثم تحقق من الحزمتين.
10. اطلب من المستخدم تجربة Artifact Windows الحديث، ثم اختبر clipboard/drag/Shift/visual layout.

## ممنوعات
لا تلمس main، لا تنشئ branch، لا تدفع على كل خط صغير، لا تحذف assertions، لا تدّع نجاحاً بلا evidence، ولا تترك تعديلاً دون تحديث Markdown في commit نفسه. ابدأ الآن بالقراءة والتحقق ثم أصلح workflow أولاً.

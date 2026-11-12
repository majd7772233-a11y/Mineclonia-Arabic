# الحالة الحالية للمشروع — المرجع التنفيذي

آخر تدقيق: 2026-11-12
المستودع: majd7772233-a11y/Mineclonia-Arabic
فرع العمل الوحيد: work/arabic-engine-support
رأس الكود قبل دفعة التوثيق: 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2
main الذي تم فحصه: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d
حالة main: لم يُعدّل. دفعة التوثيق الحالية تنشئ commit فوق الرأس أعلاه؛ يجب قراءة HEAD الجديد من GitHub قبل أي عمل.

## 1. الخلاصة

المشروع يحتوي تعريب Mineclonia ودعم العربية داخل Luanti 5.17.0. تقدم مسار المحرك حقيقي في المصدر، لكنه غير مكتمل ولا يوجد بناء حديث ناجح مثبت على الرأس الحالي.

آخر commit برمجي:
6e8511ef4f7b437e0ce7c6769a3405ba68c077b2 — Remove duplicate Arabic tests and verify packaged builtin integrity

حذف تعريفين مكررين كانا يوقفان ترجمة اختبار C++، وأضاف source validator، ووسّع فحوص builtin المعبأ. لا يوجد CI يثبت نجاحه بعد.

## 2. ما هو مثبت في المصدر

- خيار العربية في luanti/code/builtin/common/settings/dlg_settings.lua يحتوي فاصلة قبل be.
- النسخة المرجعية luanti/5.17.0/builtin/common/settings/dlg_settings.lua تحتوي الفاصلة.
- SHA path normalization موجود في src/builtin_files.h ويستخدم في s_security.cpp.
- TestArabicTextLayout الحالي لديه 17 declarations و17 TEST registrations و17 definitions دون duplicates حسب فحص نصي مباشر.
- validate_arabic_test_source.py موجود.
- validate_lua_syntax.py يمر على builtin trees الاثنين، لكن regex مخصصاً فيه معطوباً بسبب double escaping.
- vcpkg cache restore/save موجود في Linux وWindows workflow.
- CMake يستخدم Vorbis/OpenAL imports، Raqm static pkg-config closure على Linux، وZlib module صراحة.

## 3. أدلة CI

| Run | SHA | النتيجة |
|---|---|---|
| #102 ID 42774549330 | 9bdd876eea9ab8087906b4d067d76fd587e84fc8 | Windows build و16/16 tests/package passed؛ Linux build passed لكن 4/16 shaping tests failed |
| #105 ID 42782985200 | 9eda41ba6285afb6ef0891a4493ac9cf35b32d99 | Windows build و16/16 tests/package passed؛ Linux 4/16 failures؛ Windows Artifact ID 13747836271 |
| #106 ID 42798177083 | 3a6fe0e7e179e01c3562869cb590c6cdb6d14131 | Catalog validation passed؛ Linux وWindows Build client failed بسبب duplicate C++ test definitions |
| #107 ID 42805300100 | 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2 | conclusion=failure؛ jobs endpoint returned 0 jobs and artifacts endpoint 0 artifacts؛ no job logs |

Artifact Run105 قديم ويحتوي 16 tests؛ ليس دليلاً على HEAD الحالي أو 17 tests.

## 4. نقطة التوقف الدقيقة

### P0: workflow مكسور

الملف .github/workflows/build-luanti-arabic-engine.yml، 748 سطراً:
- line 187: Linux grep pattern غير مغلق الاقتباس قبل الأمر التالي.
- line 332: PowerShell Select-String شرط مبتور، والشرط التالي يبدأ line 333.
- line 539: مقطع يتيم -Quiet)).
- lines 539–589: ذيول package مكررة ومختلطة بين Windows وLinux.
- line 591: تعريف windows job ثانٍ مكرر حتى نهاية الملف.

Run107 failure مع 0 jobs يتوافق مع فشل workflow parsing/structure، لكن لا توجد logs لتأكيد استجابة GitHub الداخلية حرفياً. أصلح الملف وتحقق من YAML قبل أي CI.

### P0: regex فاحص Lua مفرط الهروب

في tools/validate_lua_syntax.py النمط الحالي يهرب الأقواس مرتين:
r'(?m)^\s*ar\s*=\s*"العربية \\[ar\\]",\s*\r?\n\s*be\s*='

هذا لا يطابق:
    ar = "العربية [ar]",
    be = "Беларуская [be]",

استخدم escaping واحداً للأقواس المربعة واختبر النمط على النص الفعلي في الشجرتين. هذا خطأ في validator وليس دليلاً على أن مصدر Lua خالٍ من الفاصلة.

### P1: إثبات إزالة تعريفات C++ المكررة

Run106:
- Linux job ID 129769141742.
- Windows job ID 129769141837.
- الملف luanti/code/src/unittest/test_arabic_text_layout.cpp.
- GCC/MSVC أبلغا عن duplicate definitions لـtestLamAlefPreservesSeparateClusters وtestArabicAllahLigatureCluster.
- commit 6e8511e حذف النسخ المكررة وأدخل validate_arabic_test_source.py.
- فحص المصدر الحالي: 17 declarations، 17 registrations، 17 definitions، بلا duplicates أو اختلافات.
- هذا فحص نصي، وليس compilation proof.

## 5. خطأ Lua الذي أبلغ عنه المستخدم

الرسالة الأصلية كانت:
builtin/common/settings/dlg_settings.lua:179:
'}' expected (to close '{' at line 176) near 'be'

السبب فاصلة مفقودة بعد خيار العربية. أصلحت في 9bdd876eea9ab8087906b4d067d76fd587e84fc8. الفاصلة موجودة في الشجرتين الحاليتين. سجل المستخدم كان 2026-11-09 23:41، أي قبل Artifact Windows في Run105. يجب إصدار حزمة حديثة واختبارها فعلياً قبل الجزم بأن الخطأ اختفى من جهازه.

## 6. تحذيرات SHA-256

التحذير No SHA256 known for builtin file منفصل عن خطأ Lua:
- Hash manifest keys تستخدم slash.
- Windows filesystem يعيد backslash.
- normalizeBuiltinFileSha256Path() يحول backslash إلى slash قبل lookup.
- Run105 Windows unit test مرّ ضمن 16 tests.
- يلزم Artifact حديث مطابق ثم تجربة المستخدم. لا تخف التحذير ولا تفترض أن exe القديم يطابق builtin الحالي.

## 7. اختبارات العربية الحالية

العدد المطلوب 17:
1 testArabicContextualShaping
2 testHarfBuzzOpenTypeArabicShaping
3 testLamAlefPreservesSeparateClusters
4 testArabicAllahLigatureCluster
5 testArabicExtendedLetters
6 testMixedDirectionParagraph
7 testBidiIsolatesAndArabicIndicDigits
8 testMixedDirectionSelectionIntervals
9 testVisualWordNavigationMixedDirection
10 testVisualWordDeletionRangesMixedDirection
11 testMouseHitTestingRespectsShapingClusters
12 testUnicodeWordSelectionRanges
13 testCombiningMarkCluster
14 testExplicitParagraphDirection
15 testFallbackFontRange
16 testEditBoxTextReplacementLimits
17 testBuiltinFileSha256Manifest

Expected summary:
Module TestArabicTextLayout passed (0 failures / 17 tests)

معلومات الخط:
- بب ينتج شكلين base سياقيين + علامتي dot-below، لذلك 4 glyphs صحيحة.
- لا لا تملك ligature في الخط المرفق وتنتج glyphs منفصلين.
- اللّٰه تتحول إلى uniFDF2 ligature واحدة.
لا تضعف assertions.

## 8. إعداد البناء

- Luanti 5.17.0 source.
- vcpkg baseline ddf0518c31a259dcc359fd4c7bcb1ce8a27e9855.
- Linux triplet x64-linux static، Ninja.
- Windows triplet x64-windows، Visual Studio 2022، MSVC.
- CMake 4.4.4، C++17، ENABLE_SOUND=ON.
- vcpkg binary cache restore/save على النظامين.
- workflow uses cancel-in-progress: true؛ push جديد قد يلغي run سابقاً.

## 9. المطلوب تالياً

1. اقرأ كل الوثائق وافحص branch HEAD الحالي وmain.
2. أصلح workflow structure والshell/PowerShell checks.
3. أصلح Lua regex واختبره على الشجرتين.
4. شغل catalog/PO/Lua/test-source/YAML validators.
5. راجع diff كاملاً، حافظ على cache.
6. وثّق كل تعديل في commit نفسه.
7. شغّل CI واحداً على Linux وWindows.
8. أثبت build و17/17 tests والحزم لكل منصة.
9. أعط المستخدم Artifact حديثاً لاختبار القائمة وSHA warnings.
10. أكمل clipboard وmouse/Shift selection وvisual alignment/clipping.

## 10. غير منجز

- لا يوجد build ناجح للرأس الحالي.
- لا توجد حزم من الرأس الحالي.
- لم يتأكد المستخدم من GUI بعد حزمة حديثة.
- لم يتأكد اختفاء SHA warnings على جهازه.
- clipboard/drag/Shift/clipping/alignment لم تُختبر يدوياً.
- لا دمج إلى main.

# سياسة التوثيق الإلزامي

## القاعدة العليا
**لا يُسمح بتغيير الكود أو CMake أو scripts أو workflows أو assets دون تحديث وثائق Markdown المناسبة في commit نفسه.** حتى تعديل سطر واحد يستلزم تحديثاً مناسباً.

## الحد الأدنى لكل commit برمجي
- docs/PROJECT_STATE.md: الحالة الفعلية والفحوص.
- docs/ARABIC_ENGINE_WORKLOG.md: مدخل مؤرخ append-only.
- docs/ARABIC_ENGINE_HANDOFF.md: نقطة الاستئناف الدقيقة.
- docs/ARABIC_ENGINE_SUPPORT.md: التقنية/الحلول/القيود.
- PROGRESS.md: حالة المحرك مع الحفاظ على سجل الترجمة.
- README.md: ملخص الحالة الظاهرة للمستخدم عند الحاجة.

حدّث أيضاً ARCHITECTURE.md أو FILE_MAP.md أو BUILD_TESTING.md أو ROADMAP.md أو NEXT_SESSION_PROMPT.md حسب نوع التغيير.

## ما يجب أن يحتويه سجل العمل
التاريخ، starting commit، سبب التغيير، الملفات، الفحوص، CI run ID ونتائج كل job، عدد الاختبارات، Artifact ID وSHA/digest، وما لم يُختبر.

السجل append-only؛ لا تمسح تاريخ الإخفاقات. صحح التفسير بإضافة إدخال جديد يشير إلى السابق.

## درجات الأدلة
- موجود في المصدر: تمت قراءته من ملفات الفرع.
- اجتاز فحصاً ساكناً: نص/regex/validator فقط.
- اجتاز configure: CMake generation نجح.
- اجتاز build: targets اكتملت.
- اجتاز test: السجل يثبت العدد والنتيجة.
- اجتاز packaging: install/files/archive/upload ثبتت.
- اختُبر فعلياً: فتح المستخدم للتطبيق أو الوظيفة وقدم log/نتيجة.

لا ترفع درجة الدليل دون سجل.

## ترتيب العمل الذري
1. اقرأ الملفات.
2. راجع السياق.
3. نفذ مجموعة متماسكة.
4. شغّل static checks.
5. حدّث Markdown فوراً.
6. راجع diff كاملاً.
7. commit ذري واحد.
8. افحص branch ref.
9. CI حسب سياسة الكاش.
10. وثّق النتائج التالية فور توفرها.

ممنوع أن تكون المحادثة هي المكان الوحيد لنقطة التوقف. ممنوع تعديل main أو إنشاء فروع جديدة.

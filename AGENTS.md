# قواعد العمل الإلزامية للمشروع

هذا الملف عقد العمل لكل مساعد أو مساهم يعمل على المستودع. يجب قراءته قبل تعديل أي ملف.

## هوية المشروع
المستودع majd7772233-a11y/Mineclonia-Arabic يجمع مسارين مستقلين:
1. تعريب Mineclonia وملفات PO/TR وأدوات تجهيز الحزم.
2. تطوير دعم العربية الأصلي داخل عميل Luanti 5.17.0، بما يشمل Raqm وHarfBuzz وFriBidi وFreeType واتجاه النص والتحرير.

لا تخلط نجاح الترجمة مع اكتمال دعم المحرك؛ لكل مسار اختباراته وأدلته.

## الفروع والحماية
- الفرع الوحيد المسموح بالعمل عليه هو work/arabic-engine-support.
- يمنع تعديل main أو إعادة ضبطه أو الدمج إليه أو الدفع إليه.
- لا تنشئ فروعاً إضافية.
- قبل أي كتابة، اقرأ رأس فرع العمل ورأس main من GitHub.
- استخدم تحديثاً Fast-forward مع expected_sha المطابق للرأس الذي فُحص. إذا تغير الرأس، أعد الفحص والمراجعة.
- لا تستخدم Force Push ولا تتجاوز تعارضاً.

## القاعدة الصارمة: التوثيق مع كل تعديل
**أي تعديل، مهما كان صغيراً أو كبيراً، يجب أن يصاحبه تحديث الوثائق المناسبة في commit نفسه.**

قبل إنشاء commit برمجي أو تغيير workflow:
1. حدّث docs/PROJECT_STATE.md بالحالة الفعلية والفحوص.
2. أضف مدخلاً مؤرخاً append-only في docs/ARABIC_ENGINE_WORKLOG.md.
3. حدّث docs/ARABIC_ENGINE_HANDOFF.md ونقطة الاستئناف.
4. حدّث docs/ARABIC_ENGINE_SUPPORT.md عند تغير تفاصيل دعم المحرك.
5. حدّث PROGRESS.md أو README.md عند تغير ملخص المشروع.
6. حدّث الوثيقة التخصصية: ARCHITECTURE أو FILE_MAP أو BUILD_TESTING أو ROADMAP بحسب نوع التغيير.

كل ذلك يدخل شجرة commit نفسها مع تعديل الكود. لا تسجل commit برمجياً ثم تؤجل التوثيق إلى commit لاحق. تحديث Markdown وحده مسموح في commit توثيقي مستقل.

## مرجع الحقيقة وترتيب القراءة
عند بدء محادثة أو جلسة جديدة، لا تعتمد على ذاكرة محادثة قديمة. اقرأ:
1. AGENTS.md
2. docs/PROJECT_STATE.md
3. docs/NEXT_SESSION_PROMPT.md وdocs/ARABIC_ENGINE_HANDOFF.md
4. docs/ROADMAP.md وdocs/BUILD_TESTING.md
5. docs/ARCHITECTURE.md وdocs/FILE_MAP.md
6. docs/DOCUMENTATION_RULES.md وdocs/ARABIC_ENGINE_WORKLOG.md
7. docs/ARABIC_ENGINE_SUPPORT.md وPROGRESS.md وREADME.md
8. SOURCE_VERSION.md وSOURCE_TEMPLATES.md وGLOSSARY.md وluanti/5.17.0/ARABIC_TEXT_FIX.md
9. بقية ملفات Markdown المتتبعة ذات الصلة.

بعد ذلك افحص HEAD والفرق وسجلات Actions. ترتيب الحقيقة: GitHub HEAD والملفات والسجلات، ثم PROJECT_STATE، ثم WORKLOG، ثم بقية الشروحات. عند الاختلاف، وثّق الاختلاف وصحح الوثائق في الدفعة نفسها.

## قواعد البناء والاختبار
- لا تعلن نجاح build أو test أو package أو runtime دون سجل أو Artifact يثبته.
- افصل بين الفحص الساكن وLua/PO validation وCMake configure وC++ build وunit test وpackage install والاختبار الفعلي.
- لا تضعف assertions لجعل CI أخضر؛ اجعل الاختبار متفقاً مع خصائص الخط المرفق.
- نجاح Windows لا يعوض فشل Linux.
- حافظ على vcpkg binary cache restore/save في المنصتين.
- CI مكلف؛ اجمع الإصلاحات المتماسكة، نفذ preflight، ثم شغّل CI واحداً شاملاً.
- workflow الحالي يستخدم cancel-in-progress؛ push جديد قد يلغي تشغيل الفرع السابق.
- لا تعتبر وجود run object أو نجاح validation وحدها نتيجة بناء.

## قواعد فحص الكود
- اقرأ السياق حول الدالة قبل تعديلها، ولا تستبدل ملفاً كاملاً اعتماداً على مقتطف.
- حافظ على التوافق مع Luanti 5.17.0 وC++17 وvcpkg baseline المثبت.
- اختبر UTF-8/UTF-16، المؤشر والتحديد، CRLF/LF، النص الفارغ، الحد الأقصى، fallback fonts، BiDi وshaping clusters.
- لا تحول font إلى CGUITTFont قبل إثبات النوع EGFT_CUSTOM.
- grapheme boundary ليست بالضرورة shaping-cluster boundary.
- خطأ Lua syntax وتحذير SHA-256 سببان منفصلان.
- لا تحذف أصولاً أو ملفات ترجمة أو بيانات مصدر دون سبب واختبار.

## قواعد التسليم
في نهاية كل دفعة، اذكر المستودع والفرع والرأس الجديد، الملفات المتغيرة، الفحوص ونتائجها، CI run ID/jobs، Artifact IDs وSHA عند وجودها، وما لم يُختبر. حدّث نقطة العمل التالية في PROJECT_STATE وNEXT_SESSION_PROMPT.

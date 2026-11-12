# تسليم دعم العربية في محرك Luanti — نقطة الاستئناف الرسمية

التاريخ: 2026-11-12
Repository: majd7772233-a11y/Mineclonia-Arabic
Branch الوحيد: work/arabic-engine-support
رأس تنفيذ Workflow/Lua: 69c62784baf96e3456239d5b954055be5e855501
main المفحوص: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (لم يُعدّل).
Run الأخير: #109 / 43212062926 على SHA 69c62784baf96e3456239d5b954055be5e855501 — completed/success.

## ما اكتمل

- أُصلح Workflow في commit 69c62784baf96e3456239d5b954055be5e855501: أربع Jobs فريدة sanity/catalog/Linux/Windows؛ أزيلت المقاطع اليتيمة والتعريفات المكررة؛ اكتملت شروط Bash وPowerShell.
- صُحح double escaping في tools/validate_lua_syntax.py.
- حُفظ cancel-in-progress=true وvcpkg cache restore/save في Linux وWindows.
- Run #109 نجح بكل وظائفه الأربع.
- catalog: 245/245 builtin keys و1,575/1,575 gettext translations؛ 0 empty و0 fuzzy.
- msgfmt: success.
- Lua parser: 210 ملفات builtin عبر الشجرتين.
- Arabic source validator: 17 declarations / 17 registrations / 17 definitions.
- Linux وWindows: CMake configure، client build، اختبار TestArabicTextLayout (17/17 و0 failures)، install/package/upload — كلها success.
- Linux artifact ID 13941099252، SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows artifact ID 13941743588، SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.

## Run #109 وروابط الحزم

Run: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926

- Linux ZIP/tar.gz artifact: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941099252
  - ID 13941099252; 22,718,197 bytes; SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows ZIP artifact: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941743588
  - ID 13941743588; 20,672,848 bytes; SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.

## ما لم يختبر بعد

- لم يُشغّل المستخدم Windows artifact على جهازه بعد.
- لا يوجد إثبات فعلي حتى الآن لاختفاء خطأ قائمة Lua أو تحذيرات SHA-256 على جهاز المستخدم.
- clipboard copy/cut/paste، mouse drag، Shift selection، Ctrl+Left/Right، Ctrl+Backspace/Delete بالنص المختلط، scrolling، clipping، shadows، centering/alignment، emoji، supplementary Unicode وcombining marks تحتاج اختبار GUI.
- Android مؤجل إلى ما بعد استقرار Desktop.

## نقطة التوقف التالية بدقة

1. استخدم Windows artifact 13941743588 كحزمة كاملة مستقلة؛ لا تنقل exe وحده إلى تثبيت قديم.
2. شغّل التطبيق وافتح القائمة الرئيسية.
3. افتح Settings > Language وتحقق من ظهور العربية واختيارها.
4. التقط log جديداً وابحث عن `Failed to load and run script` و`No SHA256 known for builtin file`. لا تفترض اختفاء التحذير قبل وجود log المستخدم.
5. ابدأ اختبارات التحرير التفاعلية المذكورة أعلاه، ثم Rendering GUI.
6. إذا ظهر فشل جديد، اقرأ السجل والكود المحيط أولاً، وجمّع الإصلاحات مع توثيق Markdown في commit واحد. لا تدفع أثناء CI بسبب cancel-in-progress=true.

هذا التحديث بعد Run #109 توثيقي فقط؛ لا يعدل كود المحرك أو Workflow.

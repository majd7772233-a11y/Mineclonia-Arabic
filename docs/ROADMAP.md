# خارطة الطريق

## P0 — CI وLua validator

- [x] تحليل Run108 من GitHub: failure دون Jobs/Artifacts/check-runs؛ لا logs تحدد الفشل.
- [x] إعادة بناء Workflow بأربع Jobs فريدة: workflow-sanity وcatalog-validation وLinux وWindows.
- [x] إزالة package fragments وWindows jobs المكررة وإكمال Bash/PowerShell conditions.
- [x] الحفاظ على cancel-in-progress وكاش vcpkg restore/save للمنصتين.
- [x] إصلاح double escaping في Arabic option regex دون تغيير Lua source السليم.
- [x] PyYAML وbash -n وregex preflight.
- [x] CI: catalog/Lua/PO/source validation نجحت في Run #109.

## P1 — Desktop build verification

- [x] Linux وWindows CMake configure.
- [x] Linux وWindows client build.
- [x] مصدر اختبارات C++: 17/17/17 بلا تكرار.
- [x] C++ compilation الفعلي على النظامين.
- [x] TestArabicTextLayout: 17/17 و0 failures على النظامين.

## P2 — Tests and packaging

- [x] Install/package verification على Linux وWindows.
- [x] Linux executable وbuiltin/font/license وldd dependency check.
- [x] Windows executable وbuiltin SHA/comma/font/license وDLL checks.
- [x] Linux Artifact ID 13941099252 وSHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- [x] Windows Artifact ID 13941743588 وSHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.
- [x] تنزيل ZIPs المنصتين والتحقق من SHA-256 المطابق لـupload action وسلامة الأرشيف الخارجي.
- [x] تشغيل Linux executable من الحزمة المستخرجة والتحقق من الإصدار وldd.
- [x] تشغيل TestArabicTextLayout من Linux package نفسه: 17/17، صفر إخفاقات.

## P3 — Windows user verification

- [ ] تسليم واختبار Windows artifact 13941743588 كحزمة كاملة.
- [ ] فتح القائمة الرئيسية وSettings > Language واختيار العربية.
- [ ] فحص Lua parser errors وSHA-256 warnings في سجل مستخدم جديد.
- [ ] إصلاح أي مشكلة باقية من log جديد وسياق المصدر، لا بتعطيل التحذيرات.

## P4 — Real-client interaction and rendering

- [ ] Clipboard copy/cut/paste round-trip.
- [ ] Mouse drag وShift selection.
- [ ] Ctrl+Left/Right وCtrl+Backspace/Delete للنص المختلط.
- [ ] clipping/shadows/centering/alignment/scrolling.
- [ ] fallback glyphs وemoji وsupplementary Unicode وcombining marks.
- [ ] Android بعد استقرار Desktop.

## Definition of Done

الـDesktop CI والحزمتان اجتازتا Run #109، لكن دعم العربية لا يكتمل إلا بعد GUI/interaction verification فعلي وتوثيق النتائج أو الأخطاء المتبقية.
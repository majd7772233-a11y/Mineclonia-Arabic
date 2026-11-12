# خارطة الطريق

الأولوية: استعادة workflow صالح، ثم إثبات build والاختبارات.

## P0 — CI configuration
- [ ] إعادة بناء .github/workflows/build-luanti-arabic-engine.yml.
- [ ] إزالة المقاطع اليتيمة بعد Windows upload والـjob windows المكرر.
- [ ] إغلاق Linux grep عند line 187.
- [ ] إكمال PowerShell Select-String عند line 332.
- [ ] التأكد من وجود workflow-sanity وcatalog-validation وLinux وWindows مرة واحدة فقط.
- [ ] الحفاظ على vcpkg cache restore/save.
- [ ] YAML validation/actionlint قبل push.
- [ ] إصلاح Lua regex واختباره على الشجرتين.

## P1 — build verification
- [ ] validators: catalogs، PO formatting، Lua syntax، Arabic test source.
- [ ] Linux وWindows compile كامل.
- [ ] تأكيد source validator 17/17/17.
- [ ] لا تضعف assertions.

## P2 — tests and packaging
- [ ] 17/17 Arabic tests على Linux وWindows.
- [ ] معالجة أي compile/link/test errors من logs.
- [ ] verify packaged builtin file matches source.
- [ ] verify runtime DLLs/fonts/license.
- [ ] Artifact لكل منصة مع IDs وSHA-256.

## P3 — Windows user verification
- [ ] تسليم Artifact Windows حديث.
- [ ] فتح القائمة/Settings/Language.
- [ ] التأكد من عدم وجود Lua parser error.
- [ ] التأكد من اختفاء SHA warnings مع exe وbuiltin من الحزمة نفسها.
- [ ] إذا بقي التحذير، سجل أول مسار وhash للملفات.

## P4 — real-client interaction
- [ ] copy/cut/paste round-trip.
- [ ] mouse drag selection.
- [ ] Shift selection.
- [ ] Ctrl+Left/Right وCtrl+Backspace/Delete في BiDi.
- [ ] clipping/shadows/centering/alignment/scroll.
- [ ] fallback glyphs وemoji وsupplementary Unicode.
- [ ] Android toolchain بعد تثبيت desktop.

## Definition of Done
workflow صالح، validators passed، Linux/Windows builds passed، 17/17 tests على المنصتين، artifacts package checks passed، تجربة Windows فعلية، وفحوص التحرير التفاعلية موثقة أو المتبقي محدد.

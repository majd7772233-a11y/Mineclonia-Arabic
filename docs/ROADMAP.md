# خارطة الطريق

## P0 — CI وLua validator

- [x] تحليل Run108 من GitHub: failure دون Jobs/Artifacts/check-runs؛ لا logs تحدد الفشل.
- [x] مراجعة workflow كاملاً ورصد Bash/PowerShell المبتور وpackage tails وWindows jobs المكررة.
- [x] تنظيم Workflow بأربع Jobs: workflow-sanity وcatalog-validation وLinux وWindows.
- [x] الحفاظ على cancel-in-progress وكاش vcpkg restore/save للمنصتين.
- [x] إصلاح double escaping في Arabic option regex دون تغيير Lua source السليم.
- [x] PyYAML وbash -n وregex preflight.
- [ ] actionlint وPowerShell parser وmsgfmt عندما تتوفر الأدوات.
- [ ] تأكيد catalog/Lua/PO validation في CI الجديد.

## P1 — build verification

- [ ] Linux وWindows CMake configure.
- [ ] Linux وWindows client build.
- [ ] إصلاح compile/link errors من السجلات دون تغيير assertions.
- [ ] إثبات source validator 17/17/17 وC++ compilation الفعلي.

## P2 — tests and packaging

- [ ] 17/17 Arabic tests على Linux وWindows.
- [ ] Verify packaged builtin matches source.
- [ ] Verify Noto Sans Arabic font and license.
- [ ] Verify Linux runtime dependencies وWindows DLLs.
- [ ] Artifact لكل منصة مع ID وSHA-256/digest.

## P3 — Windows user verification

- [ ] تسليم Windows artifact حديث مطابق لرأس البناء.
- [ ] فتح القائمة الرئيسية وSettings > Language.
- [ ] التأكد من عدم ظهور Lua parser error.
- [ ] تجربة SHA warnings مع exe وbuiltin من الحزمة نفسها.
- [ ] جمع log جديد إذا استمر التحذير.

## P4 — real-client interaction

- [ ] Clipboard copy/cut/paste round-trip.
- [ ] Mouse drag وShift selection.
- [ ] Ctrl+Left/Right وCtrl+Backspace/Delete للنص المختلط.
- [ ] clipping/shadows/centering/alignment/scrolling.
- [ ] fallback glyphs وemoji وsupplementary Unicode وcombining marks.
- [ ] Android بعد استقرار Desktop.

## Definition of Done

Validators، وLinux/Windows builds، و17/17 على النظامين، وpackages verified، وArtifacts مبصومة ومطابقة، ثم تجربة GUI فعلية. فحص الترجمة أو YAML لا يكفي لإعلان اكتمال دعم العربية.

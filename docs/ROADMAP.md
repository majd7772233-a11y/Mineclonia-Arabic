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

## Windows portable profile — four implementation phases

This work is scoped to the Windows RUN_IN_PLACE / ZIP distribution used for desktop testing. Linux and other platform path behavior remains unchanged in this first implementation.

### Phase 1 — Separate application files from writable data
- [x] Keep path_share at the package root (read-only builtin, fonts, textures, locale, and engine resources).
- [x] Set Windows RUN_IN_PLACE path_user to <package>/Minetest through getPortableUserPath().
- [x] Let all path_user-derived data (games, worlds, config, logs, cache, screenshots/content profile data) resolve under that profile.
- [x] Add a TestFileSys contract assertion for the portable profile path.
- [ ] Confirm Windows and Linux build/test results in CI.

### Phase 2 — First-launch migration and path isolation
- [x] On Windows, copy the existing %APPDATA%/Minetest profile into the portable profile only when the destination does not exist.
- [x] Use a staging directory and activate the copy only after a complete copy; leave the old profile untouched.
- [x] Do not silently merge AppData into an existing portable profile.
- [x] Keep the cache inside the portable profile and copy the existing cache with the rest of the profile.
- [x] Prevent Windows RUN_IN_PLACE config lookup from falling back to files above the portable profile.
- [ ] Verify migration and non-modification of the legacy profile in the Windows package smoke test.

### Phase 3 — Package regression coverage and documentation
- [x] Add TestFileSys to both Linux and Windows CI.
- [x] Add a Windows portable-package integration probe using a cloned package and a synthetic AppData profile.
- [x] Check that worlds are discovered from Minetest/worlds and games/config/cache are copied into Minetest.
- [x] Check that the original AppData profile is retained and not modified.
- [x] Document the Windows ZIP directory layout in Luanti's README.
- [ ] Wait for CI evidence before marking the implementation validated.

### Phase 4 — Windows user acceptance and portability audit
- [ ] Run the new Windows ZIP on the user's machine and verify the profile appears at luanti/Minetest.
- [ ] Verify Content tab game downloads, world creation, config saves, logs, screenshots, texture packs, and cache are written inside the portable tree.
- [ ] Verify no new data is written to %APPDATA%/Minetest after migration.
- [ ] Test launch from a path containing spaces and from a removable/non-system drive.
- [ ] Confirm upgrade behavior when Minetest already exists and preserve both profiles safely.
- [ ] Only after desktop acceptance, decide whether any extra platform-specific portable behavior is needed.

Do not call the application fully portable until Phase 4 passes on a real Windows installation.

## Run #110 validation note

Run #110 (43231076763, HEAD 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427) passed Linux build/tests/package and passed Windows Configure/Build plus both C++ suites (17/17 and 14/14). Windows install and DLL checks also passed, but the package smoke step failed at its duplicate installed-package TestFileSys invocation; the process output was absent from the job log, so no migration defect was established. The Windows Artifact was not created. The corrected package smoke test now checks first/second world discovery and the migrated tree itself; the independent Windows TestFileSys test remains mandatory. Phase 3 is not complete until the next Windows package job passes.

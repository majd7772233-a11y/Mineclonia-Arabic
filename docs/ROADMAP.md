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
- [x] Confirm Windows and Linux base builds and the 16-test FileSys suite in CI (Run #116).

### Phase 2 — First-launch migration and path isolation
- [x] On Windows, copy the existing %APPDATA%/Minetest profile into the portable profile only when the destination does not exist.
- [x] Use a staging directory and activate the copy only after a complete copy; leave the old profile untouched.
- [x] Do not silently merge AppData into an existing portable profile.
- [x] Keep the cache inside the portable profile and copy the existing cache with the rest of the profile.
- [x] Prevent Windows RUN_IN_PLACE config lookup from falling back to files above the portable profile.
- [x] Verify first-run migration and legacy AppData immutability in the Windows package smoke test (Run #116).

### Phase 3 — Package regression coverage and documentation
- [x] Add TestFileSys to both Linux and Windows CI.
- [x] Add a Windows portable-package integration probe using a cloned package and a synthetic AppData profile.
- [x] Confirm migrated world/game/config/cache files and contents under Minetest (Run #116).
- [x] Confirm that the legacy AppData profile remains content-identical after migration (Run #116 SHA-256 file inventory comparison).
- [x] Document the Windows ZIP directory layout in Luanti's README.
- [x] Wait for CI evidence before marking the automated implementation validated (Run #116).

### Phase 4 — Windows user acceptance and portability audit
**Automated CI coverage — completed in Run #116:**
- [x] Launch the package from a directory containing spaces and verify that the legacy profile is copied to the portable directory.
- [x] Launch again and verify that the portable profile remains authoritative.
- [x] Start with an existing portable Minetest directory and verify that AppData is not merged or allowed to overwrite it.
- [x] Verify that both worlds and downloaded-game discovery resolve beneath a path_user directory containing spaces (TestFileSys 16/16).

Real Windows acceptance still required:
- [ ] Run the new Windows ZIP on the user's machine and verify the profile appears at luanti/Minetest.
- [ ] Verify Content tab game downloads, world creation, config saves, logs, screenshots, texture packs, and cache are written inside the portable tree.
- [ ] Verify no new data is written to %APPDATA%/Minetest after migration.
- [ ] Test launch from a removable/non-system drive.
- [ ] Confirm upgrade behavior when Minetest already exists and preserve both profiles safely.
- [ ] Only after desktop acceptance, decide whether any extra platform-specific portable behavior is needed.

Do not call the application fully portable until Phase 4 passes on a real Windows installation.

## Run #110 validation note

Run #110 (43231076763, HEAD 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427) passed Linux build/tests/package and passed Windows Configure/Build plus both C++ suites (17/17 and 14/14). Windows install and DLL checks also passed, but the package smoke step failed at its duplicate installed-package TestFileSys invocation; the process output was absent from the job log, so no migration defect was established. The Windows Artifact was not created. The follow-up smoke test then asserted captured first/second `--worldlist=path` output. Run #111 showed that parsing the GUI-subsystem process output was invalid. Run #112 then exposed an additional, concrete command-line bug: Luanti expects `--worldlist path` as two arguments, but the test used `--worldlist=path`. The parser rejected the option and the process did not initialize the portable profile; the later on-disk check correctly failed because `minetest.conf` was absent. The smoke test now uses the documented two-argument form and keeps the on-disk and no-merge assertions. Phase 3 is not complete until the next Windows package job passes.


## Run #111 failure analysis and corrective test design

Run #111 (ID 43236622672, implementation HEAD bb3df102f129b2ef11b50f16911f292e20881f74) passed workflow sanity, catalog validation, Linux build/tests/package/upload, and Windows configure/build plus TestArabicTextLayout 17/17 and TestFileSys 14/14. The Windows package job failed because the smoke test required captured stdout from `luanti.exe --worldlist=path`. The MSVC release link flags explicitly use `/SUBSYSTEM:WINDOWS`, so this GUI executable does not provide a dependable console-output stream in the headless PowerShell runner. The blank output is an invalid test assertion, not evidence that profile migration failed. The run stopped before checking the migrated profile on disk. Linux Artifact 13953262107 was uploaded (22,719,344 bytes; SHA-256 `86dd6ecab7085bcde58b03fbd2225e347bed4326e86c46753fbf28070bc6889d`); no Windows artifact was uploaded.

The correction replaces stdout parsing with process exit-code and filesystem/content assertions, adds a C++ regression test that calls `getAvailableWorlds()` and `getAvailableGameIds()` with a temporary user profile path containing spaces, and adds an isolated second-package test for the existing-profile/no-merge rule. The Windows GUI, removable-drive and real user-data checks remain manual Phase 4 acceptance items.


## Run #112 — exact Windows package failure and next correction

Run #112 (ID 43245244962, HEAD cc9ff808542bafd0d5849203684798d74eaf2fcf) passed scheduling, catalog/source validation, Linux build/tests/package/upload, and Windows Configure/Build plus TestArabicTextLayout 17/17 and TestFileSys 15/15. The Linux artifact was 13957061595 (22,727,482 bytes; SHA-256 `660232c9ddeae37fabe2e9a2264be3fa0d0493145b96c8da8cfd76a0306a756f`). The Windows package smoke test failed because the client reported `Unknown command-line parameter "--worldlist=path"`. Its help text lists `--worldlist <value>`, so the correct invocation is two separate arguments: `--worldlist path`. Since option parsing rejected the command, startup did not reach path initialization/migration; the subsequent file check correctly found no portable `minetest.conf`. The Windows artifact was therefore skipped.

The follow-up changes all three package invocations to `& $exe --worldlist path`. It retains both first-run migration and existing-profile/no-merge filesystem checks. The invalid stdout assertion remains removed because this client is linked with `/SUBSYSTEM:WINDOWS`; package acceptance is based on generated files and contents after a valid command invocation. This correction requires another full CI run.


## Run #113 — staging-path collision during Windows first-launch migration

Run #113 (ID 43247942695, HEAD e0b1fc5bf517f216523f5a53d3d6b049673432d0) passed scheduling and catalog/source validation. Linux configure/build, TestArabicTextLayout 17/17, TestFileSys 15/15, package install/upload all passed. Linux Artifact 13957134160: 22,723,769 bytes; SHA-256 `9b4178bba4ac2e284e51b85b170a3311b5d8f4adb753cd9323be6f67eb290ee6`.

Windows configure/build and both C++ suites passed (Arabic layout 17/17; FileSys 15/15), but the package smoke failed after valid `--worldlist path` invocations. The log reported that the fixed staging path `...in\..\Minetest.migration-tmp` already existed, then could not activate the portable profile; `minetest.conf` consequently remained absent. The log proves a staging-path collision and failed activation, but does not establish who or what originally created that path. No Windows artifact was uploaded.

The next corrective batch:
- normalizes an existing Windows RUN_IN_PLACE share path through `fs::AbsolutePath()` before deriving the portable profile, removing the `bin\..` path component from the writable destination;
- allocates a process-specific sibling staging path (`Minetest.migration-tmp-<pid>`, with a numeric suffix on collision), preserving older staging directories instead of deleting or overwriting them;
- reports the CRT rename error code/text if activation fails;
- adds a FileSys regression for a share path containing `bin/..`;
- cleans CI package/probe directories and injects an explicit old-style staging directory with a marker, then checks it remains unchanged and that no new temporary staging directory remains.

Phase 2/3 migration verification remains incomplete until a new Windows package smoke succeeds and the Windows artifact uploads. Phase 4 still requires testing the ZIP on a real Windows installation; do not begin font/UI work yet.

## Run #114 — world discovery passed; root-level minetest.conf still missing

Run #114 (ID 43289640898, tested HEAD 334f6fba00c9a093b2e5511a2987f45aab91296e) passed workflow scheduling and Arabic catalog/source validation. Linux configure/build, TestArabicTextLayout 17/17, TestFileSys 15/15, install/package and upload all passed. Linux Artifact 13976243943: 22,728,834 bytes; SHA-256 5966c9a2f7be474973eaa12881970ae71778b76a8aee77b6549f8d933a5ef1bb.

Windows configure/build passed; TestArabicTextLayout 17/17 and TestFileSys 15/15 passed. The Windows package command used the corrected --worldlist path syntax, and its output listed the synthetic world twice beneath the portable Minetest/worlds path. The subsequent filesystem assertion failed because Minetest/minetest.conf was missing. No Windows artifact was uploaded.

This shows the package initialized and world data was visible in the portable profile, but it does not yet establish that every root-level and nested file was copied. The current workflow did not print the source and destination inventories when the assertion failed, so the missing root-level config's exact cause remains undetermined. Do not change the production migration implementation based solely on that incomplete diagnostic.

### Current corrective batch — diagnostic and copy regression
- Add a 16th TestFileSys case that calls fs::CopyDir() on a source directory containing a root-level minetest.conf and a nested world.mt, with spaces in the source/destination paths, then compares both copied file contents.
- The Windows smoke test now explicitly verifies the synthetic AppData config fixture exists before launch, logs the seeded source inventory, and on failure reports missing paths plus full source/destination file inventories.
- Update both Linux and Windows CI gates to require TestFileSys 16/16.
- Keep the package's existing assertions for first/second launch, migrated file contents, cache tag, AppData file hashes, stale-staging marker preservation, no process staging leftovers, and existing-profile/no-merge.

**Phase status:** still in Phase 3 (package regression coverage and documentation). Automated checks from Phase 4 (path spaces, second launch, stale staging, and no-merge) are also under test. Phase 3 cannot be marked complete until the Windows package smoke and artifact upload pass. Phase 4 still requires real Windows acceptance for ContentDB, new worlds, config/log/screenshot/texture/cache writes, no new AppData writes, and removable/non-system drive behavior.


## Run #116 — automated portable-profile acceptance passed

Run #116 (ID 43376273868) tested HEAD `b0c486db3a0960c7779c2f341198fe267356f61b` and completed successfully on both operating systems. All four workflow jobs passed: scheduling, Arabic catalog/source validation, Windows x64 build/tests/package, and Linux x64 build/tests/package.

Both hosts passed TestArabicTextLayout 17/17 and TestFileSys 16/16. The Windows trace proves that CopyFileEx copied the synthetic legacy cache marker, downloaded-game metadata, root-level `minetest.conf`, and world metadata into a unique process-specific staging directory, then activated the portable `Minetest` profile. The second launch detected that profile and skipped another merge. The smoke test passed content comparisons, AppData before/after SHA-256 inventory comparison, stale-staging marker preservation, cleanup of the new process staging directory, and the existing-profile/no-merge scenario. Package paths containing spaces and direct world/game discovery checks also passed.

Artifacts and exact hashes are recorded in `docs/PROJECT_STATE.md` and `docs/ARABIC_ENGINE_WORKLOG.md`.

**Phase boundary:** Phases 1–3 and all four automated Phase 4 checks are complete. The remaining Phase 4 items require actual use on the user's Windows installation: Content tab downloads, creating a world, config/log/screenshot/texture/cache writes, no new AppData writes, and a non-system/removable-drive launch if available. Do not declare full real-world portability until those checks pass.



## Run #117 — Windows smoke regression after Run #116

**Current tested HEAD:** `efbf5dc3b1d73b99df3ec1a14a30666540d729ca`  
**Run:** [#117 / 43382276907](https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43382276907) — completed / failure  
**Protected main:** `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d` (unchanged).

### Actual results
- Scheduling and Arabic catalog/source validation: passed.
- Linux x64: configure/build, TestArabicTextLayout 17/17, TestFileSys, package install/upload all passed. Linux Artifact 14017277623: 22,717,242 bytes; SHA-256 `0f6e51221dbf526c6898d4897f4bf729e7569a5540bf1c8d1f6e75e874a2fc8c`.
- Windows x64: configure/build, TestArabicTextLayout 17/17 and the then-current 14-case TestFileSys passed.
- Windows package smoke failed immediately after the first `--worldlist=path` invocation because the workflow tried to match a path in captured stdout. The GUI-subsystem executable returned without captured console text; the logged output was blank, and the script threw at the stdout assertion before checking the migrated files. Windows Artifact upload was skipped.
- Therefore Run #117 does **not** prove that the production copy failed. It demonstrates that the stdout-based smoke assertion was reintroduced and was invalid for this Windows GUI executable.

### Regression found by comparing Run #116's tested HEAD with Run #117's HEAD
The latest commit changed four validated files relative to `b0c486db3a0960c7779c2f341198fe267356f61b`:
- `getPortableUserPath()` no longer normalizes the executable-derived `bin/..` share path with `fs::AbsolutePath()`.
- Migration staging reverted from a PID-specific sibling with collision suffixes to the fixed `Minetest.migration-tmp` name.
- TestFileSys lost the direct `CopyDir` root/nested-file regression and the path-normalization assertions; the suite returned from 16 to 14 cases.
- The package workflow lost the trace-based on-disk migration diagnostics and reverted to matching world paths from captured stdout.

### Corrective batch
Restore the exact four source/workflow files from the successful Run #116 tested commit. That version passed both platform builds and suites (Arabic layout 17/17; FileSys 16/16), and the Windows trace proved first/second launch, file copying, activation, staging collision recovery, AppData immutability and no-merge. This is a restoration of already-tested code, not a new production rewrite.

**Current validation boundary:** Run #116 remains valid evidence for its own HEAD, but Run #117 tested a later, regressed HEAD. The restored batch must pass a fresh full Run #118 before the current branch is treated as CI-accepted again. Keep the Windows real-device acceptance and font/UI work deferred until that run succeeds.

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


## فحص الـArtifacts بعد التنزيل

بعد Run #109 تم تنزيل الحزمتين وفحصهما بشكل مستقل:
- Linux Artifact 13941099252: SHA-256 المنزّل يطابق `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`. الأرشيف سليم، وELF يعمل من جذر الحزمة، و`--version` يعرض Luanti 5.17.0. `ldd` بلا مكتبات مفقودة.
- من نفس الحزمة المستخرجة، نجح `TestArabicTextLayout` بنتيجة 17/17 وصفر إخفاقات.
- ملف builtin الخاص باختيار اللغة وخط Noto والترخيص موجودة.
- Windows Artifact 13941743588: SHA-256 المنزّل يطابق `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`، وZIP الداخلي سليم، ووجدت exe وDLLs وbuiltin وخط Noto والترخيص. لم يتم تشغيل exe على Windows في هذه البيئة.

## نقطة الاستئناف الآن

1. Windows artifact 13941743588 هو الخطوة التالية على جهاز المستخدم: استخرج الحزمة كاملة وشغّل `bin/luanti.exe` منها.
2. افتح القائمة الرئيسية ثم Settings > Language، واختر العربية.
3. احتفظ بسجل تشغيل جديد؛ لا تعتبر Lua menu error أو SHA warnings منتهية قبل التحقق من السجل.
4. استكمل اختبارات clipboard round-trip وmouse drag وShift selection والتنقل والحذف المختلط، ثم Rendering GUI.
5. لا تبدأ Android قبل استقرار Desktop.

## 2026-11-12 — Windows portable profile work

Starting point: 01a3dbbb196da678448211f818f9f5ab0a99f8d0 on work/arabic-engine-support. The protected main head checked before edits was a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d.

The Windows RUN_IN_PLACE profile is being separated from the app/share root:
- share stays at the Luanti package root.
- user data moves to <package>/Minetest.
- path_cache remains under path_user.
- Windows config search is restricted to Minetest/minetest.conf.
- Existing %APPDATA%/Minetest is copied on first launch using a staging directory; the source remains untouched. If the portable directory already exists, no automatic merge occurs.

The batch also adds a portable path unit test and Windows package integration test in the build workflow. TestFileSys is expected to report 14 tests on both platforms.

**Validation status:** not built or run yet. Workflow YAML, PowerShell and C++ still need preflight and the full CI run. Do not claim the application is portable until the Windows smoke test and the user's Windows GUI acceptance both pass.

**Next step:** inspect the final diff, run available YAML/Bash/PowerShell/C++ preflight, re-check branch refs, commit all code and Markdown together, then wait for one complete CI. After CI, test the Windows artifact on the user's computer.

## Handoff update after Run #110

Run #110 (43231076763) for commit 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427:
- Linux build, Arabic tests 17/17, TestFileSys 14/14, install verification and artifact upload passed.
- Windows configure/build, Arabic tests 17/17, TestFileSys 14/14 and install/runtime DLL checks passed.
- Windows packaging failed only in the package smoke step, during a duplicate TestFileSys invocation from the copied install tree. Its process output was empty in the job log; do not claim a specific C++ or migration root cause from this.
- The first packaged `--worldlist=path` process returned exit code 0.
- Linux artifact exists (ID 13950892008); Windows artifact was not uploaded.

The follow-up workflow correction removes only the duplicate package-level TestFileSys invocation. It keeps the mandatory build-tree TestFileSys checks on both platforms and strengthens the actual package probe with first- and second-launch world discovery, migrated-file content checks, and AppData immutability.

Next: inspect the exact patch, verify branch/main refs, commit the correction with all documentation in the same commit, then inspect the new CI run. If Windows smoke passes, inspect Artifact ID, size and digest before giving the Windows ZIP to the user.


## Handoff after Run #111 — fix the Windows smoke harness and advance the portable audit

Run #111 (43236622672, HEAD bb3df102f129b2ef11b50f16911f292e20881f74) compiled and tested both native targets. Linux passed TestArabicTextLayout 17/17, TestFileSys 14/14, package checks, and upload (Artifact 13953262107; 22,719,344 bytes; SHA-256 86dd6ecab7085bcde58b03fbd2225e347bed4326e86c46753fbf28070bc6889d). Windows passed configure/build, 17/17 Arabic tests, 14/14 FileSys tests, install and DLL checks, but failed the package smoke test because it expected stdout from a Windows GUI executable linked with `/SUBSYSTEM:WINDOWS`. The process returned 0 and output was empty; the script stopped before checking the copied profile. Do not describe this as a proven migration bug. No Windows artifact was uploaded.

This follow-up changes the package test to rely on exit codes and on-disk content, adds a 15th TestFileSys case to verify that the actual world/game discovery functions use a path_user containing spaces, and tests both first-run migration and the existing-profile/no-merge case from package directories containing spaces.

Next steps after this commit:
1. Re-fetch live refs; only move `work/arabic-engine-support` from the expected previous SHA, and leave `main` untouched.
2. Run one full CI workflow. Check catalog validation, configure/build, TestArabicTextLayout 17/17, TestFileSys 15/15, Windows smoke checks, and both artifact uploads.
3. If Windows fails, diagnose the exact failed assertion and do not weaken assertions just to make CI green.
4. After CI success, provide the newly uploaded Windows artifact and request real-device Phase 4 acceptance. Test Content tab downloads, creating worlds, config/log/screenshot/cache paths, no new AppData writes, and a removable/non-system drive.
5. Do not move to font/UI or RTL rendering work until portable-profile acceptance is verified. Android stays deferred.


## Handoff after Run #112 — fix the command-line option form

Run #112 (43245244962; HEAD cc9ff808542bafd0d5849203684798d74eaf2fcf) confirms the new 15-test C++ suite compiles and passes on both hosts: Windows TestArabicTextLayout 17/17 and TestFileSys 15/15; Linux passed the same suites plus package install/upload. Linux Artifact 13957061595 (22,727,482 bytes; SHA-256 660232c9ddeae37fabe2e9a2264be3fa0d0493145b96c8da8cfd76a0306a756f) is available. Windows package smoke failed because `--worldlist=path` is not valid syntax for this engine; the parser expects `--worldlist path` as two arguments and logged `Unknown command-line parameter "--worldlist=path"`. The app therefore never reached path initialization or profile migration, and the missing config check was expected. No Windows artifact was uploaded.

The current workflow correction replaces all three invocations:
- first launch: `& $probeExe --worldlist path`
- second launch: `& $probeExe --worldlist path`
- existing-profile launch: `& $existingProbeExe --worldlist path`

Next steps:
1. Re-check live refs; commit the workflow syntax correction together with the Run #112 Markdown updates, fast-forwarding only `work/arabic-engine-support`.
2. Wait for the new full CI run to finish. Confirm TestFileSys 15/15 on both hosts, Windows first-run migration and second-start filesystem assertions, existing-profile/no-merge, AppData file hashes, and Windows artifact upload.
3. If the smoke still fails, diagnose the exact command output and filesystem state; do not weaken assertions.
4. After successful upload, provide the Windows ZIP and continue Phase 4 real-device acceptance: ContentDB download, world creation, config/log/screenshot/texture/cache paths, no new AppData writes, and removable/non-system drive.
5. Keep font/UI work deferred until the portable-profile acceptance is complete.


## Handoff after Run #113 — normalize Windows profile path and avoid staging collisions

Run #113 (43247942695; tested HEAD e0b1fc5bf517f216523f5a53d3d6b049673432d0) passed catalog validation and the complete Linux build/test/package/upload job. Linux TestArabicTextLayout 17/17; TestFileSys 15/15; Artifact 13957134160 (22,723,769 bytes; SHA-256 9b4178bba4ac2e284e51b85b170a3311b5d8f4adb753cd9323be6f67eb290ee6). Windows configure/build, Arabic tests 17/17 and FileSys 15/15 passed, but its package smoke failed after the valid `--worldlist path` command reached initialization. The engine reported an already-existing fixed staging path `...in\..\Minetest.migration-tmp`, could not activate the portable profile, and the later file check found no `minetest.conf`. The origin of the already-existing stage was not proven. No Windows artifact was uploaded.

The current corrective batch:
- normalizes existing share paths with `fs::AbsolutePath()` before appending the portable `Minetest` directory, eliminating the executable-derived `bin/..` components from the writable profile path;
- uses a process-specific sibling staging path with collision suffixes, preserving any old staging directory;
- includes the underlying rename error in logs;
- tests the `bin/..` normalization in TestFileSys;
- cleans CI install/probe paths and seeds a known old-style staging directory whose marker must remain unchanged, while confirming no newly generated staging directory remains after migration.

Next:
1. Re-check refs before writing. Only advance `work/arabic-engine-support` from the recorded expected HEAD; keep `main` unchanged.
2. Review the complete code/workflow/docs diff; run YAML/source/static preflight.
3. Run one full CI. Verify 15/15 TestFileSys on Linux and Windows; the Windows package must migrate all synthetic AppData files, preserve the stale-stage marker, leave no PID staging folder, keep AppData hashes unchanged, pass the existing-profile/no-merge case, and upload the Windows artifact.
4. On failure, inspect exact log/error code; don't weaken assertions or delete unknown data.
5. If CI succeeds, provide the Windows ZIP and continue Phase 4 real-device acceptance. Keep font/UI work deferred until portability is accepted on the actual Windows machine.

## Handoff after Run #114 — isolate missing root-level config copy

Run #114 (43289640898; tested HEAD 334f6fba00c9a093b2e5511a2987f45aab91296e) passed the full Linux job, including TestArabicTextLayout 17/17, TestFileSys 15/15, package and upload. Linux Artifact 13976243943 (22,728,834 bytes; SHA-256 5966c9a2f7be474973eaa12881970ae71778b76a8aee77b6549f8d933a5ef1bb) exists. Windows configure/build and both C++ suites passed (17/17 and 15/15). Windows --worldlist path ran twice and printed the world inside the portable profile. The smoke test then failed because Minetest/minetest.conf was absent. No Windows Artifact was uploaded.

Current evidence says world discovery is using the portable path, but the complete root profile copy remains unverified. The workflow did not emit a full source/target file inventory, so do not infer the cause of the missing config yet.

The current patch adds:
- testCopyDir() as TestFileSys case 16, verifying a root minetest.conf and a nested world.mt are copied with exact contents under paths containing spaces;
- the expected TestFileSys summary 16/16 on both CI hosts;
- a preflight assertion that the synthetic AppData config exists before process launch;
- missing-file diagnostics that print the missing list and full source/destination inventories.

Next:
1. Re-fetch live refs and preserve the new HEAD; do not touch main.
2. Commit the test/workflow/doc updates atomically.
3. Run one complete CI. Check Linux and Windows build, Arabic 17/17, FileSys 16/16, then Windows package migration, stale staging, AppData hashes, no-merge and artifact upload.
4. If the new diagnostic says minetest.conf was seeded but not copied while root-level CopyDir test passes, trace the path/fixture used by the migration function before changing production code.
5. Continue Phase 3 until the Windows artifact uploads; then ask for real-device Phase 4 acceptance. Keep font/UI work deferred.

# رسالة الاستئناف الرسمية — Mineclonia-Arabic

## مرجع GitHub عند آخر تدقيق — 2026-11-12

- Repository: majd7772233-a11y/Mineclonia-Arabic
- Branch الوحيد: work/arabic-engine-support
- آخر commit تنفيذ: 69c62784baf96e3456239d5b954055be5e855501 — Repair Arabic engine CI workflow and Lua validator.
- main: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d — بقي دون تعديل.
- هذا التحديث يوثق Run #109؛ اقرأ HEAD الحالي من GitHub قبل أي عمل جديد ولا تفترض أن SHA لم يتغير.
- لا تعديل على main، لا فرع جديد، لا merge، لا Force Push.

## Run #109 — SUCCESS

- Run ID: 43212062926; run number 109; HEAD SHA 69c62784baf96e3456239d5b954055be5e855501.
- بدأ 2026-11-12T11:46:47Z، اكتمل 2026-11-12T11:57:15Z.
- Workflow sanity — Job 131161916735: success.
- Catalog/source validation — Job 131161957086: success.
- Linux x64 build/tests/package — Job 131162246258: success.
- Windows x64 MSVC build/tests/package — Job 131162246670: success.

### Validation evidence
- builtin: 245/245 keys، 0 empty، source/bundle byte-identical.
- gettext: 1,575/1,575 effective، 0 untranslated، 0 fuzzy، LANG_CODE=ar.
- msgfmt check-format: success.
- Lua syntax: 210 builtin Lua files parsed across both trees.
- C++ source validator: 17 declarations / 17 registrations / 17 definitions.
- vcpkg cache restore succeeded on both platforms; cache save skipped on both because cache hit.
- CMake configure and client builds succeeded on both.
- TestArabicTextLayout: exact zero-failure 17-test gate passed on both.
- Linux package: executable, matching builtin, Noto font/license, ldd runtime resolution checked.
- Windows package: executable, source/package builtin SHA match, Arabic comma check, Noto font/license, Raqm/HarfBuzz/FriBidi/OpenAL/Ogg/Vorbis DLLs checked.

### Artifacts
Linux:
- ID 13941099252; size 22,718,197 bytes.
- SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941099252

Windows:
- ID 13941743588; size 20,672,848 bytes.
- SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.
- https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941743588

Run: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926

## قواعد العمل الإلزامية

1. اقرأ branch HEAD وmain من GitHub مباشرة قبل أي كتابة.
2. اعمل حصراً على work/arabic-engine-support؛ استخدم expected_sha عند تحريك المرجع.
3. كل تعديل كود/Workflow/CMake/Lua/Python/Asset يتطلب تحديث Markdown المناسب في commit نفسه، على الأقل PROJECT_STATE وWORKLOG append-only وHANDOFF وSUPPORT وPROGRESS وNEXT_SESSION_PROMPT؛ README وBUILD_TESTING/ROADMAP حسب الحاجة.
4. لا تخفف assertions ولا تحذف اختبارات.
5. لا تدفع أثناء CI؛ cancel-in-progress=true قد يلغي التشغيل.
6. احفظ cache restore/save على المنصتين.
7. عند فشل جديد، اقرأ log الأصلي والكود المحيط قبل تغيير السبب.

## الخطوة التالية

1. المستخدم يختبر Windows artifact أعلاه كحزمة كاملة، لا exe منفرداً فوق تثبيت قديم.
2. تحقق من فتح القائمة الرئيسية ثم Settings > Language واختيار العربية.
3. اطلب/افحص log حديثاً للتحقق من Lua parser errors وSHA warnings؛ لم يُثبت اختفاؤها على جهاز المستخدم بعد.
4. تابع clipboard copy/cut/paste round-trip، mouse drag، Shift selection، Ctrl+Left/Right وCtrl+Backspace/Delete للنص المختلط.
5. تابع clipping/shadows/centering/alignment/scrolling/fallback/emoji/supplementary Unicode/combining marks.
6. Android يبقى مؤجلاً بعد Desktop.

لا تعلن اكتمال دعم العربية قبل إغلاق تحقق GUI والتفاعل الفعلي.


## Independent artifact audit after Run #109 — 2026-11-12

The two Run #109 GitHub Artifacts were downloaded and checked independently:
- Linux ID 13941099252; downloaded outer ZIP SHA-256 matches `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows ID 13941743588; downloaded outer ZIP SHA-256 matches `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.
- Both outer ZIP archives passed integrity checks and contain the expected package archive.

Linux extracted package:
- `bin/luanti --version` exited 0 and reported Luanti 5.17.0, Release, RUN_IN_PLACE=1.
- `ldd` had no missing dependencies.
- Arabic language Lua row, Arabic builtin translation, Noto Sans Arabic and OFL license are present.
- Running the Arabic module from the extracted package passed 17/17, exact zero-failure summary.

Windows extracted package:
- `bin/luanti.exe`, Raqm/HarfBuzz/FriBidi/FreeType DLLs, OpenAL/Ogg/Vorbis DLLs, Arabic language Lua row, Noto font and license are present.
- Windows binary was not executed here; do not claim Windows GUI/runtime success.

## Updated immediate next step

Run Windows artifact 13941743588 on the user's Windows machine as a complete extracted portable package. Open the main menu, then Settings > Language, select Arabic, and retain the new log to check for Lua parser errors and `No SHA256 known for builtin file` warnings. Do not overwrite an older install with only the EXE. Then proceed to clipboard, drag/Shift selection, mixed-direction keyboard tests and rendering. Android remains deferred.

## Windows portable profile — active implementation batch

Current work starts from parent commit 01a3dbbb196da678448211f818f9f5ab0a99f8d0 on work/arabic-engine-support. Re-read the live branch and main refs before any writes. main was a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d at the pre-edit check and must remain untouched.

The intended Windows ZIP layout is:
- Read-only application files: package root.
- Writable profile: package/Minetest.
- Worlds: package/Minetest/worlds.
- Downloaded games: package/Minetest/games.
- Config, logs, cache, content/profile data derived from path_user: inside package/Minetest.

The current source batch adds:
- getPortableUserPath() in porting.h/porting.cpp.
- Windows RUN_IN_PLACE path_user separation and a copy-first migration from %APPDATA%/Minetest using a staging sibling; legacy data is retained, and an existing portable directory is never merged automatically.
- Windows config lookup isolation so it does not read minetest.conf above the portable profile.
- TestFileSys portable-path assertion; expected module total 14 tests.
- Linux/Windows TestFileSys workflow gates and a Windows package smoke test for synthetic AppData migration.
- Four phase plan in docs/ROADMAP.md.

This batch is not yet validated: no C++ build, TestFileSys execution, YAML/PowerShell parser, or CI result is recorded. Before commit, preflight all changed files, especially the PowerShell block in build-luanti-arabic-engine.yml. Then verify the branch and main heads again, commit all code and Markdown in one commit, and trigger one full workflow. Do not push an extra change while CI runs because cancel-in-progress=true.

After CI, inspect its complete Linux and Windows jobs. Only if the package smoke test succeeds should the Windows artifact be given to the user. User acceptance still must verify the actual directory, settings, downloaded games/worlds, logs, cache and absence of new AppData writes. Do not claim fully portable before the actual Windows test.

## Continuation after Run #110 — Windows package smoke-test refinement

Repository: majd7772233-a11y/Mineclonia-Arabic. Work only on work/arabic-engine-support. Never modify main, create branches, or force-push. Before writes, fetch the live refs and verify protected main remains unchanged.

Previous code HEAD: 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427.

Run #110 (ID 43231076763, same SHA) completed with overall failure:
- Workflow sanity and catalog validation passed.
- Linux Configure/Build passed; Arabic text-layout 17/17; TestFileSys 14/14; package checks and artifact upload passed. Artifact 13950892008, 22,717,628 bytes, SHA-256 ac9c301921e1443b0bd89383a1b2a0d58bbeb0def9e5a404a293ebdb12eaee1b.
- Windows Configure/Build passed; Arabic text-layout 17/17; TestFileSys 14/14; install and DLL checks passed.
- Windows package smoke failed during a duplicate TestFileSys invocation from the copied install directory. The job log contained no captured native output, so its underlying process reason is unknown. The preceding package `--worldlist=path` invocation returned success.
- Windows artifact was not uploaded.
- vcpkg cache restore passed on both hosts; save was skipped because cache restore was a hit.

The current workflow correction keeps the mandatory build-tree TestFileSys checks unchanged and removes only the duplicate TestFileSys invocation inside the package smoke test. It now runs `--worldlist=path` twice against the installed package and validates:
- world discovery beneath package/Minetest/worlds on both starts;
- migrated minetest.conf and cache marker contents;
- world.mt, game.conf, cache marker, and CACHEDIR.TAG presence under Minetest;
- the legacy AppData profile's file inventory remains unchanged.

This refinement and its Markdown updates are committed together in the follow-up code commit. Inspect branch HEAD directly rather than relying on a copied SHA in this prompt. After the push, inspect the new full workflow by exact HEAD SHA, all jobs/steps/logs and artifacts. Do not claim portability until Windows package smoke and upload both pass. A real Windows GUI acceptance test is still required.


## Continuation after Run #111 — Windows portable profile Phase 4 audit

Repository: `majd7772233-a11y/Mineclonia-Arabic`. Work only on `work/arabic-engine-support`; do not touch `main`, create branches, or force-push. Previous HEAD before the corrective batch: `bb3df102f129b2ef11b50f16911f292e20881f74`. Protected `main` was `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.

Run #111 (43236622672) passed catalog validation and the complete Linux build/test/package job. Linux: TestArabicTextLayout 17/17; TestFileSys 14/14; artifact 13953262107, 22,719,344 bytes, SHA-256 `86dd6ecab7085bcde58b03fbd2225e347bed4326e86c46753fbf28070bc6889d`. Windows configure/build, TestArabicTextLayout 17/17, TestFileSys 14/14, install and DLL checks all passed. The Windows package smoke failed because the script demanded captured stdout from `--worldlist=path`; the process returned 0 but the GUI client is linked with `/SUBSYSTEM:WINDOWS`. The test stopped before inspecting the profile; migration success/failure was not established. No Windows artifact was uploaded.

The corrective batch:
- Adds a 15th TestFileSys case that points `porting::path_user` at a temporary profile path containing spaces, writes a fixture world and game, and checks `getAvailableWorlds()` and `getAvailableGameIds()`.
- Removes the invalid stdout-content assertion. Package verification checks exit codes and exact files/contents under package/Minetest, the CACHEDIR.TAG signature and location, AppData file hashes, and staging cleanup.
- Uses package paths containing spaces.
- Tests an already-existing portable profile and verifies no legacy AppData merge/overwrite.

Inspect the patch and run static checks before writing. Re-check both refs just before commit, then push one atomic source/workflow + Markdown commit using expected_sha. Run one full CI and inspect every step and artifact. Do not claim migration success until Windows smoke and artifact upload pass. Phase 4 still requires the user to test the actual full ZIP on Windows: ContentDB downloads, new worlds, config/log/screenshot/cache data, no new AppData writes, a removable/non-system drive, and existing-profile upgrade behavior. Keep font/UI work deferred until this is complete.


## Continuation after Run #112 — fix the Luanti CLI option invocation

Repository: `majd7772233-a11y/Mineclonia-Arabic`; work only on `work/arabic-engine-support`. Do not edit `main`, create branches, or force-push. Run #112 tested HEAD `cc9ff808542bafd0d5849203684798d74eaf2fcf`; protected main is `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.

Run #112 (ID 43245244962) passed catalog validation and the Linux build/test/package/upload. Linux TestArabicTextLayout 17/17, TestFileSys 15/15; artifact 13957061595, 22,727,482 bytes, SHA-256 `660232c9ddeae37fabe2e9a2264be3fa0d0493145b96c8da8cfd76a0306a756f`. Windows configure/build passed; TestArabicTextLayout 17/17 and TestFileSys 15/15 also passed. Windows package smoke failed because the workflow used `--worldlist=path`; the client printed `Unknown command-line parameter "--worldlist=path"`. Its help output shows `--worldlist <value>`, so the option and value must be separate arguments. Parser rejection occurred before portable-path initialization; no migration was attempted, and the missing `minetest.conf` assertion was expected. No Windows artifact was uploaded.

The current prepared correction changes all three calls to:
- `& $probeExe --worldlist path` (first startup)
- `& $probeExe --worldlist path` (second startup)
- `& $existingProbeExe --worldlist path` (existing portable profile case)

The output-capture assertion remains removed because the Windows client uses `/SUBSYSTEM:WINDOWS`. Keep on-disk content/hash checks, source AppData immutability, cache tag location/signature, staging cleanup, path-with-spaces, second launch and no-merge assertions.

After committing the workflow syntax correction with all Run #112 docs, run exactly one complete CI. Inspect Linux and Windows, require 15/15 FileSys on both, and wait for Windows smoke to pass before claiming portability. Then inspect the Windows artifact's ID/size/digest. Phase 4 still requires actual Windows use: ContentDB downloads, world creation, config/log/screenshot/texture/cache writes, no further AppData writes, and a removable/non-system drive. Do not start font/UI work before those tests pass.


## Continuation after Run #113 — normalize the package path and preserve stale staging

Repository: `majd7772233-a11y/Mineclonia-Arabic`. Work only on `work/arabic-engine-support`; never modify `main`, create branches, or force-push. Starting HEAD for the current corrective batch: `e0b1fc5bf517f216523f5a53d3d6b049673432d0`. Protected main: `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.

Run #113 (43247942695) passed source/catalog validation, Linux configure/build, TestArabicTextLayout 17/17, TestFileSys 15/15, package install/upload. Linux Artifact 13957134160, 22,723,769 bytes, SHA-256 `9b4178bba4ac2e284e51b85b170a3311b5d8f4adb753cd9323be6f67eb290ee6`. Windows configure/build and both test suites passed (17/17 and 15/15). Windows package smoke reached path initialization with valid `--worldlist path`, but the log reported an existing fixed staging path `...in\..\Minetest.migration-tmp`; it couldn't activate the profile, then the config-file assertion failed. The log doesn't prove where the old staging path came from. Windows artifact upload skipped.

Current corrective blobs prepared (not committed yet):
- porting.cpp: `7aa3e0dd0a6ea664347b5e1e45db2937119e9a6d`
- porting.h: `e5a9fd6cd97fc39a50860ec365861fe053162623`
- TestFileSys: `9f3d2a90b8af8d09bc67edb34eb75c12c1749de4`
- Windows workflow: `35ac21e00700e3b05deb2b2bb325af83b201745b`

Changes included:
- normalize existing `path_share` with `fs::AbsolutePath()` when computing `path_user`, removing `bin/..` components on Windows; preserve the original share path as fallback if canonicalization fails;
- use a process-specific sibling staging path and numeric suffix on collision, preserve unknown old staging directories, add rename errno diagnostics;
- add a TestFileSys `bin/..` normalization regression;
- clean install/probe dirs in the Windows workflow, seed a stale `Minetest.migration-tmp` marker, verify it is preserved, and verify no PID staging folder remains after migration.

Re-fetch branch/main refs immediately before committing. Use base tree from the live `e0b1fc5...` commit, create one source/workflow+docs commit parented to `e0b1fc5...`, and update only the working branch with expected_sha. Then run one full CI and inspect every job and artifact. Do not claim portable migration succeeds until Windows package checks and artifact upload pass. Real-device Phase 4 acceptance remains separate.

## Continuation after Run #114 — diagnose incomplete root-level profile copy

Repository: majd7772233-a11y/Mineclonia-Arabic. Work only on work/arabic-engine-support; don't edit main, create branches, or force-push. Current HEAD at this diagnosis: 334f6fba00c9a093b2e5511a2987f45aab91296e. Protected main remains a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d.

Run #114 (43289640898) passed source/catalog validation, Linux configure/build, TestArabicTextLayout 17/17, TestFileSys 15/15, package install/upload. Linux Artifact 13976243943, 22,728,834 bytes, SHA-256 5966c9a2f7be474973eaa12881970ae71778b76a8aee77b6549f8d933a5ef1bb. Windows configure/build and both suites passed (Arabic 17/17, FileSys 15/15). The package command with valid --worldlist path ran twice and printed the synthetic world beneath the portable Minetest folder. The next assertion failed because Minetest/minetest.conf was absent; Windows Artifact upload was skipped.

The current corrective blobs:
- TestFileSys adds testCopyDir(), checking a root minetest.conf and a nested world.mt under a path containing spaces, copied by fs::CopyDir() with exact contents.
- TestFileSys expected summary is 16/16 on both platforms.
- The Windows smoke asserts the synthetic source config exists before launch, prints the seeded source inventory, and reports the missing paths plus complete source and portable destination inventories on a failure.

Before committing, re-fetch live branch/main refs and ensure the branch remains at the expected HEAD. Commit source/test, workflow, and docs together. Run one full CI and inspect all logs. If CopyDir's direct test passes but the workflow still misses the config, inspect the inventories and trace which source path the migration actually reads before modifying production migration logic. Continue Phase 3 until Windows smoke and Windows artifact upload pass. Phase 4 real-user checks remain pending; do not start font/UI work.


## Continuation after Run #115 — trace incomplete Windows profile copy

Repository: `majd7772233-a11y/Mineclonia-Arabic`. Work only on `work/arabic-engine-support`; do not change `main`, create another branch, or force-push. Current starting HEAD: `dab127e225c2083f37255ed4d829d207506aad4d`. Protected `main`: `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.

Run #115 (43297133596) passed source/catalog validation, Linux/Windows configure/build, Arabic tests 17/17 on both hosts, and TestFileSys 16/16 on both hosts. Linux package and upload succeeded: Artifact 13979992497, 22,730,451 bytes, SHA-256 `6f00827ab6f3de6dca56f195e857881116993271838d03c279d6ec4b0685c579`. Windows package smoke failed after two valid `--worldlist path` invocations printed the world path under the portable profile. The synthetic legacy AppData source contained four verified files; the subsequent file assertion found all five required portable files missing and a zero-file destination inventory. Windows artifact upload was skipped.

Current batch changes:
- The 16-case `testCopyDir()` now pre-creates its target root before invoking `fs::CopyDir()`, matching the migration's pre-created staging directory.
- First/second and existing-profile package invocations use `--trace` and capture combined output. `setup_log_params()` handles `--trace` before path initialization, so this should reveal info/verbose/trace events from path selection, migration skip/copy, each CopyFileEx call and activation/rename.
- Missing-file diagnostics list PowerShell files, .NET files and directories separately.
- No production migration implementation is changed unless the next trace establishes the exact failure.

Before committing, re-check current branch/main refs and use the fresh live base tree; commit C++ test, workflow and all Markdown updates atomically. Run one full CI, then inspect the complete Windows trace. Require 16/16 FileSys on both platforms, successful first-run migration, second launch, AppData hash preservation, stale-staging marker preservation, existing-profile/no-merge, and Windows artifact upload. Phase 3 remains open until these pass. Phase 4 real Windows GUI acceptance is still pending. Keep font/UI work deferred.


## Continue from Run #116 — begin real Windows acceptance on the user's machine

Repository: `majd7772233-a11y/Mineclonia-Arabic`. Branch: `work/arabic-engine-support`. Tested source HEAD: `b0c486db3a0960c7779c2f341198fe267356f61b`. Protected main: `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`. Do not change main, create branches, or force-push.

Run #116 (43376273868) is fully successful. Four jobs passed: scheduling, catalog validation, Windows x64, Linux x64. Both platforms passed configure/build, TestArabicTextLayout 17/17 and TestFileSys 16/16; both packages uploaded successfully. Windows first-run migration copied the four synthetic AppData files through CopyFileEx into a process-specific staging directory and activated `Minetest`. The second launch detected and reused it. Assertions passed for file contents, cache tag, AppData SHA-256 inventory unchanged, stale staging marker preservation, no process stage left behind, paths containing spaces, world/game discovery, and existing-profile/no-merge. Linux and Windows Artifacts: 14014987601 and 14015073206 respectively.

Windows artifact wrapper: 14015073206, 20,682,182 bytes, SHA-256 `66e58c0a28de9b99215fb348a70f80a77a7f186875b6b97646119e348bf32ecc`. The actual inner portable ZIP is `luanti-arabic-windows-x64-portable.zip`, 20,792,245 bytes, SHA-256 `b14c92260b6c9a315f63594d71c5008df24a95a229f8bf4d6b7bd33d65bd146f`. This ZIP was integrity-tested and contains 533 entries including `bin/luanti.exe`, builtin, Noto Sans Arabic and its license.

**Portable four-phase automation is complete. Phase 4 real-device GUI acceptance is pending.** Continue by having the user extract the full ZIP into a new empty folder, preferably a path with spaces and outside their existing installation. Run `bin/luanti.exe`. Do not remove the old `%APPDATA%/Minetest` source; verify the copy appears in package-root `Minetest` and the source remains intact. Test Content tab download, create a world, save config, inspect logs/screenshots/texture packs/cache, ensure no new AppData writes, and test a non-system/removable drive if possible. Ask for screenshots/logs/results and diagnose actual failures. Keep font/UI and clipboard/text interaction work deferred until this acceptance is complete.


## Resume after Run #117 — restore and revalidate the known-good portable baseline

Repository: `majd7772233-a11y/Mineclonia-Arabic`. Only work on `work/arabic-engine-support`; leave `main` at `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`. Starting correction parent: `efbf5dc3b1d73b99df3ec1a14a30666540d729ca`.

Run #116 (43376273868) passed both platforms and all package checks on source HEAD `b0c486db3a0960c7779c2f341198fe267356f61b`. Run #117 (43382276907) was then triggered by a newer commit and failed only because its Windows smoke test again matched captured stdout from a Windows GUI-subsystem executable. Windows build, Arabic 17/17 and then-current FileSys 14/14 passed; the package test stopped at the empty stdout assertion before disk checks. Linux passed and uploaded Artifact 14017277623. No Windows artifact was uploaded in #117.

A diff against the successful Run #116 source revealed regressions introduced after it: `fs::AbsolutePath()` normalization was removed from `getPortableUserPath`, process-specific/collision-safe staging reverted to a fixed name, FileSys lost its path-normalization and CopyDir tests (dropping from 16 to 14), and the workflow reverted from trace/filesystem checks to stdout parsing.

The current corrective commit restores these exact four files from the Run #116 tested tree:
- `luanti/code/src/porting.cpp`
- `luanti/code/src/porting.h`
- `luanti/code/src/unittest/test_filesys.cpp`
- `.github/workflows/build-luanti-arabic-engine.yml`

Next step: inspect this commit's diff and refs, run one full CI (Run #118), and inspect all jobs. Required: Linux and Windows build; TestArabicTextLayout 17/17; TestFileSys 16/16; Windows `--trace --worldlist path` migration and file-content checks; AppData SHA-256 immutability; stale staging preservation; PID-stage cleanup; existing-profile/no-merge; both artifact uploads. Do not infer migration failure from stdout. After a successful Run #118, deliver the Windows artifact and continue Phase 4 actual Windows 10 GUI acceptance. Keep fonts/UI deferred until the user's storage workflows have been tested.


## Continue after Run #118 — real Windows 10 acceptance is the next task

Repository: `majd7772233-a11y/Mineclonia-Arabic`. Work only on `work/arabic-engine-support`; do not touch main, create branches or force-push. Tested source/workflow commit: `8896f0f3eaa7ca19a97bd0cfcfcfac5a05bf7192`. Protected main remains `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.

Run #118 (43385926666) completed successfully. Scheduling and catalog validation passed. Linux Job 131754594968 and Windows Job 131754595207 passed configure/build, TestArabicTextLayout 17/17, TestFileSys 16/16, install/package and upload. The Windows trace verified four CopyFileEx migrations into a PID-specific staging path, portable-profile activation, second-launch reuse, exact content/cache-tag checks, AppData preservation, stale staging preservation, new-stage cleanup, paths with spaces, and existing-profile/no-merge.

Artifacts:
- Windows wrapper ID 14018204334, 20,682,129 bytes, SHA-256 `88277ca613a04ffa255bedbc538229a260f32b94bc5bcb3558cc2402d45f8571`.
- Inner portable ZIP `luanti-arabic-windows-x64.zip`, 20,792,171 bytes, SHA-256 `36062ddb228fa9fec9bb864036012d4ac6a93e75c7e50f031b26a462baeebb39`, ZIP integrity passed, 533 entries.
- Linux artifact 14018973226, 22,731,333 bytes, SHA-256 `9072268c9b3a87bb75a5a01bd0c09fd713733e637c2ad01e9a12575ce7f38c6f`.

Now perform the actual user-side Phase 4 acceptance:
1. Extract the full ZIP into a new empty directory, ideally a path with spaces and outside the existing installation. Keep the old `%APPDATA%/Minetest` intact.
2. Run `bin/luanti.exe` and confirm the adjacent package-root `Minetest` folder appears and contains the migrated profile.
3. Use Content tab to download a game, create a world, change/save a setting, and close normally.
4. Inspect `Minetest/worlds`, `games`, `minetest.conf`, logs/debug output, screenshots, texture packs, and cache. Confirm these new writes go under the package's `Minetest` folder, not AppData.
5. Verify the old AppData profile remains intact; test a non-system/removable drive if available.

Record screenshots, exact file paths, and any logs. Diagnose any actual runtime issue before starting font/UI improvements. Do not merge the branch into main without explicit instruction.

## Continuation — mixed Arabic/Latin baseline and clipping

Repository: `majd7772233-a11y/Mineclonia-Arabic`. Work only on `work/arabic-engine-support`. Never modify `main`, create branches, or force-push. Previous HEAD before this change: `bc446c40f7d4f37625052fed7dec4cbdc21421a1`. Protected main: `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`.

The user's Windows GUI runs Arabic but shows vertical mismatch: Arabic sits lower than English/digits and short labels/progress/inventory text are clipped at the bottom.

Diagnosis: the default UI font chain uses Arimo for Latin/digits and Noto Sans Arabic as its fallback. CGUITTFont previously positioned glyphs using each face's independent ascender; line height used only the primary face. This creates inconsistent baselines and can under-measure fallback descenders.

The source/doc batch:
- calculates one baseline from the maximum ascender across the full fallback chain;
- calculates line height from maximum face height plus the greatest ascender/deepest descender, with one pixel of clearance;
- initializes font metrics using FreeType width=0,height=size like the shaping/rasterization paths;
- uses symmetric rounding for signed Raqm y offsets;
- adds TestArabicTextLayout testMixedFontBaselineAndLineBox using Arimo, Noto Sans Arabic and "Luanti مرحبا 123";
- updates both CI summary gates to require 18 tests.

Review all files in the new commit. Run the source validator and one full Linux/Windows CI. Require configure/build and Arabic 18/18 on both systems; inspect package and artifact uploads. Do not claim the visual issue fixed until the user retests small labels, inventory components, loading/progress text and mixed Arabic/English/digit lines in a fresh Windows build. If a widget still clips, inspect its fixed element bounds and clipping owner before a further change.

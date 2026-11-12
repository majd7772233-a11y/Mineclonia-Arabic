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

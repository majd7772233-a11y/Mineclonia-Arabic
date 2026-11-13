# Arabic Engine Support — Work Log

This file is an append-only record of the work sessions and verified outcomes. Update it in the same commit as every code change.

## 2026-11-10 — Main-menu startup syntax failure

**Repository:** `majd7772233-a11y/Mineclonia-Arabic`  
**Working branch:** `work/arabic-engine-support`  
**Starting commit:** `5412b95bf891000b783aa1acff23709512d957e5`  
**Protected branch:** `main` (must remain unchanged)

### User-reported runtime failure

The Windows log identifies `builtin/common/settings/dlg_settings.lua:179` and reports `'}' expected (to close '{' at line 176) near 'be'`. The table of language option labels omitted the comma after the Arabic entry. The defect was in the build-source tree at `luanti/code/builtin/common/settings/dlg_settings.lua`. The parallel reference tree at `luanti/5.17.0/builtin/common/settings/dlg_settings.lua` already had the comma.

### Changes in this work batch

1. Correct the missing comma in the build-source Lua file.
2. Add `tools/validate_lua_syntax.py`, which parses all `.lua` files in both builtin trees.
3. Add a fast CI gate pinned to `luaparser==4.2.0` before long Linux/Windows compilation.
4. Add a C++ regression assertion for the generated builtin SHA manifest entries of `common/math.lua`, `common/vector.lua`, `common/settings/dlg_settings.lua`, and `mainmenu/init.lua`.
5. Update `docs/ARABIC_ENGINE_SUPPORT.md` and `PROGRESS.md`, and maintain this work log and `docs/ARABIC_ENGINE_HANDOFF.md`.

### SHA-256 startup warnings

The C++ source emits `No SHA256 known for builtin file` when a builtin path does not map to an entry in `g_builtin_file_sha256_map`. The implementation logs the warning and then continues loading the script; this is not the syntax error that stopped the menu. The new regression assertion checks that key entries exist in the CI-built executable. It does not prove that the user's currently installed executable and builtin directory are a matching pair. Do not silence the warning before checking that.

### Last verified native CI state before this batch

Run #101 — commit `5412b95bf891000b783aa1acff23709512d957e5`:

- Arabic catalog validation: passed.
- Windows: client build passed; Arabic module passed (15/15); portable package checks passed; artifact uploaded (ID `13716251277`).
- Linux: client build passed; Arabic module failed (4 failures / 15 tests); no Linux package.
- Failing Linux assertions: contextual Arabic shaping, direct HarfBuzz shaping, lam-alef ligature, and shaping-cluster caret boundary.
- Remaining 11 Linux Arabic tests passed.

The Linux shaping problem is independent of the Lua syntax error and remains open.

### Verification required after this batch

- Lua syntax validation must pass for both builtin trees.
- The C++ module must pass with the builtin SHA-manifest assertion included.
- The Windows GUI must be tested using the new artifact; verify the language selector opens.
- Check whether the SHA warnings remain with the matching new executable and builtin files.
- Linux Arabic shaping must still be investigated and fixed without weakening the failing assertions.


## 2026-11-10 — Normalize Windows builtin SHA-256 paths

**Starting commit:** 9bdd876eea9ab8087906b4d067d76fd587e84fc8  
**Run reviewed:** #102, ID 42774549330.

Run #102 passed the quick catalog/Lua validation. Windows compiled, passed all 16 Arabic tests, passed portable-package verification, and uploaded artifact 13742874657. Linux compiled but failed four of 16 Arabic shaping tests; the Linux package step did not run.

The Windows startup warnings were traced to a path-key mismatch. HashBuiltinFiles.cmake emits forward-slash keys, while fs::MakePathRelativeTo() returns Windows backslashes. The runtime now normalizes before looking up the SHA manifest, and the existing manifest test covers POSIX, Windows, and mixed separators. The warning is deliberately not suppressed.

This change awaits a fresh Windows CI result and real-client confirmation with a newly downloaded artifact. The Linux HarfBuzz/Raqm shaping failures remain open and their assertions remain unchanged.


## 2026-11-10 — Run #103 verified Windows artifact; expand Linux shaping diagnostics

**Starting commit:** `baa79571161b0558afb3185ee7bbab3f349a4c44`  
**Run reviewed:** #103, ID `42777977334`.

The reported startup syntax error was a missing comma after the Arabic language-label entry in `luanti/code/builtin/common/settings/dlg_settings.lua`. It was already fixed in commit `9bdd876eea9ab8087906b4d067d76fd587e84fc8`; Run #103's pinned Lua parser successfully parsed all files in both builtin trees. The Windows install log confirms the corrected file is included in the produced package.

The repeated Windows SHA warnings were a separate native-path separator mismatch. The current source normalizes backslashes to forward slashes before looking up generated builtin manifest keys. The Windows manifest regression test passed in Run #103's 16-test module. This validates the normalizer in CI but does not replace a real launch test with the newly generated package.

Run #103 results:
- Catalog and Lua validation: passed.
- Linux client build: passed; 4/16 Arabic shaping assertions failed, no Linux artifact.
- Windows client build: passed.
- Windows Arabic module: 16/16 passed.
- Windows package verification: passed.
- Windows artifact ID: `13745910497`, size 20,663,963 bytes, SHA-256 `e8a6e012915bed39de21d23cce6d8afb1639bf5744d86217512a06b5b9255ddb`.

The Linux shaping failures are reproducible in direct HarfBuzz as well as Raqm, so they are not merely a selection/caret mapping issue. The source-level diagnostic now reports the selected HarfBuzz plan shaper, `HB_SHAPER_LIST`, actual font path/family, nominal Beh glyph, Arabic GSUB Script/default LangSys features, and forced-OpenType versus default shaping results for contextual Beh and lam-alef. Existing glyph-count and cursor-boundary assertions remain unchanged.


## 2026-11-10 — Run #104 diagnostic compile correction

**Starting commit:** `8f65db053c48e78c7bfe208b3f8c15562b7706a8`  
**Run reviewed:** #104, ID `42781584482`.

The Lua/catalog validation and both CMake configurations passed. Linux failed while compiling the new diagnostic because HarfBuzz 14.3.1 forbids direct inclusion of `hb-shape-plan.h`; `hb.h` is the supported public umbrella and already declares the shape-plan functions. The direct internal-header include was removed.

A second issue was found during API review: the feature-tag query returns the total number of GSUB feature tags as its function result, while its in/out count parameter specifies output capacity and then the number copied. The diagnostic had mistakenly used the copied count from a zero-capacity query, so it printed an empty feature list. The revised code allocates from the returned total and fetches the actual tags.

Run #104's Windows build was still running at the time of this patch. The new commit supersedes that run via the workflow's branch concurrency policy. Run #105 must confirm the diagnostic builds on both platforms, record the selected HarfBuzz shaper/Arabic GSUB features, and determine whether forced OpenType shaping behaves differently. Linux shaping assertions remain unchanged.


## 2026-11-10 — Run #105 font analysis and regression-test correction

**Starting HEAD:** `9eda41ba6285afb6ef0891a4493ac9cf35b32d99`  
**Run reviewed:** #105, ID `42782985200`.

The user's Lua startup syntax issue was already fixed in the current source, and the Lua parser gate passed in Run #105. The user's log timestamp (2026-11-09 23:41) predates the matching Windows package produced by Run #105 (2026-11-10 08:39 UTC). The old installation is not evidence that the latest fixed package still fails.

The SHA-256 warnings are independent. Path normalization from native separators to the generated forward-slash manifest keys is already implemented and its Windows unit test passed in Run #105. The user still needs to test with the matching artifact to verify the warnings disappear in their installed environment.

Run #105 result:
- Linux: catalog/Lua validation, CMake configure and client compilation passed; Arabic test module failed 4/16; no Linux package.
- Windows: catalog/Lua validation, configure, build, 16/16 Arabic tests, package verification and upload passed.
- Windows artifact ID `13747836271`, SHA-256 `4a9a73c22c6c1f3d1b818472d22eaf0abf6549357b6cf26c43b0a336c4636890`.

A direct inspection of the exact bundled Noto Sans Arabic font demonstrated that the failed glyph-count assumptions were not portable font requirements:
- `بب` uses two contextual glyph forms and two separately drawn dot-below marks. Four glyphs are correct, not a failure.
- The font's GSUB does not define a plain `لا` ligature; it shapes the two characters as `uni0644.init.rlig` and `uni0627.fina.rlig`.
- The font does define a real Allah ligature: the six codepoints in `اللّٰه` are replaced by `uniFDF2`, a single glyph/cluster.

The revised tests preserve strong validation while matching the bundled font's actual OpenType tables. Contextual tests require exact initial/final base glyphs and both dot marks. Plain Lam-Alef tests preserve two legal cursor stops. A real `uniFDF2` ligature exercises indivisible cluster selection and mouse hit-testing. The Windows and Linux workflow guards now require an exact 17-test zero-failure summary, reducing ambiguity about whether the intended module actually ran.

This correction modifies tests, CI assertions, and handoff documentation only; no production shaping workaround or weakened runtime logic is introduced. Run #106 must validate compilation and all 17 tests on both systems before packaging and completion are claimed.


## 2026-11-12 — تدقيق رأس الفرع واستعادة نقطة الاستئناف

Repository: majd7772233-a11y/Mineclonia-Arabic
Branch: work/arabic-engine-support
HEAD البرمجي الذي تم تدقيقه: 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2
main الذي تم فحصه: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (لم يُعدّل).
أحدث run: #107، ID 42805300100.

### ما فُحص
- راجعت 13 commit أحدث من نقطة المحادثة القديمة 79df4158d5ccd78308fab653170b207e98c4ae93.
- راجعت انتقالات CMake/static linking وRaqm static closure وتشخيص HarfBuzz، تحليل خصائص الخط، إصلاح Lua القائمة، SHA path normalization، وحذف duplicate tests.
- Run106 logs على المنصتين: GCC/MSVC أبلغا عن duplicate definitions لـtestLamAlefPreservesSeparateClusters وtestArabicAllahLigatureCluster؛ فشل build قبل tests/packages.
- فحص نصي مستقل للمصدر الحالي: 17 declaration و17 TEST registration و17 definition دون duplicates أو اختلافات مجموعات. هذا ليس compilation proof.
- Run107 conclusion=failure، لكن jobs endpoint أعاد 0 jobs وartifacts endpoint فارغ. لا توجد job logs.

### العوائق
1. build-luanti-arabic-engine.yml: grep/Select-String غير مغلقين، orphan -Quiet))، package blocks مكررة، وwindows job ثانية.
2. validate_lua_syntax.py: double escaping في regex للأقواس المربعة؛ يجب اختباره على النص الصحيح.
3. لا يوجد CI حديث يثبت إزالة duplicate C++ methods.
4. لا يوجد Artifact من HEAD الحالي ولا تأكيد فعلي من المستخدم حول اختفاء SHA warnings.

### دفعة التوثيق
أُنشئت وثائق PROJECT_OVERVIEW وPROJECT_STATE وARCHITECTURE وFILE_MAP وBUILD_TESTING وROADMAP وDOCUMENTATION_RULES وNEXT_SESSION_PROMPT، وتم تحديث HANDOFF وSUPPORT وREADME وPROGRESS. هذه دفعة توثيق فقط؛ لم يُعدل كود أو workflow.

### نقطة الاستئناف
إصلاح workflow وLua validator regex في دفعة برمجية واحدة، تشغيل validators السريعة، ثم CI واحد شامل مع المحافظة على cache. انظر PROJECT_STATE.md وNEXT_SESSION_PROMPT.md.


## 2026-11-12 — Run #108 analysis and workflow/Lua validator repair

Starting HEAD: 1c5bab319be89eb2aeba40f6626c1333dd495a7a
Branch: work/arabic-engine-support
main checked: a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (unchanged).

### Run #108

- ID 43201985097, run #108, push event, head SHA 1c5bab319be89eb2aeba40f6626c1333dd495a7a, completed/failure at 2026-11-12T10:37:19Z.
- GitHub Jobs API returned total_count=0; Artifacts API returned total_count=0; commit check-runs returned total_count=0.
- Combined status returned pending with no statuses. No per-job logs exist in the API response, so the failure cannot be assigned to compilation, tests, or packaging.

### Source review

- The 748-line workflow contained malformed grep quoting, a truncated PowerShell Select-String condition, orphaned/mixed package fragments, and repeated Windows jobs.
- Both source and bundled dlg_settings.lua files already contain the Arabic option comma before be; no source Lua edit was needed.
- validate_lua_syntax.py had a double-escaped square-bracket regex. This batch reduces it to one regex escape and retains full-tree parsing and non-zero failure behavior.
- C++ test source was read directly: 17 declarations, 17 registrations, 17 definitions, no duplicate names.
- normalizeBuiltinFileSha256Path exists and is used before SHA manifest lookup in ScriptApiSecurity::safeLoadFile. It was not modified.

### Repair batch

- Rebuild the workflow with exactly workflow-sanity, catalog-validation, linux, and windows jobs.
- Preserve cancel-in-progress, vcpkg baseline, and cache restore/save in both platform jobs.
- Keep the strict 17-test zero-failure gates before packaging on both systems.
- Verify installed Lua builtin against source, font/license presence, Linux ldd resolution, and Windows runtime DLLs.
- Change only the Lua validator regex; do not change Lua source, C++ assertions, or production shaping code.
- Update PROJECT_STATE, HANDOFF, SUPPORT, BUILD_TESTING, ROADMAP, NEXT_SESSION_PROMPT, README, and PROGRESS in the same changeset.

### Preflight evidence and limitations

- YAML parsing with PyYAML: passed.
- bash -n for each Bash run block: passed.
- Workflow structure: four unique jobs; two cache restore and two cache save steps; cancel-in-progress retained.
- Arabic option regex: passed against the actual source-line form and rejects a missing comma.
- actionlint, PowerShell parser, and msgfmt were unavailable locally.
- Installing luaparser==4.2.0 failed due DNS resolution; full Lua parser remains to be run by pinned CI validation.
- No CMake configure, native build, C++ tests, package install/archive, Artifact upload, or GUI runtime test was performed locally.

### Next evidence required

Inspect the next run on the exact repair commit SHA. Record catalog/Lua/PO results, Linux and Windows configure/build/17-test/package results, Artifact IDs and digests. A workflow repair is not a build or test success.


## 2026-11-12 — Run #109 full Linux/Windows Desktop verification

**Starting implementation HEAD:** 69c62784baf96e3456239d5b954055be5e855501  
**Parent commit:** 1c5bab319be89eb2aeba40f6626c1333dd495a7a  
**Branch:** work/arabic-engine-support  
**Protected main checked:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (unchanged).  
**Run:** #109, ID 43212062926, push event, head SHA 69c62784baf96e3456239d5b954055be5e855501.  
**Completed:** 2026-11-12T11:57:15Z — completed/success.

### Jobs

- Workflow sanity — job 131161916735: success.
- Arabic catalog/source validation — job 131161957086: success.
- Linux x64 build/tests/package — job 131162246258: success.
- Windows x64 MSVC build/tests/package — job 131162246670: success.

### Catalog and validator evidence

- Builtin catalog: 245/245 keys, 0 empty; source and bundled files byte-identical.
- Engine gettext: 1,575/1,575 effective translations, 0 untranslated, 0 fuzzy; LANG_CODE='ar'.
- `msgfmt --check --check-format`: success.
- Pinned Lua parser processed 210 builtin Lua files across the two trees successfully.
- Arabic C++ source validator confirmed 17 declarations, 17 registrations, and 17 definitions exactly once.

### Linux native path

- vcpkg bootstrap, binary-cache restore, manifest install and CMake configure: success.
- vcpkg cache save was skipped because restore reported a cache hit; restore/save actions remain in the workflow.
- Full client build: success.
- TestArabicTextLayout: 17 PASS lines, 0 failures; exact summary `Module TestArabicTextLayout passed (0 failures / 17 tests)`.
- cmake --install: success.
- Executable and builtin checked; installed dlg_settings.lua byte-identical to source; Arabic comma check passed.
- Noto Sans Arabic font and OFL license present; `ldd` output contained no missing runtime dependency.
- Linux package uploaded successfully.

Artifact `luanti-arabic-linux-x64`: ID 13941099252, size 22,718,197 bytes, SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`. URL: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941099252

### Windows native path

- vcpkg bootstrap, binary-cache restore, manifest install and CMake configure: success.
- vcpkg cache save was skipped because restore reported a cache hit.
- MSVC client build: success.
- Arabic text-layout regression step: success. The step required the process exit code to be zero and used PowerShell `Select-String -SimpleMatch` to require the exact 17-test zero-failure summary before continuing.
- cmake --install: success.
- Packaged dlg_settings.lua SHA-256 matched source; Arabic option comma before `be` was checked.
- Executable, Noto Sans Arabic font/license, Raqm/HarfBuzz/FriBidi DLLs, and OpenAL/Ogg/Vorbis DLLs checked successfully.
- ZIP package upload completed.

Artifact `luanti-arabic-windows-x64`: ID 13941743588, size 20,672,848 bytes, SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`. URL: https://github.com/majd7772233-a11y/Mineclonia-Arabic/actions/runs/43212062926/artifacts/13941743588

### Remaining acceptance work

The successful CI establishes desktop compilation, the dedicated 17-test module on both platforms, and package contents/dependency checks. It does not prove that the user's real GUI starts correctly or that SHA warnings are absent on their machine. The user must launch the complete Windows artifact, open Settings > Language, and provide a new log if startup or SHA warnings remain. Clipboard round-trip, mouse drag, Shift selection, mixed-direction keyboard deletion/navigation, clipping/shadows/alignment/scrolling and broader Unicode rendering remain to be verified. Android remains deferred.

No assertions were weakened and no tests were deleted in the workflow repair. No code change is part of this follow-up documentation commit.


## 2026-11-12 — Independent verification of Run #109 downloadable artifacts

**Starting HEAD:** 596c9dd10e98f37057f87a92873d77a21ebc4a95 (documentation-only commit after Run #109).  
**Implementation HEAD represented by the artifacts:** 69c62784baf96e3456239d5b954055be5e855501.  
**Branch:** `work/arabic-engine-support`.  
**Run:** #109, ID 43212062926. No new CI was launched for this audit.

### Download and digest verification

Downloaded Linux artifact 13941099252 and Windows artifact 13941743588 from the GitHub Actions run. SHA-256 was calculated over the downloaded outer artifact ZIP files:
- Linux: `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999` — exact match with the digest printed by `actions/upload-artifact@v4`.
- Windows: `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6` — exact match with the digest printed by `actions/upload-artifact@v4`.

Both outer ZIPs passed the archive integrity check (`ZipFile.testzip() == None`) and contained the expected inner package archive.

### Linux package execution

The extracted Linux package contains an x86-64 ELF executable. `ldd` reported no `not found` dependencies. Running `./bin/luanti --version` from the package root exited 0 and reported Luanti 5.17.0, Release, RUN_IN_PLACE=1, USE_CURL=1, USE_GETTEXT=1, and USE_SOUND=1.

The extracted package contains the language selector builtin with `ar = "العربية [ar]",`, `builtin/locale/__builtin.ar.tr`, Noto Sans Arabic Regular (844,676 bytes), and the Noto OFL license (4,382 bytes).

The test suite was run directly from the extracted package:
`./bin/luanti --run-unittests --test-module TestArabicTextLayout`

Result: 17 PASS lines; exact summary `Module TestArabicTextLayout passed (0 failures / 17 tests)`; `Unit Test Results: PASSED`; `0 / 17 failed tests`.

### Windows package inspection

The extracted inner ZIP contains `bin/luanti.exe` (9,920,000 bytes), the expected Raqm/HarfBuzz/FriBidi/FreeType and OpenAL/Ogg/Vorbis DLLs, the language selector Lua file with the Arabic comma, Noto Sans Arabic Regular, and its OFL license.

This was a file/archive inspection only. The Windows executable was not run in this environment, and no Windows GUI/runtime success is claimed.

### Remaining next step

The Linux package has independent extracted-artifact execution evidence. Windows still needs a real launch on the user's machine using the whole extracted package, followed by Settings > Language and a fresh log inspection for Lua parser and SHA-256 warnings. Interactive EditBox and rendering checks remain open. Android remains deferred.

## 2026-11-12 — Windows portable user profile, legacy migration, and CI probe

**Starting HEAD:** 01a3dbbb196da678448211f818f9f5ab0a99f8d0  
**Branch:** work/arabic-engine-support  
**Protected main checked:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (unchanged before edits).  
**Reason:** User needs all writable game data under the portable Luanti package rather than %APPDATA%/Minetest.

### Changes prepared
- Windows RUN_IN_PLACE keeps path_share at the package root and derives path_user as path_share/Minetest.
- Added getPortableUserPath() and a TestFileSys assertion.
- Added first-launch copy of %APPDATA%/Minetest into a staging directory beneath the package; it activates only after a complete copy and retains the original profile.
- If the portable profile already exists, migration does not merge or overwrite it.
- path_cache remains derived from path_user, so cache/CACHEDIR.TAG is inside the portable profile.
- Windows RUN_IN_PLACE no longer searches for minetest.conf one or two levels above the portable profile.
- Added TestFileSys CI regression steps for Linux and Windows, with the expected summary 0 failures / 14 tests.
- Added a Windows integration smoke test using an isolated package clone and synthetic legacy profile. It checks discovery of a migrated world, migrated game/config/cache files, portable cache tag creation, and that the old AppData profile file set is unchanged.
- Updated Luanti's path table and four-phase project roadmap.

### Files
luanti/code/src/porting.h; luanti/code/src/porting.cpp; luanti/code/src/main.cpp; luanti/code/src/unittest/test_filesys.cpp; .github/workflows/build-luanti-arabic-engine.yml; luanti/code/README.md; docs/ROADMAP.md; docs/PROJECT_STATE.md; docs/ARABIC_ENGINE_WORKLOG.md; docs/ARABIC_ENGINE_HANDOFF.md; docs/ARABIC_ENGINE_SUPPORT.md; docs/NEXT_SESSION_PROMPT.md; docs/BUILD_TESTING.md; docs/ARCHITECTURE.md; docs/FILE_MAP.md; README.md; PROGRESS.md.

### Validation at authoring time
- GitHub source review completed for porting path initialization, user-path config lookup, filesystem copy helpers, TestFileSys, and the full Windows/Linux packaging workflow.
- C++ compilation and all runtime tests: not run yet.
- YAML/actionlint/PowerShell parser: not run yet in a local shell.
- Full CI: not started yet.
- No Artifact is produced by this documentation/source preparation step; wait for CI.
- The old AppData profile is copied, not deleted; any migration failure logs an error and leaves the original intact.

**Next:** run preflight, then commit this complete source+Markdown batch atomically and start one full CI run.

## 2026-11-12 — Run #110 result and Windows package smoke-test correction

**Code commit under test:** 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427  
**Run:** #110 — ID 43231076763 — HEAD SHA 3ca51d4b8ffeb8f06bec4a3a52aa761310d1c427  
**Overall:** failure (Linux job passed; Windows package verification step failed).

### Actual CI evidence
- Workflow sanity: passed.
- Catalog validation: passed (Arabic catalog, PO format, Lua syntax, C++ test-source validator).
- Linux: Configure passed; Build passed; TestArabicTextLayout passed (0 failures / 17 tests); TestFileSys passed (0 failures / 14 tests); install checks passed; Linux artifact uploaded.
- Linux artifact ID: 13950892008; 22,717,628 bytes; SHA-256 digest ac9c301921e1443b0bd89383a1b2a0d58bbeb0def9e5a404a293ebdb12eaee1b.
- Windows: Configure passed; Build passed; TestArabicTextLayout passed (0 failures / 17 tests); TestFileSys passed (0 failures / 14 tests).
- Windows install checks and runtime DLL checks passed. The package smoke test's first `--worldlist=path` process exited successfully. The subsequent duplicate TestFileSys invocation from the copied install package did not produce the expected summary and caused the PowerShell step to fail. The captured output was empty in the job log, so the underlying native-process reason is not established.
- Windows artifact upload was skipped; no Windows ZIP exists for Run #110.
- vcpkg cache restore succeeded on both platforms; save steps were skipped because restore returned a cache hit.

### Corrective action
The independent build-tree TestFileSys runs remain mandatory on Linux and Windows (both passed 14/14 in Run #110). The Windows install smoke test no longer repeats that suite inside the copied package. It now checks first and second `--worldlist=path` launches, verifies the migrated world remains under Minetest across launches, checks config/cache file contents plus world/game/cache files and CACHEDIR.TAG, and confirms the legacy AppData profile file inventory is unchanged.

**Next:** run one new full CI for this workflow correction. Treat Windows portability as unverified until its package smoke test and artifact upload succeed.


## 2026-11-12 — Run #111 smoke-test diagnosis and Phase 4 portability audit

**Starting HEAD:** bb3df102f129b2ef11b50f16911f292e20881f74  
**Branch:** work/arabic-engine-support  
**Protected main checked:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d  
**Run:** #111 — ID 43236622672 — implementation HEAD bb3df102f129b2ef11b50f16911f292e20881f74  
**Overall:** failure in the Windows package smoke assertion only.

### Actual CI evidence
- Workflow scheduling: success.
- Catalog/source validation: success.
- Linux: configure/build success; TestArabicTextLayout 17/17; TestFileSys 14/14; install/package and upload success.
- Linux artifact 13953262107: 22,719,344 bytes; SHA-256 `86dd6ecab7085bcde58b03fbd2225e347bed4326e86c46753fbf28070bc6889d`.
- Windows: configure/build success; TestArabicTextLayout 17/17; TestFileSys 14/14; install checks and required DLL checks success.
- Windows smoke test failed because it expected `--worldlist=path` output on captured stdout. The process returned exit code 0, while captured output was empty. Windows Release links the client with `/SUBSYSTEM:WINDOWS`; the GUI target cannot be treated as a console-output process in this CI invocation. The workflow threw before checking whether migration files existed, so Run #111 does not prove either success or failure of migration.
- Windows artifact upload was skipped; no Windows artifact was produced.
- vcpkg cache restore was successful; cache save was skipped on cache hit.

### Corrective implementation in this commit
- TestFileSys adds a direct profile-consumer regression test: with path_user set to a temporary directory containing spaces, it creates a world and a game and checks that `getAvailableWorlds()` and `getAvailableGameIds()` discover both there. TestFileSys is now expected to report 15 tests.
- The Windows package probe no longer treats stdout as evidence. It requires a zero process exit and checks the migrated profile, settings/world/game/cache contents, CACHEDIR.TAG signature/location, unchanged legacy AppData file hashes and no leftover staging folder.
- The migration package is copied under a path with spaces.
- A second isolated package starts with an existing Minetest profile and a legacy AppData profile. It checks that existing portable files are not overwritten, legacy-only content is not merged, and AppData remains intact.
- No production migration/path implementation is changed unless a later CI failure provides evidence requiring it.

### Validation status at commit preparation
- Code/source and workflow patch reviewed against the Run #111 logs and MSVC subsystem linker flags.
- C++ compile/tests, PowerShell parse, complete workflow YAML validation, and new full CI run are pending this commit.
- No Windows Artifact is available from Run #111.
- Manual testing on the user's Windows installation remains mandatory for the rest of Phase 4, especially ContentDB downloads, new worlds, settings/log/screenshot/cache writes, removable/non-system drive, and no AppData writes.


## 2026-11-12 — Run #112: corrected option syntax exposed by Windows smoke test

**Starting HEAD:** cc9ff808542bafd0d5849203684798d74eaf2fcf  
**Branch:** work/arabic-engine-support  
**Protected main:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d  
**Run:** #112 — ID 43245244962  
**Overall:** Linux job passed; Windows compile/tests passed; package smoke did not reach initialization.

### CI evidence
- Workflow scheduling and source/catalog validation: passed.
- Linux configure/build, TestArabicTextLayout 17/17, TestFileSys 15/15, package install and upload: passed.
- Linux artifact 13957061595: 22,727,482 bytes; SHA-256 `660232c9ddeae37fabe2e9a2264be3fa0d0493145b96c8da8cfd76a0306a756f`.
- Windows configure/build: passed; TestArabicTextLayout 17/17; TestFileSys 15/15.
- Windows package step failed at its first filesystem assertion. The log shows `Unknown command-line parameter "--worldlist=path"` (repeated for both launches); the help output lists `--worldlist <value>`. The test passed the option and its value as one equals-form argument, but the engine parser requires two arguments.
- Because argument parsing rejected the command before `initializePaths()`, the migration code did not run. The missing `Minetest/minetest.conf` is therefore expected with that invalid invocation; Run #112 does not establish a defect in profile migration.
- Windows artifact upload was skipped; no Windows package was produced.

### Corrective change now prepared
- Replace all three invocations with `& $exe --worldlist path`.
- Retain the filesystem/content assertions, legacy AppData hash comparison, staging cleanup, package path with spaces, second launch, and existing-profile/no-merge scenario.
- Keep stdout parsing removed; this executable uses `/SUBSYSTEM:WINDOWS`.
- Record the valid CLI form in the testing and architecture docs to prevent recurrence.

### Validation still pending
A new full CI run is required after this workflow/doc-only correction. Linux packaging should remain unchanged; the key gate is that Windows actually enters initialization, migrates the synthetic AppData profile, passes every on-disk assertion, and uploads the Windows artifact. Real-device Phase 4 acceptance remains separate.


## 2026-11-12 — Run #113 staging-path collision and portable-profile hardening

**Starting HEAD:** e0b1fc5bf517f216523f5a53d3d6b049673432d0  
**Branch:** work/arabic-engine-support  
**Protected main:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d  
**Run:** #113 — ID 43247942695  
**Overall:** Linux job passed; Windows compile/tests passed; Windows package smoke failed in profile staging/activation.

### Actual CI evidence
- Workflow scheduling and catalog/source validation: passed.
- Linux: configure/build passed; TestArabicTextLayout 17/17; TestFileSys 15/15; package install/upload passed.
- Linux Artifact 13957134160: 22,723,769 bytes; SHA-256 `9b4178bba4ac2e284e51b85b170a3311b5d8f4adb753cd9323be6f67eb290ee6`.
- Windows: configure/build passed; TestArabicTextLayout 17/17; TestFileSys 15/15.
- Windows smoke used the valid two-argument form `--worldlist path`, so option parsing proceeded into path initialization. The log then reported `Cannot migrate legacy profile because staging path already exists` at `...in\..\Minetest.migration-tmp`, followed by failure to activate the portable profile. The file check found no portable `minetest.conf`.
- This log establishes a collision at the fixed staging path and failed migration activation. It does **not** establish why the staging path existed before the migration function checked it; don't describe its origin as proven.
- Windows artifact upload was skipped. No Windows artifact was produced.

### Corrective code/test/workflow batch
- Normalize an existing share directory with `fs::AbsolutePath()` in `getPortableUserPath()`, which removes the installed executable's `bin/..` path components before forming the writable profile path. If normalization fails, preserve the input path as fallback.
- Use a process-specific staging sibling `.migration-tmp-<pid>`; add a numeric suffix if that name already exists. Preserve old staging folders. This avoids the old fixed-name collision without deleting unknown data.
- Report CRT `errno` and its text when staging activation via rename fails.
- Extend TestFileSys path contract to cover a real existing directory referenced through a nested `bin/..` path containing spaces.
- Windows CI deletes prior install/probe folders, checks that the installed package is free of profile/staging directories, creates an explicit legacy `Minetest.migration-tmp` folder with a preservation marker in the test copy, and checks that the marker remains while any newly generated process-specific stage is cleaned up. It also keeps the existing-profile/no-merge scenario.

### Validation pending
- No C++ configure/build or unit tests have run for this corrective batch yet.
- No PowerShell/YAML preflight or full CI run has run for this corrective batch yet.
- No Windows Artifact exists from Run #113.
- Real Windows GUI acceptance remains outstanding: ContentDB download, new world, settings/log/screenshot/cache paths, no new AppData writes, and removable/non-system drive.

## 2026-11-12 — Run #114: world migrated, root config assertion failed

**Starting HEAD:** 334f6fba00c9a093b2e5511a2987f45aab91296e  
**Branch:** work/arabic-engine-support  
**Protected main:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d  
**Run:** #114 — ID 43289640898  
**Overall:** Linux job passed; Windows build/tests passed; Windows package smoke failed at the portable minetest.conf presence check.

### Actual CI evidence
- Workflow scheduling and source/catalog validation: success.
- Linux configure/build: success; TestArabicTextLayout 17/17; TestFileSys 15/15; package install/upload success.
- Linux Artifact 13976243943: 22,728,834 bytes; SHA-256 5966c9a2f7be474973eaa12881970ae71778b76a8aee77b6549f8d933a5ef1bb.
- Windows configure/build: success; TestArabicTextLayout 17/17; TestFileSys 15/15.
- Windows package: valid --worldlist path invocation ran twice and printed the expected world path under the probe package's Minetest/worlds/portable-world-probe. The job then failed because Minetest/minetest.conf was absent.
- Windows Artifact upload was skipped; no Windows package is available.
- This proves initialization reached world discovery and the world was present under the portable profile. It does not prove all root-level profile files were copied. The current output does not show the source and destination directory inventories, so the missing config's exact cause remains unknown.

### Corrective test changes in the current batch
- Add testCopyDir() as TestFileSys case 16. It creates a source profile under a path containing spaces, writes a root-level minetest.conf and a nested worlds/world with spaces/world.mt, copies the directory with fs::CopyDir(), and verifies exact bytes/content for both files.
- Require TestFileSys 16/16 on Linux and Windows.
- Before launch, assert that the PowerShell fixture really contains its synthetic minetest.conf and print the seeded source file inventory.
- On package assertion failure, print all missing required paths and the complete source and destination inventories. Retain the other profile migration, cache, AppData hash, staging, and no-merge checks.

### Phase position
The work is still in **Phase 3 — Package regression coverage and documentation**. Phase 4 automation is being exercised as part of the Windows package test, but real-device acceptance is still outstanding. No production migration change is justified until the direct CopyDir test and next diagnostic run establish the failure cause.

## 2026-11-13 — Run #115 diagnostic and destination-precreated regression

**Starting HEAD:** dab127e225c2083f37255ed4d829d207506aad4d  
**Branch:** work/arabic-engine-support  
**Protected main:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d  
**Run:** #115 — ID 43297133596  
**Overall:** Linux job passed; Windows build/tests passed; Windows package smoke failed after world-list initialization.

### Actual CI evidence
- Scheduling and catalog/source validation: passed.
- Linux: configure/build passed; TestArabicTextLayout 17/17; TestFileSys 16/16; install/package/upload passed.
- Linux Artifact 13979992497: 22,730,451 bytes; SHA-256 `6f00827ab6f3de6dca56f195e857881116993271838d03c279d6ec4b0685c579`.
- Windows: configure/build passed; TestArabicTextLayout 17/17; TestFileSys 16/16.
- The synthetic AppData source profile was confirmed to contain `minetest.conf`, `worlds/portable-world-probe/world.mt`, `games/portable-game-probe/game.conf`, and `cache/legacy-cache.marker`.
- Both `--worldlist path` launches printed the world path under the probe package's portable `Minetest/worlds` directory.
- The later PowerShell checks reported all required files absent; the recursively enumerated portable file list was empty. The old AppData source remained available. No Windows artifact was uploaded.
- Run #115 did not use `--trace`, so the job did not expose the full initialization/copy logs needed to reconcile the world listing and empty file inventory.

### Corrective changes in this commit
- The direct `testCopyDir()` case now pre-creates its destination directory before calling `fs::CopyDir()`, matching the migration function, which creates a process-specific staging directory before copying into it.
- Each Windows package startup probe runs with `--trace` and captures combined output. Because `setup_log_params()` registers info, verbose and trace levels to stderr before `porting::initializePaths()`, this should expose the selected `path_share/path_user`, whether migration skips, every successful CopyFileEx operation, and activation/rename errors.
- Missing-file diagnostics print PowerShell files, .NET files and destination directories separately to avoid conflating provider enumeration with actual filesystem state.
- No production migration implementation is changed in this batch; the new evidence must identify whether the defect lies in the helper, source selection, destination/staging handling, or post-copy activation before production code is changed.

### Validation boundary
The new source/workflow changes still require a new complete CI run. Require 16/16 FileSys on both platforms, inspect every trace line in the Windows smoke, pass first-run/second-run and existing-profile/no-merge assertions, and upload the Windows artifact. Phase 3 remains open. Real-device Phase 4 and UI/font work remain deferred.


## 2026-11-13 — Run #116: complete portable-profile CI acceptance

**Starting/tested HEAD:** b0c486db3a0960c7779c2f341198fe267356f61b  
**Branch:** work/arabic-engine-support  
**Protected main:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (unchanged)  
**Run:** #116 — 43376273868 — completed / success  
**Purpose:** close Windows first-launch migration, staging recovery and package regression gates, then begin real-device Phase 4.

### Actual CI evidence
All four jobs succeeded:
- Scheduling — Job 131723388273.
- Catalog/source validation — Job 131723410331.
- Windows x64 MSVC — Job 131723573694.
- Linux x64 — Job 131723573700.

Both platforms configured and built successfully. Both passed TestArabicTextLayout 17/17 and TestFileSys 16/16. Install/package checks and both artifact uploads passed.

### Windows trace and integration results
The first package launch used a package path containing spaces and a synthetic AppData profile. The trace recorded successful CopyFileEx operations for:
- cache/legacy-cache.marker
- games/portable-game-probe/game.conf
- minetest.conf
- worlds/portable-world-probe/world.mt

All four files were copied under a process-specific staging path, and the engine logged successful activation of the portable `Minetest` directory while retaining the AppData source. A second launch logged that the portable profile already existed and that AppData would not be merged.

The Windows smoke assertions passed:
- exact contents of migrated config, world, game and cache files;
- CACHEDIR.TAG signature and portable cache location;
- AppData file inventory and SHA-256 hashes unchanged;
- stale legacy staging marker preserved;
- no process-specific staging directory left behind;
- package copied/launched under a path with spaces;
- pre-existing portable profile unchanged, with legacy-only AppData files not merged and the old source retained.
The trace showed world discovery beneath the portable Minetest/worlds folder on both first and second launch.

### Artifacts
- Windows: Artifact 14015073206, 20,682,182 bytes, SHA-256 `66e58c0a28de9b99215fb348a70f80a77a7f186875b6b97646119e348bf32ecc`.
- Linux: Artifact 14014987601, 22,731,184 bytes, SHA-256 `d9ee80d7af0a291bc83479870598634b530f68b301818c903efdd83e6c7428d9`.
- The GitHub Windows artifact wrapper was downloaded and validated. Its inner portable ZIP is `luanti-arabic-windows-x64.zip`, 20,792,245 bytes, SHA-256 `b14c92260b6c9a315f63594d71c5008df24a95a229f8bf4d6b7bd33d65bd146f`; ZIP integrity passed with 533 entries, including bin/luanti.exe, builtin, Noto Sans Arabic and its OFL license.

### Four-phase portable implementation status
- Phase 1 — application/share files separated from writable Minetest profile: complete.
- Phase 2 — first-launch AppData copy, cache/config isolation, no automatic merge: complete.
- Phase 3 — C++ and packaged regression coverage, documentation, Linux/Windows artifact evidence: complete.
- Phase 4 automated portion — path-with-spaces, repeat launch, stale staging, AppData immutability, existing-profile/no-merge and world/game discovery: complete.

### Not yet user-tested
Run #116 is CI/package evidence, not GUI usage on the user's laptop. The complete Windows ZIP still needs local verification: opening the application and menu, Content tab download, creating a world, config/log/screenshot/texture/cache output paths, no new AppData writes, and launch from a non-system/removable drive if available. Do not claim real-world portability until these steps are confirmed. Font/UI and later Arabic interaction checks remain subsequent work.



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


### سجل الإصلاح بعد Run #117
أُعيدت الملفات الأربعة إلى نسخها المطابقة للرأس الذي اختبره Run #116 بنجاح:
- `luanti/code/src/porting.cpp`: استعادة تطبيع share path وتسمية staging الخاصة بالعملية مع معالجة التصادم.
- `luanti/code/src/porting.h`: استعادة واجهة/تعليق النسخة المختبرة.
- `luanti/code/src/unittest/test_filesys.cpp`: استعادة 16 حالة، ومنها CopyDir المباشر مع ملف root وملف nested ومسارات ذات مسافات.
- `.github/workflows/build-luanti-arabic-engine.yml`: استعادة --trace وملفات القرص وAppData hashes وstale-stage/no-merge بدلاً من الاعتماد على stdout.

هذا إصلاح استرجاعي لنسخة مثبت نجاحها في Run #116. يجب إعادة بناء واختبار المنصتين في Run #118 قبل اعتبار HEAD الحالي مقبولاً.


## 2026-11-13 — Run #118: restored Windows portable profile acceptance

**Starting HEAD:** efbf5dc3b1d73b99df3ec1a14a30666540d729ca  
**Tested commit:** 8896f0f3eaa7ca19a97bd0cfcfcfac5a05bf7192  
**Branch:** work/arabic-engine-support  
**Protected main:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d (unchanged)  
**Run:** #118 — 43385926666 — completed / success.

### What was restored
The four code/workflow files were restored to their exact Run #116-tested versions:
- normalized `getPortableUserPath()` using `fs::AbsolutePath()`;
- process-specific staging with collision suffixes, retaining any stale fixed-name directory;
- TestFileSys 16 cases, including path normalization and direct CopyDir root/nested-file tests;
- Windows smoke uses `--trace --worldlist path` and filesystem assertions, never a stdout match from the GUI executable.

### Actual CI results
All four Jobs passed:
- Scheduling 131754268500.
- Catalog/source validation 131754303790.
- Windows x64 131754595207.
- Linux x64 131754594968.

Both platforms passed configure/build, TestArabicTextLayout 17/17, TestFileSys 16/16, package installation and upload.

### Windows integration trace
The synthetic AppData profile was verified to contain all four files. CopyFileEx traces confirm each was copied to `Minetest.migration-tmp-3616`; activation then logged the package-root portable `Minetest` folder. The second launch logged that the portable profile already existed and the old profile would not be merged. The smoke completed exact file/content checks, cache tag, AppData preservation, stale staging marker, PID-stage cleanup, path-with-spaces, and existing-profile/no-merge assertions.

### Artifacts
- Windows 14018204334 — 20,682,129 bytes — SHA-256 88277ca613a04ffa255bedbc538229a260f32b94bc5bcb3558cc2402d45f8571.
- Inner Windows ZIP — 20,792,171 bytes — SHA-256 36062ddb228fa9fec9bb864036012d4ac6a93e75c7e50f031b26a462baeebb39 — ZIP integrity passed, 533 entries.
- Linux 14018973226 — 22,731,333 bytes — SHA-256 9072268c9b3a87bb75a5a01bd0c09fd713733e637c2ad01e9a12575ce7f38c6f.

### Remaining work
Automated phases are complete. Await the actual Windows 10 GUI/data-storage acceptance from the user before any font/UI changes. The ZIP should be extracted to a new empty folder; do not delete the AppData source profile; check Content tab download, world creation, config, logs, screenshots, texture packs/cache, AppData immutability, and a non-system drive if possible.

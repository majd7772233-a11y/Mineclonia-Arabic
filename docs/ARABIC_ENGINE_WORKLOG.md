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

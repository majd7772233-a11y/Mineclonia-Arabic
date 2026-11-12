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

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

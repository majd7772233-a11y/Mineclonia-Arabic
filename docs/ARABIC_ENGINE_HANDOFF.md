# Arabic Engine Support — Continuation Handoff

**Last updated:** 2026-11-10  
**Repository:** majd7772233-a11y/Mineclonia-Arabic  
**Active branch:** work/arabic-engine-support  
**Starting HEAD for this follow-up:** baa79571161b0558afb3185ee7bbab3f349a4c44  
**Protected main HEAD observed:** a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d

## Safety and workflow rules

- Make changes only on work/arabic-engine-support.
- Never modify, reset, merge, or push to main.
- Do not create additional branches.
- Preserve vcpkg binary-cache restore/save steps in the build workflow.
- Prefer a thoroughly reviewed coherent batch before starting the expensive native workflow.
- Update PROGRESS.md, docs/ARABIC_ENGINE_SUPPORT.md, this handoff, and docs/ARABIC_ENGINE_WORKLOG.md in the same commit as every code change.
- Do not claim a test, build, artifact, or real-client result without its corresponding evidence.

## User-reported application startup error

The user's Windows log reported:

    builtin/common/settings/dlg_settings.lua:179:
    '}' expected (to close '{' at line 176) near 'be'

The confirmed defect was a missing comma after the Arabic language label in luanti/code/builtin/common/settings/dlg_settings.lua. It was fixed in commit 9bdd876eea9ab8087906b4d067d76fd587e84fc8. The duplicate luanti/5.17.0/builtin copy already had the comma. The fast workflow now parses all builtin Lua files in both trees using pinned luaparser==4.2.0.

## SHA-256 warnings — cause and fix in this follow-up

The generated map in builtin_files.cpp uses forward-slash keys. fs::MakePathRelativeTo() returns native separators, which are backslashes on Windows. The old map lookup therefore missed existing keys.

This follow-up:
- Adds normalizeBuiltinFileSha256Path() in builtin_files.h.
- Converts backslashes to forward slashes before the SHA map lookup in ScriptApiSecurity::safeLoadFile().
- Extends testBuiltinFileSha256Manifest() to check forward-slash, Windows-backslash, and mixed-separator inputs against real manifest entries.
- Keeps the warning enabled; the fix normalizes the lookup rather than suppressing the warning.

A new Windows artifact must still prove that the warning disappears when the executable and builtin directory are from the same package.

## Run #102 checkpoint

Run #102 ID 42774549330, HEAD 9bdd876eea9ab8087906b4d067d76fd587e84fc8:

- Catalog and Lua syntax validation: passed.
- Windows client build: passed.
- Windows TestArabicTextLayout: 16 tests, zero failures.
- Windows portable package checks: passed.
- Windows artifact luanti-arabic-windows-x64: ID 13742874657, size 20,661,857 bytes, SHA-256 223f79db5016444ac4524a5f2b72e14762cd128d247b3f888e096a7828a7a626.
- Linux client build: passed.
- Linux TestArabicTextLayout: 4 failures / 16 tests; no Linux package was produced.

## Outstanding Linux shaping failures

1. testArabicContextualShaping: two Beh input scalars yielded four glyphs.
2. testHarfBuzzOpenTypeArabicShaping: direct HarfBuzz path also yielded four glyphs for two Beh scalars.
3. testLamAlefLigature: two glyphs returned instead of the expected mandatory ligature.
4. testMouseHitTestingRespectsShapingClusters: the interior lam-alef caret boundary remained allowed because shaping did not create the ligature cluster.

Run #102 diagnostics showed a Unicode charmap, a valid Beh glyph, and a 5,854-byte GSUB table visible through FreeType, hb-ft, and direct HarfBuzz. Direct HarfBuzz used RTL direction, Arabic script, and Arabic language, yet output four glyphs. Continue investigating the exact GSUB Script/LangSys/Feature/Lookup data and HarfBuzz OpenType shaping on Linux. Do not weaken the four assertions.

## Next steps

1. Run CI for the Windows SHA path-normalization fix.
2. Confirm the new Windows SHA regression tests pass.
3. Test a newly downloaded Windows artifact; verify the language settings dialog opens and the SHA warnings disappear.
4. Continue Linux HarfBuzz/FreeType/GSUB investigation until all Arabic tests pass, then verify Linux packaging.
5. Test clipboard copy/cut/paste, mouse drag/Shift-selection, and visual clipping/alignment on the real client.

## Documentation maintenance

For every code commit, append a dated entry to docs/ARABIC_ENGINE_WORKLOG.md, update this handoff, PROGRESS.md, and docs/ARABIC_ENGINE_SUPPORT.md. Record exact commit SHA, workflow run ID, job conclusions, test counts, and artifact IDs.


## Run #103 checkpoint — current continuation state

Run #103 ID `42777977334`, HEAD `baa79571161b0558afb3185ee7bbab3f349a4c44`:

- Catalog validation: passed.
- Lua syntax validation: passed for both builtin trees, including the corrected language-label table.
- Linux client build: passed; Arabic regression module failed 4/16 tests, so no Linux package was produced.
- Windows client build: passed.
- Windows Arabic regression module: passed (16 tests; the workflow's zero-failure summary guard succeeded).
- Windows portable package checks: passed.
- Windows artifact: `luanti-arabic-windows-x64`, ID `13745910497`, size 20,663,963 bytes, SHA-256 `e8a6e012915bed39de21d23cce6d8afb1639bf5744d86217512a06b5b9255ddb`.

The user-reported Lua parser error is confirmed fixed: the Arabic option now ends with a comma before the Belarusian `be` entry. The fast parser validates every builtin Lua file in both trees, and the Windows package installation log confirms that the corrected `dlg_settings.lua` is included.

The Windows SHA path-normalization test also passes as part of the 16-test module. The code normalizes native backslashes before the generated forward-slash manifest lookup. A real desktop launch with the matching artifact is still needed to verify that the original warnings disappear in the user's installed environment.

The Linux client compiles, but its four shaping failures remain:
- two Beh characters produce four glyphs instead of two in both Raqm and direct HarfBuzz paths;
- lam-alef produces two glyphs instead of the expected ligature;
- the corresponding interior caret boundary remains allowed.

Run #103's diagnostics show matching 5,854-byte GSUB tables through FreeType, hb-ft, and direct HarfBuzz; the resolved static link closure contains vcpkg's Raqm, HarfBuzz, FreeType, and FriBidi archives. The next test revision now records the exact font path/family, HarfBuzz version and environment override, chosen shape-plan shaper, nominal Beh glyph, Arabic GSUB Script/default-LangSys feature tags, and compares the default shaper with an explicitly selected OpenType shaper for Beh and lam-alef. The four existing correctness assertions remain enforced.

### Immediate next steps

1. Run the expanded Linux shaping diagnostic and compare its output with the passing Windows result.
2. Fix the actual Linux shaping discrepancy without weakening the contextual, lam-alef, or cluster-boundary assertions.
3. Re-run Linux and Windows once the shaping patch is complete; require all 16 Arabic tests to pass and verify both packages.
4. Ask the user to test the matching Windows artifact linked in the conversation: launch the menu, open Settings/Language, and confirm the SHA warnings no longer appear.
5. Continue real-client checks for clipboard, drag/Shift selection, and text clipping/alignment.


## Run #104 correction — public HarfBuzz header and GSUB feature count

Run #104 ID `42781584482`, HEAD `8f65db053c48e78c7bfe208b3f8c15562b7706a8`, passed catalog/Lua validation and CMake configuration. Linux stopped compiling the diagnostic test at `#include <harfbuzz/hb-shape-plan.h>`: HarfBuzz 14.3.1 deliberately rejects direct inclusion of that internal header and requires the public `<hb.h>` umbrella header. The test already included `<harfbuzz/hb.h>`, which exports the shape-plan declarations, so the direct internal include has been removed.

Reviewing the diagnostic against HarfBuzz's API implementation exposed a second issue: `hb_ot_layout_language_get_feature_tags()` returns the total available feature count, whereas its in/out `feature_count` argument is a caller-supplied output capacity and becomes the number copied. The previous code passed zero capacity and then incorrectly used the output count as the vector size, making the feature list empty. It now uses the function's return value to allocate the vector, then reads the tags with matching capacity.

The Windows job in Run #104 was still building when this correction was prepared. The push of the follow-up commit supersedes that run through the workflow's `cancel-in-progress` policy. Run #105 must verify both compilation and the new diagnostic output; no shaping fix is claimed yet.

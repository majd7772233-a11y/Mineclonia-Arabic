# Arabic Engine Support — Continuation Handoff

**Last updated:** 2026-11-10  
**Repository:** majd7772233-a11y/Mineclonia-Arabic  
**Active branch:** work/arabic-engine-support  
**Starting HEAD for this follow-up:** 9bdd876eea9ab8087906b4d067d76fd587e84fc8  
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

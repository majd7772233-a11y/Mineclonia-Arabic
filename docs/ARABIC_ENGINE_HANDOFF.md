# Arabic Engine Support — Continuation Handoff

**Last updated:** 2026-11-10  
**Repository:** `majd7772233-a11y/Mineclonia-Arabic`  
**Active branch:** `work/arabic-engine-support`  
**Starting HEAD for this session:** `5412b95bf891000b783aa1acff23709512d957e5`  
**Protected `main` HEAD observed:** `a738a6bddbc0a75fc882ef68d3ffb3b00592aa0d`

## Safety and workflow rules

- Make all changes only on `work/arabic-engine-support`.
- Never modify, reset, merge, or push to `main`.
- Do not create extra branches.
- Preserve the vcpkg manifest binary-cache restore/save steps in the build workflow.
- Prefer one thoroughly reviewed commit per coherent batch; the full native workflow is expensive.
- Every code commit must update `PROGRESS.md`, `docs/ARABIC_ENGINE_SUPPORT.md`, this handoff, and `docs/ARABIC_ENGINE_WORKLOG.md`.
- Do not claim a compile, test, artifact, or real-client result unless the corresponding CI log or user test confirms it.

## Immediate application startup issue

User ran the Windows client and received:

```
builtin/common/settings/dlg_settings.lua:179:
'}' expected (to close '{' at line 176) near 'be'
```

Root cause: the Arabic language-label table in `luanti/code/builtin/common/settings/dlg_settings.lua` lacked a comma after the `ar` entry. The duplicate file under `luanti/5.17.0/builtin` already had the comma.

Changes prepared in this batch:
- Add the missing comma in the source builtin file.
- Add `tools/validate_lua_syntax.py` to parse every builtin Lua script in `luanti/code/builtin` and `luanti/5.17.0/builtin`.
- Install pinned `luaparser==4.2.0` and run the validator in the quick catalog-validation job, before lengthy native builds.
- Add a C++ test for generated SHA-map entries used by the main-menu startup scripts.

## SHA-256 warning investigation

User also saw repeated `No SHA256 known for builtin file` warnings. In `src/script/cpp_api/s_security.cpp`, this means the runtime relative builtin path has no key in `g_builtin_file_sha256_map`. That branch emits a warning and continues to load the file; it is separate from the Lua syntax error. The C++ regression assertion checks that known menu scripts have 64-character hexadecimal SHA entries in the CI-built executable.

If the new artifact still emits the warnings:
1. Confirm the executable and its `builtin` directory came from the same artifact.
2. Reproduce the exact `fs::MakePathRelativeTo(path, Server::getBuiltinLuaPath())` result on Windows.
3. Compare the normalized relative path with the generated map key.
4. Fix path normalization or packaging only after the mismatch is demonstrated; do not suppress the warning.

## Native CI checkpoint before this patch

Run #101, ID `42710027118`, HEAD `5412b95bf891000b783aa1acff23709512d957e5`:
- Catalog validation: passed.
- Windows client: built successfully.
- Windows Arabic regression module: 15/15 passed.
- Windows portable package checks and upload: passed. Artifact ID `13716251277`.
- Linux client: built successfully.
- Linux Arabic regression module: 4 failures / 15 tests. There is no Linux package artifact from this run.

Linux failures:
1. `testArabicContextualShaping`: 4 glyphs for two Beh characters.
2. `testHarfBuzzOpenTypeArabicShaping`: direct HarfBuzz path returns 4 glyphs.
3. `testLamAlefLigature`: ligature not formed.
4. `testMouseHitTestingRespectsShapingClusters`: lam-alef caret boundary is incorrectly allowed.

Run #101 diagnostic showed the font has a Unicode charmap, the Beh glyph exists, and GSUB tables are visible from FreeType, hb-ft, and direct HarfBuzz. The actual HarfBuzz direct output was four glyphs. Investigate the hb-ft/OpenType font funcs, scale/ppem, buffer direction/script/language, and HarfBuzz build configuration. Do not weaken expected glyph counts or cursor-boundary tests.

## Required next steps

1. Verify the latest commit and the new Lua syntax gate in CI.
2. If the syntax parser flags other builtin files, repair all confirmed syntax defects in one review batch and update this file.
3. Ensure the SHA manifest regression assertion passes; if it passes yet the user still sees warnings, investigate artifact mismatch and runtime path normalization.
4. Test the Windows GUI with the new artifact, especially opening the language settings list.
5. Continue Linux HarfBuzz/FreeType shaping diagnosis and do not declare the Linux build complete until all Arabic regression assertions pass and the Linux package is uploaded.
6. Keep clipboard, real mouse-selection, and visual UI testing outstanding until tested on the real client.

## Documentation maintenance

For every follow-up commit, add a dated entry to `docs/ARABIC_ENGINE_WORKLOG.md`, update the state and next action in this file, and update `PROGRESS.md` plus `docs/ARABIC_ENGINE_SUPPORT.md`. Record exact commit SHA, workflow run ID, job conclusions, test counts, and artifact IDs.

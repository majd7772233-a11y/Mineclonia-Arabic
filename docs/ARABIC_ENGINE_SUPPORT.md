# دعم العربية في محرك Luanti

> **الحالة الحالية 2026-11-12:** هذه الصفحة تحتفظ بالتاريخ التقني. المرجع التنفيذي هو [PROJECT_STATE.md](PROJECT_STATE.md)، وخارطة العمل [ROADMAP.md](ROADMAP.md)، وتعليمات البناء [BUILD_TESTING.md](BUILD_TESTING.md)، ونقطة التسليم [ARABIC_ENGINE_HANDOFF.md](ARABIC_ENGINE_HANDOFF.md). لا تستخدم تشغيلات تاريخية لإثبات HEAD الحالي.


## Objective

Implement native Unicode text layout in Luanti's client renderer. Arabic support must not
depend on pre-shaped translation strings, font-specific presentation-form substitutions,
or reversing the source text. The engine must retain logical Unicode text for translation,
copy/paste, search, editing, and network transport, and derive visual glyph runs only at
layout/render time.

## Chosen layout stack

The client uses **libraqm** as the single paragraph-layout interface. Raqm combines:

- Unicode Bidirectional Algorithm processing through FriBidi;
- OpenType shaping and glyph positioning through HarfBuzz;
- FreeType face selection and glyph metrics/rasterization.

The pinned vcpkg baseline already contains libraqm, so the source uses that package rather
than maintaining a local shaping implementation. Linux distribution builds additionally use
`libraqm-dev`; the existing Homebrew CI path installs the `libraqm` formula. The C++ renderer consumes Raqm's glyph
IDs, advances, offsets, source clusters, and selected FreeType face. It must not treat a
Unicode scalar value as if it were a glyph ID.

## Required implementation areas

1. **Font and glyph pipeline**
   - Shape logical Unicode paragraphs with Raqm.
   - Cache and rasterize by shaped glyph ID, not by source character.
   - Support font fallback by grapheme/run, retaining the face associated with each
     returned glyph.
   - Bundle a license-cleared Arabic-capable UI font and retain the existing generic
     fallback for other scripts.
2. **One layout result for every consumer**
   - Use the same shaped result for drawing, width/height measurement, clipping,
     centering/alignment, cursor hit-testing, and editing.
   - Preserve cluster-to-source-index mappings, including UTF-16 surrogate pairs on
     Windows.
   - Keep combining marks and ligatures indivisible during wrapping and cursor movement.
3. **Bidirectional and paragraph behavior**
   - Resolve paragraph direction using Unicode BiDi, including mixed Arabic, Latin,
     European/Arabic-Indic digits, punctuation, brackets, isolates and embeddings.
   - Shape each paragraph independently; line wrapping happens before final line layout.
   - Keep logical text unchanged. Never reverse/rewrite the string that is stored or sent.
4. **GUI coverage**
   - Labels, buttons, menus, tooltips, tables, chat, inventory, settings, formspecs,
     hypertext, multiline static text, and text input must use the same engine layout.
   - Respect enriched per-character colors without splitting shaping clusters.
   - Explicit RTL alignment and automatic first-strong direction must both work.
5. **Language and distribution**
   - Add Arabic to the source language selector and remove only the obsolete engine
     blacklist entry; keep installed and source builtin files synchronized.
   - Preserve the complete Luanti gettext catalog while enabling LANG_CODE=ar.
   - Ship libraqm runtime dependencies and the bundled font in Windows and other clients.

## Acceptance test matrix

Tests are required at three levels:

- **Layout unit tests:** the current suite covers Arabic contextual joining, lam-alef and its
  shaping-cluster cursor/selection boundaries, selected Arabic Extended letters, Arabic/Latin
  mixed runs with discontiguous visual selection intervals, BiDi isolates with Arabic-Indic
  digits, combining-mark grapheme boundaries, explicit paragraph direction, font fallback,
  visual word movement, visual word-deletion ranges in mixed-direction text, mouse hit-testing at safe cursor boundaries, and Unicode-aware word-selection ranges.
  Further coverage remains for paired brackets/embeddings, emoji and supplementary-plane
  characters, invalid Unicode replacement, color spans, and additional grapheme boundaries.
- **GUI integration tests:** measurement equals rendered advances; wrapping never detaches
  marks or splits a grapheme or shaped ligature; hit-testing returns logical source indices in
  mixed-direction text; edit-box selection recolors the full shaped line rather than reshaping
  an isolated Arabic substring; Ctrl+Backspace/Delete use Raqm visual word boundaries; mouse hit-testing returns only safe grapheme/shaping-cluster boundaries; double-click selection uses Unicode word categories.
  Still to verify: discontiguous visual selection backgrounds, copy/paste, clipping, shadows,
  centering, and every alignment.
- **Runtime/build tests:** Windows MSVC and Linux client builds; the dedicated Arabic layout regression module on both platforms; a Windows portable package
  containing all required DLLs and fonts; startup without external font installation;
  Arabic game UI and Mineclonia content; a mixed-direction regression screenshot suite.

Android and the legacy MinGW/macOS-precompiled-dependency paths still require matching
Raqm, HarfBuzz, and FriBidi binaries before those targets can be called supported. The
standard vcpkg Windows and Linux jobs are the first supported build targets.

A translation-PO validation pass is not evidence that the engine renders Arabic correctly.
The feature is complete only when these tests run against the built client and the rendered
output is inspected.


## Implementation status — 2026-11-08

### Completed in the working branch

- Added the complete Arabic built-in runtime catalog: 245/245 keys from `builtin/locale/template.txt`.
- Added a byte-identical packaged copy under `luanti/5.17.0/builtin/locale/__builtin.ar.tr`.
- Preserved `LANG_CODE=ar` in the Arabic Luanti gettext catalog.
- Added 840 Arabic engine gettext translations across six batches (111, 163, 160, 160, 151, 95) and reviewed all 249 previously fuzzy entries.
- Added `tools/validate_luanti_arabic.py` and made engine builds depend on Arabic catalog validation plus `msgfmt --check --check-format`.
- Current gettext coverage: **1,575 / 1,575 messages (100%)**. There are **0 empty entries** and **0 fuzzy entries**; all 249 previously fuzzy entries were individually corrected/reviewed and their fuzzy status removed.
- The validation workflow checks source/bundle synchronization, template key parity, empty built-in translations, placeholders, PO metadata, `LANG_CODE`, and a complete gettext coverage floor of 1,575 messages.

### Still in progress / not yet accepted

- Complete Linux and Windows client builds, run the C++ unit tests, and verify all Raqm/HarfBuzz/FriBidi runtime dependencies are included in the portable packages.
- Verify the GUI integration matrix: static labels, buttons, menus, formspecs, hypertext, chat, inventory, settings, input editing, selection, cursor movement, mixed-direction text, ligatures, diacritics, combining marks, and font fallback.
- Capture and inspect mixed Arabic/Latin regression screenshots on actual clients. A successful PO validation alone is not evidence that Arabic rendering is correct.

The current work is isolated to `work/arabic-engine-support`; it has not been merged into `main`. Android and legacy toolchain targets remain unsupported until matching Raqm, HarfBuzz, and FriBidi binaries are available and tested.


### Complete gettext catalog — 2026-11-08

The Arabic Luanti gettext catalog is now complete: **1,575/1,575 messages translated**, with no empty entries, no remaining fuzzy flags, no duplicate gettext keys, and no placeholder mismatches in the catalog scan. The 249 entries that were previously fuzzy were individually corrected or confirmed before removing their fuzzy markers. `LANG_CODE` remains `ar`.

The CI coverage floor is set to 1,575 so a newly introduced untranslated engine message causes validation to fail. This is a catalog-completeness milestone only; it is not yet proof of a successful native client build or correct visual layout in every GUI surface. The Linux/Windows build, C++ regression tests, packaged runtime dependencies, and real-client RTL screenshot matrix remain outstanding until their CI and runtime results are verified.


### Additional edit-box shaping safeguards — 2026-11-08

- Cursor movement and deletion now respect both Unicode grapheme boundaries and Raqm glyph-cluster boundaries. This prevents the cursor from stopping between two source characters that are rendered as one mandatory ligature, such as lam-alef.
- Added the shared helper `src/irrlicht_changes/font_text_layout.h` and regression assertions proving that the lam-alef source boundary is a valid UAX #29 grapheme boundary but not an allowed shaped-cursor boundary.
- Edit-box selection highlighting now passes the full logical line through Raqm with per-character colors. It no longer reshapes only the selected Arabic substring, which could change joining forms at the selection edges.
- Mixed-direction selection backgrounds are now derived from Raqm glyph clusters and drawn as multiple visual intervals when required. The full logical line remains in the shaping context, and partial selections expand their foreground color to the complete ligature cluster. Copy/paste and real-client verification still need to be completed.


### Discontiguous BiDi selection geometry — 2026-11-08

- Added `getVisualSelectionIntervals` in `font_text_layout.h`. It walks Raqm's visual glyph order, maps each glyph's logical cluster range to its visual advance, then merges only adjacent/overlapping visual intervals. A logical selection can therefore produce multiple separate rectangles.
- Added `CGUITTFont::getVisualSelectionLayout`, which returns visual pixel intervals and a cluster-expanded source range. It converts UTF-16 indices safely on Windows and keeps color selection aligned with complete shaping clusters.
- Updated `CGUIEditBox::draw` to draw each returned highlight interval independently and render the complete enriched logical line with cluster-expanded selected colors.
- Added a regression using `abc مرحبا def`: selecting the Latin prefix and only the first logical Arabic character must produce two separated visual intervals. Added a lam-alef regression showing that selecting only the alef expands the foreground color range to the complete lam-alef glyph.
- This implementation still requires the current Linux and Windows CI jobs to compile and run the unit tests. It also needs interactive verification for mouse drag selection, keyboard Shift-selection, copy/paste, and scrolling.

### Visual word deletion in edit boxes — 2026-11-08

- `CGUIEditBox::onKeyBackDelete` now routes Ctrl+Backspace and Ctrl+Delete through
  `CGUITTFont::getVisualWordCursorPosition` when the active font is Raqm-backed.
  This keeps deletion aligned with the visual word navigation already used by Ctrl+Left/Right.
- The returned boundary is a logical source index, so deletion removes the contiguous source
  interval between that boundary and the caret even when visual order reverses logical indices.
  Existing fallback behavior remains for other font implementations.
- A no-op boundary at the current caret is treated as no deletion.
- Added `testVisualWordDeletionRangesMixedDirection` to check both visual directions in
  `abc مرحبا def`, including source-range and shaping-cluster-safe endpoints. The test
  still needs to pass in the Linux and Windows client test runs.

### Safe edit-box hit testing and Unicode word selection — 2026-11-09

- Replaced raw `raqm_position_to_index` mouse hit results with nearest visual caret-stop selection. Candidate stops are filtered through both Unicode grapheme boundaries and Raqm shaping-cluster boundaries, so clicks cannot place the caret inside a combining sequence or a mandatory ligature such as lam-alef.
- Preserved the logical source-index model and the UTF-16-to-UTF-32 index conversion used by Windows.
- Added a Unicode-aware logical word-range helper for double-click selection. It classifies whitespace and punctuation independently of the process locale, so Arabic letters are not mistaken for non-alphanumeric punctuation under the default C locale.
- Word ranges and selected-color ranges are expanded to complete grapheme and shaping clusters. The same expanded range now drives both visual selection rectangles and foreground selection colors.
- Added regression coverage for hit-testing across lam-alef, combining marks and mixed BiDi text, as well as Arabic words, Arabic punctuation and repeated whitespace.
- These changes, together with the include-path, helper declaration-order, edit-box scope and unit-test macro fixes identified in run #81, require the next complete Linux/Windows build and C++ unit-test run. Interactive clipboard round trips and real-client mouse/selection checks remain outstanding.


### Cross-platform test and packaging hardening — 2026-11-09

- MSVC now receives `/utf-8` for every source file, so Arabic UTF-8 string literals in the native shaping tests are decoded consistently with Linux.
- Linux and Windows CI invoke `TestArabicTextLayout` directly and require both a zero exit code and an explicit zero-failure module summary.
- The run-in-place texture marker is now tracked and explicitly unignored, matching the existing CMake install rule.
- Static vcpkg builds now prefer imported CMake targets for cURL, OpenAL, FreeType, fmt, BZip2 and Brotli so their static transitive libraries are linked rather than silently omitted.
- The next CI run still needs to confirm the toolchain-specific imported target names, both native builds, the dedicated Arabic module and both packages.


## Run #87 diagnosis and follow-up corrections — 2026-11-09

Run #87 passed catalog validation and completed vcpkg manifest installation on both platforms, but both native jobs stopped during CMake configuration. Neither compiler build, Arabic regression test, nor package installation ran. The shared failure was the legacy `VORBIS_FOUND` check after `CMAKE_FIND_PACKAGE_PREFER_CONFIG=ON` selected vcpkg's `VorbisConfig.cmake`, which exports imported targets and the canonical mixed-case package result instead of the legacy variables expected by the old code.

The correction prefers the vcpkg `Vorbis::vorbisfile`, `Vorbis::vorbis`, and `Ogg::ogg` imported targets, with an explicitly forced `MODULE` fallback for the repository's legacy `FindVorbis.cmake`. Sound configuration now also requires a non-empty Vorbis link list. The client subdirectory no longer overwrites the parent link list, preserving `OpenAL::OpenAL`, `fmt::fmt`, and their transitive link requirements.

The follow-up review also guarded automatic paragraph-direction detection in `StaticText`: Raqm detection is now called only for the custom TrueType font, rather than casting every GUI font to `CGUITTFont`. Both CI configurations explicitly keep sound enabled, and the Windows package check now verifies the OpenAL/Ogg/Vorbis runtime DLLs as well as the Arabic shaping DLLs.

These are source/configuration corrections only. A new Linux and Windows run must still prove that the client compiles, all 13 Arabic layout tests pass, and the portable packages contain their runtime requirements. The upstream full unit-test suite and real-client visual/clipboard verification are not covered by the dedicated Arabic test-module gate.


## Run #91 diagnosis and CMake signature normalization — 2026-11-09

The Run #91 Linux configure log confirms that the vcpkg Vorbis correction is effective: CMake reports `Vorbis found through imported CMake targets` and `Sound enabled with OpenAL and Vorbis`. Configuration then exposed a separate CMake contract violation: the `luanti` target's original `target_link_libraries` call uses the plain signature, while four newly added calls used the keyword signature (`PRIVATE`). CMake rejects mixing the two signatures for one target.

The four imported-dependency calls (BZip2/Brotli, fmt, and cURL) are now normalized to the target's existing plain signature. This preserves the imported targets and their static transitive dependencies without changing unrelated targets. The Linux configure did not proceed to compilation, unit tests, or packaging in Run #91; Windows was still installing manifest dependencies when this correction was prepared. Those outcomes therefore remain unverified.


## Run #92 diagnosis and imported Zlib target ordering — 2026-11-09

The Run #92 Linux configure log confirms both preceding fixes: Vorbis and OpenAL are found, and the plain/keyword `target_link_libraries` signature errors are gone. CMake then reported that imported targets from vcpkg's cURL, FreeType, and PNG packages reference `ZLIB::ZLIB` before that target exists.

The project already called `find_package(ZLIB REQUIRED)`, but only after the cURL and FreeType package configs were loaded. The existing lookup is moved ahead of those config packages so the imported `ZLIB::ZLIB` target exists at the moment their target interfaces are declared. This is an ordering fix; it does not add a dependency, disable cURL/audio, or replace imported targets. Run #92 did not reach compilation, Arabic tests, or packaging, so those remain pending verification.


## Run #93 diagnosis and static Zlib target normalization — 2026-11-09

Run #93 still failed at CMake generation on Linux. The earlier ordering change alone did not solve the issue: the vcpkg static zlib config exports `ZLIB::ZLIBSTATIC`, while the imported cURL, FreeType, and PNG interfaces in this build require the conventional `ZLIB::ZLIB` target. Since `CMAKE_FIND_PACKAGE_PREFER_CONFIG=ON` prefers the config package, the prior unqualified lookup did not create the target expected by those dependent packages.

The lookup now explicitly selects CMake's built-in `FindZLIB.cmake` module (`find_package(ZLIB MODULE REQUIRED)`). The module reads the include and library paths supplied by vcpkg and creates the standard `ZLIB::ZLIB` imported target. A configure-time guard fails immediately with a specific diagnostic if that target was not created. This preserves static linking and keeps the rest of the package lookups in config mode. Compilation and tests are still unverified.


## Run #94 compile failure and edit-box state hardening — 2026-11-09

Run #94 passed CMake configuration on both Linux and Windows and reached native compilation. Both compilers stopped at the same real source error in `CGUIEditBox::onKeyControlX`: local constants reused the names of its `mark_begin` and `mark_end` reference parameters. The local variables are now named `clamped_mark_begin` and `clamped_mark_end`.

The shared replacement algorithm is extracted into `editbox_text_edit.h` and now builds into a temporary candidate, publishing it only after the maximum-length check succeeds. The Arabic text-layout test module now also tests selected-range replacement, rejection without mutating the output, overwrite behavior at newlines, stale index clamping, and empty input.

A further edit-box state audit fixed three adjacent issues: replacing the caption now refreshes scroll/layout state; reducing the maximum length clamps the caret and selection and rebuilds wrapped lines; and disabling multiline/word-wrap clears stale line caches. CRLF normalization now bounds-checks the look-ahead and adjusts selection endpoints when it removes the carriage return. Mouse hit testing also safely returns when no active font or wrapped lines are available.

The next CI run must still confirm the full client compiles, the now-expanded Arabic module passes, and both packages install. No compile/test/package success is inferred from these source changes.


## Run #95 linker diagnosis and imported dependency corrections — 2026-11-09

Run #95 used commit `6a2ac99`. Both platforms completed vcpkg installation, CMake configuration, and all C++ compilation. The Linux executable failed only at the final link because the fallback `libsha256.a` and static OpenSSL `libcrypto.a` both exported the same SHA256 symbols. The source explicitly selects OpenSSL's EVP implementation when `USE_OPENSSL` is true, so the in-tree `sha256` library is now linked only in the non-OpenSSL fallback branch for EngineCommon, client, and server.

The Windows link exposed two dependencies that the previous variable-based linkage did not preserve. Zstd's vcpkg x64-windows package uses DLL-import declarations and its `zstd::libzstd` target carries the matching import library. IrrlichtMt's static archive also needs libpng, but the old `${PNG_LIBRARY}` variable did not carry the vcpkg config target into the final executable. The project now prefers `zstd::libzstd` and `PNG::PNG` imported targets, retaining module-based fallbacks for non-vcpkg environments. Both target selections fail during configure with a direct message if neither a usable target nor fallback library is available.

No test module or package step ran in Run #95 because the final link failed. The new corrections still require a fresh Linux and Windows build, Arabic regression test execution, and package verification.


## Run #96 scope-specific Zlib and SRP header corrections — 2026-11-09

Run #96 confirmed that the Run #95 dependency-link changes moved the failure forward. On Windows, Zstd/PNG configuration passed and the compiler reached `src/util/srp.cpp`, but that source includes `my_sha256.h` unconditionally even when OpenSSL provides the SHA implementation. The header directory had previously arrived transitively through linking the fallback `sha256` target; removing that target from OpenSSL builds correctly avoids duplicate SHA symbols but also removed the include path. `EngineCommon` now receives the header directory explicitly, independently of whether the fallback library is linked.

On Linux, CMake still failed while importing PNG because `PNG::png_static` references `ZLIB::ZLIB`. The earlier `FindZLIB MODULE` call was in the sibling `src/` directory and its non-global imported target was not visible in `irr/src/`. Irrlicht now invokes `find_package(ZLIB MODULE REQUIRED)` locally before loading PNG and checks that `ZLIB::ZLIB` exists in that scope.

Run #96 did not reach linking, Arabic tests, or package installation on either platform. The next run must verify these scope/include corrections and then continue through the final link and packaging.


## Run #97 Linux Raqm dependency-coherence correction — 2026-11-09

Run #97 completed the full Windows path: the client built, all 14 Arabic layout tests passed, the portable package passed its runtime-DLL checks, and the Windows artifact uploaded. Linux also configured and built the client successfully, but three Arabic shaping assertions failed: contextual Arabic shaping returned four glyphs for two Beh letters, lam-alef returned two glyphs instead of one, and the shaping-cluster cursor test consequently allowed the interior lam-alef boundary. The other 11 tests passed.

The regression is isolated to the Linux static dependency path; Windows uses the vcpkg dynamic triplet and passed the same tests. The Linux configuration previously resolved `raqm`, `harfbuzz`, and `fribidi` as independent raw library paths. The fix now uses the vcpkg-generated `raqm.pc` with `PKG_CONFIG_USE_STATIC_LIBS=ON` on Linux, constraining package discovery to the active triplet's `lib/pkgconfig` directory. This carries the Raqm archive's declared static dependency closure and prevents a mixed system/vcpkg shaping stack. Windows and non-vcpkg platforms retain their existing library discovery path.

The next CI run must confirm that Linux now uses the vcpkg Raqm 0.11.0 package closure and that the contextual shaping, lam-alef, and cluster-boundary tests pass. No new test or package success is claimed from this source change alone.


## Run #98 Linux static Raqm package-metadata correction — 2026-11-09

Run #98 confirmed that both client builds and the Windows Arabic regression suite now work. Windows completed the portable package and uploaded artifact. Linux compiled the full client but failed three shaping assertions: contextual Arabic shaping, lam-alef ligature formation, and the corresponding shaping-cluster cursor boundary. The remaining 11 of 14 Arabic tests passed.

The Run #97 change intended to request static pkg-config dependencies by setting `PKG_CONFIG_USE_STATIC_LIBS=ON`. Inspection of CMake 4.4.4's `FindPkgConfig.cmake` shows that this is not a supported control variable. Imported targets are built from the regular `<prefix>_LDFLAGS`, while `--static` is used only to populate the separate `<prefix>_STATIC_*` result variables. Consequently, the imported `PkgConfig::RAQM` target was not guaranteed to contain the static package's `Requires.private`/`Libs.private` dependency closure.

The Linux path now appends `--static` through the documented `PKG_CONFIG_ARGN` variable for the Raqm query, restores the previous value immediately afterward, and inspects the imported target's actual link interface. Configuration fails with a diagnostic unless both HarfBuzz and FriBidi are present in that interface; the resolved closure is also printed to the job log. This keeps the correction limited to the Linux vcpkg Raqm target and leaves Windows/non-vcpkg discovery unchanged.

Run #98's Linux build itself succeeded, but the Arabic test module did not; no Linux artifact was produced. The next run must show the complete static dependency closure and pass all 14 Arabic tests before Linux is considered verified.


## Run #99 Linux shaping isolation — 2026-11-10

Run #99 built the Linux client successfully and printed the expected static vcpkg Raqm link closure (libraqm, HarfBuzz, FreeType, BZip2, PNG, zlib, Brotli, FriBidi). The Linux Arabic regression module nevertheless failed three assertions: contextual Arabic shaping (4 glyphs for two Beh characters), lam-alef ligature formation (2 glyphs instead of 1), and the dependent shaping-cluster caret boundary. The other 11 tests passed.

The same commit completed the full Windows path: client build, all 14 Arabic tests, portable-package dependency verification, and Windows artifact upload. This narrows the remaining issue to the Linux static Raqm/HarfBuzz/FreeType path, not to the common edit-box implementation or CMake configuration.

A new regression diagnostic directly invokes HarfBuzz's FreeType integration using the same Noto Sans Arabic face and pixel size as the Raqm tests. It verifies that the OpenType shaper is available, checks contextual forms for two Beh characters, and checks lam-alef ligature formation. This isolates HarfBuzz/FreeType from Raqm's script, language, and bidi-run setup. The next CI result will tell whether the defect is already present in direct HarfBuzz shaping or is introduced by Raqm integration. This diagnostic has not yet been run; no Linux shaping fix or Linux package success is claimed.


## Run #100 Linux HarfBuzz/FreeType diagnostic expansion — 2026-11-10

Run #100 provides a strong platform split: Windows completed the client build, all 15 Arabic regression tests, portable package verification and artifact upload. Linux also built and linked the client, but failed four assertions in the Arabic module. Three failures are the same shaping defect (contextual forms, lam-alef ligature and the resulting cluster boundary); the independent direct-HarfBuzz test reproduced the incorrect output, so this is not limited to Raqm's bidi/script itemization.

The test helper previously reinterpreted a `char32_t` buffer as `uint32_t` for Raqm. Those are distinct C++ types, so the read could violate strict-aliasing rules under optimized Linux builds. The helper now copies code points into a `std::vector<uint32_t>`, matching the production path and avoiding type-punning. The diagnostic also compares HarfBuzz's direct OpenType face with its FreeType face adapter using the exact same font file, and records Unicode charmap selection, FreeType glyph mapping, GSUB table visibility through FreeType and HarfBuzz, UPEM, and resulting glyph IDs/clusters.

This is a targeted correctness fix plus diagnostics, not a claimed final shaping fix. The next CI result will show whether removing the type-punning defect restores Linux shaping and, if not, whether GSUB is lost specifically through the FreeType adapter. The Windows run remains a verified successful reference; Linux packaging is still blocked until the Arabic test module passes.


## Main-menu startup failure report — 2026-11-10

### Confirmed Lua syntax defect

The user's Windows startup log reports:

```
builtin/common/settings/dlg_settings.lua:179:
'}' expected (to close '{' at line 176) near 'be'
```

The Arabic option in the language-label table lacked a trailing comma before the Belarusian option. The source copy under `luanti/code/builtin` is corrected. The separate bundled reference copy under `luanti/5.17.0/builtin` already contained the comma and was left unchanged.

A fast syntax-validation script now parses every `.lua` file in both builtin trees before Linux/Windows native compilation. It uses pinned `luaparser==4.2.0`; parser success is a grammar check, not a substitute for loading the menu in the real Luanti executable.

### SHA-256 warnings

The repeated `No SHA256 known for builtin file` messages are emitted by `ScriptApiSecurity::safeLoadFile` when a loaded builtin path has no matching key in `g_builtin_file_sha256_map`. That branch logs a warning and continues loading the file; it is not the fatal error in the supplied log. A separate C++ regression assertion now verifies that the generated manifest contains valid 64-hex-digit SHA entries for `common/math.lua`, `common/vector.lua`, `common/settings/dlg_settings.lua`, and `mainmenu/init.lua`. This verifies those map entries in the CI-built executable; if the warning persists in a newly downloaded matching artifact, the next check must compare its executable with its installed builtin tree and reproduce the runtime relative-path lookup. The warning is not suppressed.

### Run #101 status carried forward

Run #101 used commit `5412b95bf891000b783aa1acff23709512d957e5`. Windows completed the client build, the 15 Arabic layout tests, portable-package checks, and uploaded artifact `luanti-arabic-windows-x64` (artifact ID `13716251277`). Linux completed the client build but failed 4 of 15 Arabic tests: direct HarfBuzz contextual shaping, Arabic contextual shaping through Raqm, lam-alef ligature formation, and the resulting shaping-cluster caret boundary. The other 11 tests passed. Linux has no package artifact from that run.

The four Linux failures are independent from the Lua comma defect. Keep the Linux HarfBuzz/FreeType investigation open; do not weaken the shaping assertions or treat Windows success as proof that the Linux shaping path works.


## Windows builtin SHA-256 path normalization — 2026-11-10

The generated builtin manifest uses forward-slash keys such as common/math.lua. On Windows, fs::MakePathRelativeTo() returns native backslash-separated paths, so the previous direct map lookup missed valid entries and emitted repeated No SHA256 known warnings.

The runtime now normalizes the relative path to forward slashes before looking it up. The new test coverage checks POSIX, Windows, and mixed-separator paths against actual entries in g_builtin_file_sha256_map. The warning remains active; this is a lookup fix, not warning suppression.

Run #102 (ID 42774549330, HEAD 9bdd876eea9ab8087906b4d067d76fd587e84fc8):
- Catalog and Lua syntax validation: passed.
- Windows client build: passed.
- Windows Arabic module: 16 tests, zero failures.
- Windows portable package: passed; artifact ID 13742874657.
- Linux client build: passed.
- Linux Arabic module: 4 failures / 16 tests; no Linux package.

The outstanding Linux failures are contextual Arabic shaping through Raqm, direct HarfBuzz shaping, lam-alef ligature formation, and the cursor boundary within that shaping cluster. Do not weaken these assertions. The separator-normalization change still needs a fresh Windows CI run and a real-client test with a newly downloaded matching artifact.


## Run #103 — current Windows verification and Linux shaping checkpoint

Run #103 (ID `42777977334`, HEAD `baa79571161b0558afb3185ee7bbab3f349a4c44`) passed catalog and Lua syntax validation. Both native clients compiled. Windows passed all 16 `TestArabicTextLayout` tests, portable package validation, and artifact upload (ID `13745910497`). Linux passed client compilation but failed four shaping assertions (contextual Beh through Raqm, direct HarfBuzz, lam-alef ligature, and the dependent caret boundary); it did not produce a Linux artifact.

The application startup syntax defect is already repaired: the source language-label table now has the comma after Arabic before the Belarusian `be` entry. The fast builtin Lua parser gate passed in Run #103. The Windows SHA warning fix normalizes the filesystem path separators before querying the generated SHA-256 manifest; its regression test passed in the Windows 16-test module. Real GUI verification still requires launching the newly uploaded artifact.

To isolate Linux shaping, the next diagnostic adds the font path/family, HarfBuzz version/environment, selected shaping-plan shaper, direct nominal glyph mapping, Arabic GSUB Script/default-LangSys features, and side-by-side default versus forced-OpenType shaping results. Do not weaken the four failing Linux assertions.


## Run #104 — diagnostic header/count correction

Run #104 (ID `42781584482`, HEAD `8f65db053c48e78c7bfe208b3f8c15562b7706a8`) passed quick validation and CMake configuration. Linux compilation then stopped because HarfBuzz deliberately rejects direct inclusion of `hb-shape-plan.h`; the supported `hb.h` umbrella header is already included and exposes those declarations. The direct include has been removed.

The diagnostic's GSUB feature enumeration now uses the total count returned by `hb_ot_layout_language_get_feature_tags()` to allocate storage, then passes that capacity to retrieve the tags. The prior zero-capacity query produced an empty copied-count and therefore an empty list. Run #105 must verify the correction and provide the first usable shaping diagnostic.


## Run #105 review and font-accurate shaping verification — 2026-11-10

Starting HEAD: `9eda41ba6285afb6ef0891a4493ac9cf35b32d99`. Run #105 ID: `42782985200`.

The user's Lua startup parser error was the missing comma after the Arabic language label. It is present in the current source and passed the pinned Lua parser gate in Run #105. The user's reported log timestamp (2026-11-09 23:41) predates the Run #105 Windows artifact (2026-11-10 08:39 UTC), so the old installation must be replaced with the matching artifact to verify the menu fix and SHA warnings.

Run #105:
- Catalog and embedded Lua validation: passed.
- Linux: CMake configure and full client compilation passed; Arabic module failed 4/16; no Linux package.
- Windows: CMake configure and full client compilation passed; Arabic module passed 16/16; portable package checks passed; artifact ID `13747836271`, digest `4a9a73c22c6c1f3d1b818472d22eaf0abf6549357b6cf26c43b0a336c4636890`.

Analysis of the exact bundled Noto Sans Arabic font explains the four Linux assertions:
- The sequence `بب` produces two contextual base glyphs (`uni066E.init`, `uni066E.fina`) plus two separately positioned `dotbelowar` marks. Counting one output glyph per input scalar was incorrect.
- The plain Lam-Alef sequence `لا` has no GSUB ligature in this bundled font. It is rendered as `uni0644.init.rlig` plus `uni0627.fina.rlig`, with two distinct clusters.
- The bundled font does contain a supported six-codepoint ligature: `اللّٰه` shapes to the single glyph `uniFDF2`. This is now the real multi-scalar ligature used to test caret and mouse-hit-test cluster protection.

The revised regression suite checks the actual glyph names and clusters instead of imposing ligatures that the selected font does not contain. It preserves strict assertions: both contextual forms and both dot marks must be present; plain Lam-Alef must keep two independent caret stops; the supported Allah ligature must be one glyph cluster with no interior caret positions. CI now requires exactly 17 tests and a zero-failure summary on both platforms.

No production shaping code was changed as part of this correction. Run #106 must prove all 17 tests and both portable packages before claiming completion. The user's installed copy still needs a real GUI launch using artifact ID `13747836271` or a newer matching Windows artifact.


## حالة أحدث من السجل التاريخي — 2026-11-12

- رأس الكود الذي تمت مراجعته 6e8511ef4f7b437e0ce7c6769a3405ba68c077b2.
- Run #106 فشل في compilation بسبب تعريفين مكررين على Linux وWindows.
- commit 6e8511e حذف النسخ المكررة وأضاف source validator؛ فحص نصي وجد 17/17/17، لكن لا يوجد build جديد يثبت compilation.
- Run #107 فشل دون Jobs أو Artifacts في API.
- workflow الحالي يحتوي أوامر تحقق مبتورة ومقاطع jobs مكررة.
- validate_lua_syntax.py يحتوي regex Arabic option double-escaped.
- خطأ الفاصلة في Lua وSHA path normalization تم إصلاحهما سابقاً؛ تجربة المستخدم على حزمة حديثة مطابقة لم تُثبت بعد.

راجع PROJECT_STATE.md وNEXT_SESSION_PROMPT.md. احتفظ بالتاريخ السابق كمرجع ولا تجعله بديلاً عن الحالة الحالية.


## إصلاح Workflow وفاحص Lua — 2026-11-12

Run108 انتهى بالفشل دون Jobs أو Artifacts أو check-runs؛ لذلك لا يثبت فشل compilation أو الاختبارات ولا توجد job logs تحدد السبب. فحص الملف نفسه كشف Bash grep مبتوراً وPowerShell condition غير مكتمل ومقاطع Package مكررة وتعريفات Windows متكررة.

أُعيد تنظيم Workflow إلى workflow-sanity وcatalog-validation وLinux وWindows، مع الحفاظ على cancel-in-progress وvcpkg cache restore/save في كل منصة، وإعداد الصوت، واشتراط ملخص 17/17 قبل package. تشمل فحوص الحزمة مطابقة builtin مع المصدر، والخط والترخيص، وldd على Linux، وDLLs على Windows.

في validate_lua_syntax.py تم تصحيح double escaping في regex خيار العربية. الفاحص ما زال يمسح كل Lua files في الشجرتين، ويعيد exit code غير صفري عند الخطأ. الفاصلة الصحيحة موجودة بالفعل في نسختي dlg_settings.lua؛ لذلك لم تُعدّل ملفات Lua.

فحص المصدر C++ وجد 17 declarations و17 registrations و17 definitions دون تكرار، لكن ذلك لا يثبت compilation. إصلاح SHA path normalizer لا يزال في builtin_files.h ويُستخدم في safeLoadFile؛ لم يتم إخفاء التحذير أو تعديل مسار الإنتاج.

Preflight محلي: YAML وbash -n وregex نجحت. actionlint وPowerShell parser وmsgfmt غير متاحة محلياً، وتعذر تثبيت luaparser بسبب DNS. لا يوجد build أو test أو package نجاح مثبت بعد هذه الدفعة.


## Run #109 — Desktop build, 17-test regression suite, and packages verified — 2026-11-12

Run #109 (ID 43212062926) used implementation HEAD 69c62784baf96e3456239d5b954055be5e855501 on `work/arabic-engine-support` and completed successfully. The workflow sanity, catalog/source validation, Linux native build/package, and Windows MSVC native build/package jobs all concluded `success`.

Catalog and validator evidence:
- Builtin Arabic catalog: 245/245 template keys, 0 empty, source/bundled byte-identical.
- Luanti gettext: 1,575/1,575 effective translations, 0 untranslated, 0 fuzzy; `LANG_CODE=ar`.
- PO formatting with `msgfmt --check --check-format`: passed.
- Pinned Lua parser: 210 builtin Lua files parsed across source and bundled trees.
- Arabic test source validator: 17 declarations, 17 registrations, 17 definitions, unique.

Both desktop targets completed CMake configure and full client build, ran the dedicated `TestArabicTextLayout` gate requiring `Module TestArabicTextLayout passed (0 failures / 17 tests)`, installed and validated their portable package, and uploaded an artifact. Linux additionally checked `ldd` for unresolved runtime dependencies; Windows checked the packaged builtin hash/comma, executable, Noto font/license, and Raqm/HarfBuzz/FriBidi/OpenAL/Ogg/Vorbis DLLs.

- Linux artifact ID 13941099252; size 22,718,197 bytes; SHA-256 `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows artifact ID 13941743588; size 20,672,848 bytes; SHA-256 `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.

The successful native jobs establish the desktop compile, the 17-test regression module on both CI platforms, and the package checks. They do not establish a real GUI launch on the user's machine. The menu startup and SHA warning outcomes must still be confirmed by running the complete Windows artifact and inspecting a fresh user log. Clipboard interaction, drag/Shift selection, mixed-direction keyboard editing, and broad GUI rendering remain open acceptance items. Android remains deferred.


## Independent inspection of Run #109 portable artifacts — 2026-11-12

The actual GitHub Actions artifacts were downloaded and their outer ZIP SHA-256 values were independently calculated. Both match the digest printed by the upload action:
- Linux artifact 13941099252: `8c845dcb78a8c7d1d201da457b54178582326c31b47d952eab6a25bc292a5999`.
- Windows artifact 13941743588: `112edd28d9dbbf8e2694198317a912afb97a0f4db8e4e4a482cceee17cf897d6`.

The outer ZIP archives passed integrity checks and contained the expected inner archives. The Linux inner tarball was extracted. Its x86-64 `bin/luanti` reports Luanti 5.17.0 / Release / RUN_IN_PLACE=1 under `--version`; all `ldd` dependencies resolved. The extracted runtime contains the corrected Arabic language-label entry, Arabic builtin translation, Noto Sans Arabic Regular and OFL license. Running the dedicated test module from this exact extracted package passed all 17 tests with zero failures and the explicit module summary.

The Windows inner ZIP was extracted and inspected. It contains `bin/luanti.exe`, Raqm/HarfBuzz/FriBidi/FreeType DLLs, OpenAL/Ogg/Vorbis DLLs, the Arabic language-label builtin, and Noto Sans Arabic with its OFL license. This is archive/file-presence verification only; the Windows executable was not launched in this environment. User-side Windows GUI and SHA-warning verification remain required.

## Windows portable RUN_IN_PLACE profile

The Windows portable ZIP keeps read-only data at the package root while setting the writable user profile to a sibling directory named Minetest:

- path_share: <package>
- path_user: <package>/Minetest
- path_cache: <package>/Minetest/cache

This central path change relocates worlds, installed games, settings, logs, cache and other data built from path_user without moving builtin, fonts, textures, locale or engine resources. The Linux/other desktop RUN_IN_PLACE path policy is unchanged by this Windows-only change.

On first launch, if %APPDATA%/Minetest exists and the portable destination does not, its contents are copied into a staging sibling and then renamed into Minetest. The original AppData profile is retained. If the destination already exists, the implementation does not merge profiles automatically. A stale migration staging directory is not deleted automatically; the startup logs the conflict rather than risking user data. The copied cache is preserved, and a CACHEDIR.TAG is created/read within the portable profile.

Windows RUN_IN_PLACE config lookup uses only Minetest/minetest.conf. It does not fall back to minetest.conf in the application root or a parent directory. Explicit --config remains an intentional user override.

The current CI coverage includes a path helper unit test, TestFileSys on both platforms, and a Windows smoke test with a synthetic AppData profile. Real user acceptance remains necessary for launches from paths containing spaces/removable drives and checking ContentDB downloads, new worlds, screenshots, logs, and config saves.

### Run #110 validation boundary

The Windows CI build compiled the portable path implementation and passed both C++ test modules (Arabic text layout 17/17; TestFileSys 14/14). The Windows install path and DLL checks also passed. The package smoke test failed at a duplicate TestFileSys invocation against the copied install package; the job log did not capture its process output, while the preceding packaged `--worldlist=path` command exited successfully. No Windows artifact was uploaded. The follow-up smoke test runs world discovery twice and validates migrated config/world/game/cache files and contents, cache tag location, and that the old AppData profile is unchanged. This still awaits a successful Windows CI run and real-machine acceptance.

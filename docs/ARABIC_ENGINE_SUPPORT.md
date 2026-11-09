# Arabic and complex-script text support

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

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

- **Layout unit tests:** Arabic joining positions; lam-alef and other OpenType ligatures;
  tashkeel/combining marks; Arabic Extended characters; paired brackets; isolates and
  embeddings; Arabic and Latin mixed runs; Western and Arabic-Indic digits; emoji and
  supplementary-plane characters; line boundaries; invalid Unicode replacement;
  color spans; clusters and grapheme boundaries.
- **GUI integration tests:** measurement equals rendered advances; wrapping never detaches
  marks or splits a grapheme; hit-testing returns logical source indices in mixed-direction
  text; editing/backspace/selection/copy/paste preserve logical Unicode; clipping, shadows,
  centering, and all alignments are consistent.
- **Runtime/build tests:** Windows MSVC and Linux client builds; a Windows portable package
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
- Added 323 non-fuzzy Arabic translations for engine settings and UI descriptions across two batches (163 + 160) in this work session.
- Added `tools/validate_luanti_arabic.py` and made engine builds depend on Arabic catalog validation plus `msgfmt --check --check-format`.
- Current gettext coverage: **920 / 1,326 non-fuzzy messages (69.4%)**. There are **406 empty non-fuzzy messages** and **249 fuzzy entries**; fuzzy entries are not counted as approved translations.
- The validation workflow checks source/bundle synchronization, template key parity, empty built-in translations, placeholders, PO metadata, `LANG_CODE`, and a non-fuzzy coverage floor of 760.

### Still in progress / not yet accepted

- Translate and review the remaining 566 non-fuzzy engine messages, especially the remaining settings descriptions from `src/settings_translation_file.cpp`.
- Review the 249 fuzzy entries individually; do not promote them wholesale.
- Complete Linux and Windows client builds, run the C++ unit tests, and verify all Raqm/HarfBuzz/FriBidi runtime dependencies are included in the portable packages.
- Verify the GUI integration matrix: static labels, buttons, menus, formspecs, hypertext, chat, inventory, settings, input editing, selection, cursor movement, mixed-direction text, ligatures, diacritics, combining marks, and font fallback.
- Capture and inspect mixed Arabic/Latin regression screenshots on actual clients. A successful PO validation alone is not evidence that Arabic rendering is correct.

The current work is isolated to `work/arabic-engine-support`; it has not been merged into `main`. Android and legacy toolchain targets remain unsupported until matching Raqm, HarfBuzz, and FriBidi binaries are available and tested.

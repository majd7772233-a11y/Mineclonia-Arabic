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
than maintaining a local shaping implementation. The C++ renderer consumes Raqm's glyph
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

A translation-PO validation pass is not evidence that the engine renders Arabic correctly.
The feature is complete only when these tests run against the built client and the rendered
output is inspected.

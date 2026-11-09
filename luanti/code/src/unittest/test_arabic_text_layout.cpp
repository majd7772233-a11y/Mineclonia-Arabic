// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Regression tests for native Unicode shaping and bidirectional layout.

#include "test.h"

#include "porting.h"
#include "../irrlicht_changes/font_text_layout.h"
#include "../irrlicht_changes/editbox_text_edit.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <raqm.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-ot.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace
{

struct FreeTypeFont
{
	FT_Library library = nullptr;
	FT_Face face = nullptr;

	~FreeTypeFont()
	{
		if (face)
			FT_Done_Face(face);
		if (library)
			FT_Done_FreeType(library);
	}
};

struct RaqmLayout
{
	raqm_t *value = raqm_create();

	~RaqmLayout()
	{
		if (value)
			raqm_destroy(value);
	}
};

struct HarfBuzzObjects
{
	hb_font_t *font = nullptr;
	hb_buffer_t *buffer = nullptr;

	~HarfBuzzObjects()
	{
		if (buffer)
			hb_buffer_destroy(buffer);
		if (font)
			hb_font_destroy(font);
	}
};

struct HarfBuzzFileObjects
{
	hb_blob_t *blob = nullptr;
	hb_face_t *face = nullptr;
	hb_font_t *font = nullptr;

	~HarfBuzzFileObjects()
	{
		if (font)
			hb_font_destroy(font);
		if (face)
			hb_face_destroy(face);
		if (blob)
			hb_blob_destroy(blob);
	}
};

void initializeFont(FreeTypeFont &font, const char *relative_path = "fonts/NotoSansArabic-Regular.ttf")
{
	UASSERT(FT_Init_FreeType(&font.library) == 0);
	const std::string path = porting::getDataPath(relative_path);
	UASSERT(!path.empty());
	UASSERT(FT_New_Face(font.library, path.c_str(), 0, &font.face) == 0);
	UASSERT(FT_Set_Pixel_Sizes(font.face, 0, 32) == 0);
}

void setRaqmText(RaqmLayout &layout, FT_Face face, const std::u32string &text)
{
	static_assert(sizeof(char32_t) == sizeof(uint32_t),
			"Raqm UTF-32 input requires 32-bit code points");
	UASSERT(layout.value != nullptr);
	UASSERT(!text.empty());
	const std::vector<uint32_t> codepoints(text.begin(), text.end());
	UASSERT(raqm_set_text(layout.value, codepoints.data(), codepoints.size()));
	UASSERT(raqm_set_freetype_face(layout.value, face));
	UASSERT(raqm_set_freetype_load_flags(layout.value, FT_LOAD_DEFAULT));
}

} // namespace

class TestArabicTextLayout : public TestBase
{
public:
	TestArabicTextLayout()
	{
		TestManager::registerTestModule(this);
	}

	const char *getName() { return "TestArabicTextLayout"; }

	void runTests(IGameDef *gamedef);

	void testArabicContextualShaping();
	void testHarfBuzzOpenTypeArabicShaping();
	void testLamAlefLigature();
	void testArabicExtendedLetters();
	void testMixedDirectionParagraph();
	void testBidiIsolatesAndArabicIndicDigits();
	void testMixedDirectionSelectionIntervals();
	void testVisualWordNavigationMixedDirection();
	void testVisualWordDeletionRangesMixedDirection();
	void testMouseHitTestingRespectsShapingClusters();
	void testUnicodeWordSelectionRanges();
	void testCombiningMarkCluster();
	void testExplicitParagraphDirection();
	void testFallbackFontRange();
	void testEditBoxTextReplacementLimits();
};

static TestArabicTextLayout g_test_instance;

void TestArabicTextLayout::runTests(IGameDef *gamedef)
{
	TEST(testArabicContextualShaping);
	TEST(testHarfBuzzOpenTypeArabicShaping);
	TEST(testLamAlefLigature);
	TEST(testArabicExtendedLetters);
	TEST(testMixedDirectionParagraph);
	TEST(testBidiIsolatesAndArabicIndicDigits);
	TEST(testMixedDirectionSelectionIntervals);
	TEST(testVisualWordNavigationMixedDirection);
	TEST(testVisualWordDeletionRangesMixedDirection);
	TEST(testMouseHitTestingRespectsShapingClusters);
	TEST(testUnicodeWordSelectionRanges);
	TEST(testCombiningMarkCluster);
	TEST(testExplicitParagraphDirection);
	TEST(testFallbackFontRange);
	TEST(testEditBoxTextReplacementLimits);
}

void TestArabicTextLayout::testArabicContextualShaping()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	const std::u32string text = U"بب";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, text.size()));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(layout.value),
			RAQM_DIRECTION_RTL);

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERTEQ(size_t, glyph_count, text.size());

	const FT_UInt isolated_beh = FT_Get_Char_Index(font.face, 0x0628);
	UASSERT(isolated_beh != 0);
	bool has_contextual_form = false;
	for (size_t i = 0; i < glyph_count; ++i) {
		UASSERT(glyphs[i].index != 0);
		UASSERTEQ(FT_Face, glyphs[i].ftface, font.face);
		if (glyphs[i].index != isolated_beh)
			has_contextual_form = true;
	}
	UASSERT(has_contextual_form);
}

void TestArabicTextLayout::testHarfBuzzOpenTypeArabicShaping()
{
	FreeTypeFont font;
	initializeFont(font);

	HarfBuzzObjects harfbuzz;
	harfbuzz.font = hb_ft_font_create_referenced(font.face);
	harfbuzz.buffer = hb_buffer_create();
	UASSERT(harfbuzz.font != nullptr);
	UASSERT(harfbuzz.buffer != nullptr);
	hb_ft_font_set_load_flags(harfbuzz.font, FT_LOAD_DEFAULT);

	// Check whether FreeType and HarfBuzz's FreeType adapter can both read the
	// font's OpenType substitution table.
	FT_ULong freetype_gsub_length = 0;
	const FT_Error freetype_gsub_error = FT_Load_Sfnt_Table(font.face,
			FT_MAKE_TAG('G', 'S', 'U', 'B'), 0, nullptr,
			&freetype_gsub_length);
	hb_blob_t *ft_gsub = hb_face_reference_table(
			hb_font_get_face(harfbuzz.font), HB_TAG('G', 'S', 'U', 'B'));
	const unsigned int ft_gsub_length = hb_blob_get_length(ft_gsub);
	hb_blob_destroy(ft_gsub);

	// Compare against HarfBuzz's direct OpenType face for the exact same file,
	// bypassing FreeType's table adapter.
	HarfBuzzFileObjects direct;
	const std::string font_path =
			porting::getDataPath("fonts/NotoSansArabic-Regular.ttf");
	UASSERT(!font_path.empty());
	direct.blob = hb_blob_create_from_file(font_path.c_str());
	UASSERT(direct.blob != nullptr);
	direct.face = hb_face_create(direct.blob, 0);
	UASSERT(direct.face != nullptr);
	direct.font = hb_font_create(direct.face);
	UASSERT(direct.font != nullptr);
	hb_ot_font_set_funcs(direct.font);
	const unsigned int upem = hb_face_get_upem(direct.face);
	UASSERT(upem != 0);
	hb_font_set_scale(direct.font, static_cast<int>(upem),
			static_cast<int>(upem));

	hb_blob_t *direct_gsub = hb_face_reference_table(
			direct.face, HB_TAG('G', 'S', 'U', 'B'));
	const unsigned int direct_gsub_length = hb_blob_get_length(direct_gsub);
	hb_blob_destroy(direct_gsub);

	std::cerr << "[Arabic shaping diagnostic] charmap="
			<< (font.face->charmap ? font.face->charmap->encoding : FT_Encoding(0))
			<< " num_charmaps=" << font.face->num_charmaps
			<< " glyph_count=" << font.face->num_glyphs
			<< " FT_Get_Char_Index(U+0628)="
			<< FT_Get_Char_Index(font.face, 0x0628)
			<< " FT_Load_Sfnt_Table(GSUB)=" << freetype_gsub_error
			<< " bytes=" << freetype_gsub_length
			<< " hb-ft GSUB bytes=" << ft_gsub_length
			<< " direct-HB GSUB bytes=" << direct_gsub_length
			<< " direct-HB upem=" << upem << std::endl;

	bool has_ot_shaper = false;
	const char **shapers = hb_shape_list_shapers();
	if (shapers) {
		for (const char **shaper = shapers; *shaper; ++shaper) {
			if (std::string(*shaper) == "ot") {
				has_ot_shaper = true;
				break;
			}
		}
	}
	UASSERT(has_ot_shaper);

	const auto shape_arabic = [&](hb_font_t *shape_font,
			const std::u32string &text) {
		const std::vector<uint32_t> codepoints(text.begin(), text.end());
		hb_buffer_reset(harfbuzz.buffer);
		hb_buffer_add_utf32(harfbuzz.buffer, codepoints.data(),
				static_cast<int>(codepoints.size()), 0,
				static_cast<int>(codepoints.size()));
		const unsigned int input_count = hb_buffer_get_length(harfbuzz.buffer);
		hb_buffer_set_direction(harfbuzz.buffer, HB_DIRECTION_RTL);
		hb_buffer_set_script(harfbuzz.buffer, HB_SCRIPT_ARABIC);
		hb_buffer_set_language(harfbuzz.buffer,
				hb_language_from_string("ar", -1));
		const bool shaped = hb_shape_full(shape_font, harfbuzz.buffer,
				nullptr, 0, nullptr) &&
				hb_buffer_allocation_successful(harfbuzz.buffer);
		const unsigned int output_count = hb_buffer_get_length(harfbuzz.buffer);
		const hb_glyph_info_t *glyphs =
				hb_buffer_get_glyph_infos(harfbuzz.buffer, nullptr);
		std::cerr << "[Arabic shaping diagnostic] input=" << codepoints.size()
				<< " buffered=" << input_count << " output=" << output_count
				<< " direction=" << hb_buffer_get_direction(harfbuzz.buffer)
				<< " script=" << hb_buffer_get_script(harfbuzz.buffer)
				<< " glyphs=";
		for (unsigned int i = 0; glyphs && i < output_count; ++i)
			std::cerr << glyphs[i].codepoint << '@' << glyphs[i].cluster << ' ';
		std::cerr << std::endl;
		return shaped;
	};

	const std::u32string contextual_text = U"بب";
	// Shape using HarfBuzz's native OpenType face first, then the hb-ft path
	// used by Raqm. The first failing assertion identifies the integration.
	UASSERT(shape_arabic(direct.font, contextual_text));
	UASSERTEQ(unsigned int, hb_buffer_get_length(harfbuzz.buffer),
			static_cast<unsigned int>(contextual_text.size()));

	UASSERT(shape_arabic(harfbuzz.font, contextual_text));
	UASSERTEQ(unsigned int, hb_buffer_get_length(harfbuzz.buffer),
			static_cast<unsigned int>(contextual_text.size()));

	const std::u32string lam_alef_text = U"لا";
	UASSERT(shape_arabic(direct.font, lam_alef_text));
	UASSERTEQ(unsigned int, hb_buffer_get_length(harfbuzz.buffer), 1);
	UASSERT(shape_arabic(harfbuzz.font, lam_alef_text));
	UASSERTEQ(unsigned int, hb_buffer_get_length(harfbuzz.buffer), 1);
}

void TestArabicTextLayout::testLamAlefLigature()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	const std::u32string text = U"لا";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, text.size()));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	// Noto Sans Arabic applies the mandatory lam-alef contextual ligature.
	UASSERTEQ(size_t, glyph_count, 1);
	UASSERT(glyphs[0].index != 0);
	UASSERTEQ(size_t, glyphs[0].cluster, 0);

	// Lam-alef is two Unicode graphemes, but one shaped glyph cluster. The
	// cursor must not stop between the two source characters.
	UASSERT(raqm_allowed_grapheme_boundary(layout.value, 0));
	const auto shaping_clusters =
			gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
	UASSERT(gui::text_layout::isAllowedCursorBoundary(
			layout.value, 0, text.size(), shaping_clusters));
	UASSERT(!gui::text_layout::isAllowedCursorBoundary(
			layout.value, 1, text.size(), shaping_clusters));
	UASSERT(gui::text_layout::isAllowedCursorBoundary(
			layout.value, 2, text.size(), shaping_clusters));

	// A selection of only the alef still colors the complete lam-alef glyph.
	const auto expanded = gui::text_layout::expandSelectionToShapingClusters(
			shaping_clusters, text.size(), 1, 2);
	UASSERTEQ(size_t, expanded.first, 0);
	UASSERTEQ(size_t, expanded.second, 2);
	const auto safe_expanded = gui::text_layout::expandSelectionToCursorBoundaries(
			layout.value, shaping_clusters, text.size(), 1, 2);
	UASSERTEQ(size_t, safe_expanded.first, 0);
	UASSERTEQ(size_t, safe_expanded.second, 2);
}

void TestArabicTextLayout::testArabicExtendedLetters()
{
	FreeTypeFont font;
	initializeFont(font);

	// Persian/extended Arabic letters must be covered by the bundled UI font.
	for (FT_ULong codepoint : {0x067E, 0x0686, 0x0698, 0x06AF, 0x06A4})
		UASSERT(FT_Get_Char_Index(font.face, codepoint) != 0);

	RaqmLayout layout;
	const std::u32string text = U"پچژگڤ";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, text.size()));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERT(glyph_count > 0);
	for (size_t i = 0; i < glyph_count; ++i) {
		UASSERT(glyphs[i].index != 0);
		UASSERT(glyphs[i].cluster < text.size());
	}
}

void TestArabicTextLayout::testMixedDirectionParagraph()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	const std::u32string text = U"مرحبا Luanti 123";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, 5));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(layout.value),
			RAQM_DIRECTION_RTL);

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERT(glyph_count > 0);

	// Raqm must expose a valid logical cluster and cursor mapping for mixed runs.
	for (size_t i = 0; i < glyph_count; ++i) {
		UASSERT(glyphs[i].index != 0);
		UASSERT(glyphs[i].cluster < text.size());
	}

	size_t cursor_index = 2;
	int x = 0;
	int y = 0;
	UASSERT(raqm_index_to_position(layout.value, &cursor_index, &x, &y));

	size_t hit_index = 0;
	UASSERT(raqm_position_to_index(layout.value, x, y, &hit_index));
	UASSERT(hit_index <= text.size());
}

void TestArabicTextLayout::testBidiIsolatesAndArabicIndicDigits()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	const std::u32string text = U"مرحبا \u2066Luanti 123\u2069 (٤٥٦)";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, 5));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(layout.value),
			RAQM_DIRECTION_RTL);
	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERT(glyph_count > 0);
	for (size_t i = 0; i < glyph_count; ++i)
		UASSERT(glyphs[i].cluster < text.size());

	// Bidi controls are default-ignorable and need not produce visible glyphs.
	// Insertion-point hit-testing must remain valid inside the isolated LTR run.
	size_t cursor_index = 8;
	int x = 0;
	int y = 0;
	UASSERT(raqm_index_to_position(layout.value, &cursor_index, &x, &y));
	size_t hit_index = 0;
	UASSERT(raqm_position_to_index(layout.value, x, y, &hit_index));
	UASSERT(hit_index <= text.size());
}

void TestArabicTextLayout::testMixedDirectionSelectionIntervals()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	// In an LTR paragraph the Arabic run is rendered in the opposite visual
	// order. Selecting the first Latin word and only the first logical Arabic
	// letter must therefore produce two separated visual highlight intervals.
	const std::u32string text = U"abc مرحبا def";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 4, 5));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));
	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(layout.value),
			RAQM_DIRECTION_LTR);

	const auto intervals = gui::text_layout::getVisualSelectionIntervals(
			layout.value, text.size(), 0, 5);
	UASSERT(intervals.size() >= 2);
	for (size_t i = 0; i < intervals.size(); ++i) {
		UASSERT(intervals[i].first < intervals[i].second);
		if (i > 0)
			UASSERT(intervals[i - 1].second < intervals[i].first);
	}
}

void TestArabicTextLayout::testVisualWordNavigationMixedDirection()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	const std::u32string text = U"abc مرحبا def";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 4, 5));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));
	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(layout.value),
			RAQM_DIRECTION_LTR);

	// Ctrl+Right from the first Latin word stops at its visual end.
	const size_t latin_end =
			gui::text_layout::getVisualWordCursorBoundary(layout.value, text, 0, 1);
	UASSERTEQ(size_t, latin_end, 3);

	// Word movement through the embedded RTL run must follow increasing or
	// decreasing visual x, not increasing/decreasing logical source indices.
	const auto cursor_x = [&](size_t boundary) {
		if (boundary == 0)
			return 0;
		size_t character = boundary - 1;
		int x = 0;
		int y = 0;
		UASSERT(raqm_index_to_position(layout.value, &character, &x, &y));
		return x;
	};

	const size_t from_rtl = 4;
	const int origin_x = cursor_x(from_rtl);
	const size_t visual_right =
			gui::text_layout::getVisualWordCursorBoundary(layout.value, text, from_rtl, 1);
	const size_t visual_left =
			gui::text_layout::getVisualWordCursorBoundary(layout.value, text, from_rtl, -1);
	UASSERT(visual_right != from_rtl);
	UASSERT(visual_left != from_rtl);
	UASSERT(cursor_x(visual_right) > origin_x);
	UASSERT(cursor_x(visual_left) < origin_x);
}

void TestArabicTextLayout::testVisualWordDeletionRangesMixedDirection()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	const std::u32string text = U"abc مرحبا def";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 4, 5));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));
	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(layout.value),
			RAQM_DIRECTION_LTR);

	const auto cursor_x = [&](size_t boundary) {
		if (boundary == 0)
			return 0;
		size_t character = boundary - 1;
		int x = 0;
		int y = 0;
		UASSERT(raqm_index_to_position(layout.value, &character, &x, &y));
		return x;
	};

	// Ctrl+Backspace and Ctrl+Delete use opposite visual directions. The
	// resulting logical source interval must stay valid even where visual
	// order makes the returned boundary fall before or after the source caret.
	const size_t cursor = 4;
	const auto clusters =
			gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
	for (int direction : {-1, 1}) {
		const size_t target = gui::text_layout::getVisualWordCursorBoundary(
				layout.value, text, cursor, direction);
		UASSERT(target != cursor);
		UASSERT(target <= text.size());

		const size_t source_start = std::min(cursor, target);
		const size_t source_end = std::max(cursor, target);
		UASSERT(source_start < source_end);
		UASSERT(gui::text_layout::isAllowedCursorBoundary(
				layout.value, source_start, text.size(), clusters));
		UASSERT(gui::text_layout::isAllowedCursorBoundary(
				layout.value, source_end, text.size(), clusters));

		if (direction < 0) {
			UASSERT(cursor_x(target) < cursor_x(cursor));
		} else {
			UASSERT(cursor_x(target) > cursor_x(cursor));
		}
	}
}


void TestArabicTextLayout::testMouseHitTestingRespectsShapingClusters()
{
	const auto verify_hits = [](RaqmLayout &layout, const std::u32string &text,
			size_t forbidden_boundary) {
		const auto clusters =
				gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
		size_t glyph_count = 0;
		raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
		UASSERT(glyphs != nullptr || glyph_count == 0);

		int pen = 0;
		int min_pen = 0;
		int max_pen = 0;
		for (size_t i = 0; i < glyph_count; ++i) {
			pen += glyphs[i].x_advance;
			min_pen = std::min(min_pen, pen);
			max_pen = std::max(max_pen, pen);
		}

		// Probe both sides and every quarter pixel across the shaped line.
		for (int x = -64; x <= max_pen - min_pen + 64; x += 16) {
			const size_t boundary =
					gui::text_layout::getCursorBoundaryFromPosition(
							layout.value, text.size(), x);
			UASSERT(boundary <= text.size());
			UASSERT(gui::text_layout::isAllowedCursorBoundary(
					layout.value, boundary, text.size(), clusters));
			UASSERT(boundary != forbidden_boundary);
		}
	};

	FreeTypeFont font;
	initializeFont(font);

	// Lam-alef is rendered as one glyph but contains two Unicode scalars.
	RaqmLayout ligature;
	const std::u32string lam_alef = U"لا";
	setRaqmText(ligature, font.face, lam_alef);
	UASSERT(raqm_set_language(ligature.value, "ar", 0, lam_alef.size()));
	UASSERT(raqm_set_par_direction(ligature.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(ligature.value));
	const auto ligature_clusters =
			gui::text_layout::getShapingClusterBoundaries(
					ligature.value, lam_alef.size());
	UASSERT(!gui::text_layout::isAllowedCursorBoundary(
				ligature.value, 1, lam_alef.size(), ligature_clusters));
	verify_hits(ligature, lam_alef, 1);

	// A combining shadda must stay attached to its base letter during hit tests.
	RaqmLayout combining;
	const std::u32string combining_text = U"ا\u0651A";
	setRaqmText(combining, font.face, combining_text);
	UASSERT(raqm_set_language(combining.value, "ar", 0, 2));
	UASSERT(raqm_set_par_direction(combining.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(combining.value));
	const auto combining_clusters =
			gui::text_layout::getShapingClusterBoundaries(
					combining.value, combining_text.size());
	UASSERT(!gui::text_layout::isAllowedCursorBoundary(
				combining.value, 1, combining_text.size(), combining_clusters));
	verify_hits(combining, combining_text, 1);

	// Mixed-direction hit testing must still return logical, safe source indices.
	RaqmLayout mixed;
	const std::u32string mixed_text = U"abc مرحبا def";
	setRaqmText(mixed, font.face, mixed_text);
	UASSERT(raqm_set_language(mixed.value, "ar", 4, 5));
	UASSERT(raqm_set_par_direction(mixed.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(mixed.value));
	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(mixed.value),
			RAQM_DIRECTION_LTR);
	verify_hits(mixed, mixed_text, mixed_text.size() + 1);
}

void TestArabicTextLayout::testUnicodeWordSelectionRanges()
{
	// Arabic letters must remain word characters regardless of the C locale.
	UASSERT(!gui::text_layout::isUnicodeWordPunctuation(U'م'));
	UASSERT(!gui::text_layout::isUnicodeWordPunctuation(U'ح'));
	UASSERT(gui::text_layout::isUnicodeWordPunctuation(U'،'));
	UASSERT(gui::text_layout::isUnicodeWordSpace(U'\t'));
	UASSERT(gui::text_layout::isUnicodeWordSpace(U'\u00A0'));
	const std::u32string mixed_text = U"abc مرحبا def";
	const auto arabic_word =
			gui::text_layout::getLogicalWordSelectionRange(mixed_text, 6);
	UASSERTEQ(size_t, arabic_word.first, 4);
	UASSERTEQ(size_t, arabic_word.second, 9);

	const std::u32string punctuation_text = U"مرحبا، عالم";
	const auto comma = gui::text_layout::getLogicalWordSelectionRange(
			punctuation_text, 5);
	UASSERTEQ(size_t, comma.first, 5);
	UASSERTEQ(size_t, comma.second, 6);

	const std::u32string whitespace_text = U"abc   def";
	const auto whitespace = gui::text_layout::getLogicalWordSelectionRange(
			whitespace_text, 4);
	UASSERTEQ(size_t, whitespace.first, 3);
	UASSERTEQ(size_t, whitespace.second, 6);
}

void TestArabicTextLayout::testCombiningMarkCluster()
{
	FreeTypeFont font;
	initializeFont(font);

	RaqmLayout layout;
	const std::u32string text = U"ا\u0651A";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, 2));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	// UAX #29 keeps the Arabic shadda attached to its base letter.
	UASSERT(!raqm_allowed_grapheme_boundary(layout.value, 0));
	UASSERT(raqm_allowed_grapheme_boundary(layout.value, 1));
	UASSERT(raqm_allowed_grapheme_boundary(layout.value, 2));
	const auto shaping_clusters =
			gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
	UASSERT(!gui::text_layout::isAllowedCursorBoundary(
			layout.value, 1, text.size(), shaping_clusters));
	UASSERT(gui::text_layout::isAllowedCursorBoundary(
			layout.value, 2, text.size(), shaping_clusters));
	const auto safe_expanded = gui::text_layout::expandSelectionToCursorBoundaries(
			layout.value, shaping_clusters, text.size(), 1, 2);
	UASSERTEQ(size_t, safe_expanded.first, 0);
	UASSERTEQ(size_t, safe_expanded.second, 2);
}

void TestArabicTextLayout::testExplicitParagraphDirection()
{
	FreeTypeFont font;
	initializeFont(font);

	const std::u32string text = U"Luanti مرحبا";

	RaqmLayout rtl;
	setRaqmText(rtl, font.face, text);
	UASSERT(raqm_set_language(rtl.value, "ar", 7, text.size() - 7));
	UASSERT(raqm_set_par_direction(rtl.value, RAQM_DIRECTION_RTL));
	UASSERT(raqm_layout(rtl.value));
	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(rtl.value),
			RAQM_DIRECTION_RTL);

	RaqmLayout ltr;
	setRaqmText(ltr, font.face, text);
	UASSERT(raqm_set_language(ltr.value, "ar", 7, text.size() - 7));
	UASSERT(raqm_set_par_direction(ltr.value, RAQM_DIRECTION_LTR));
	UASSERT(raqm_layout(ltr.value));
	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(ltr.value),
			RAQM_DIRECTION_LTR);
}

void TestArabicTextLayout::testFallbackFontRange()
{
	FreeTypeFont arabic_font;
	FreeTypeFont fallback_font;
	initializeFont(arabic_font);
	initializeFont(fallback_font, "fonts/DroidSansFallbackFull.ttf");

	const std::u32string text = U"مرحبا 世界";
	RaqmLayout layout;
	setRaqmText(layout, arabic_font.face, text);
	UASSERT(raqm_set_freetype_face_range(layout.value, fallback_font.face, 6, 2));
	UASSERT(raqm_set_language(layout.value, "ar", 0, 5));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_RTL));
	UASSERT(raqm_layout(layout.value));

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	bool found_fallback = false;
	bool found_primary = false;
	for (size_t i = 0; i < glyph_count; ++i) {
		UASSERT(glyphs[i].index != 0);
		if (glyphs[i].ftface == fallback_font.face)
			found_fallback = true;
		if (glyphs[i].ftface == arabic_font.face)
			found_primary = true;
	}
	UASSERT(found_primary);
	UASSERT(found_fallback);
}



void TestArabicTextLayout::testEditBoxTextReplacementLimits()
{
	using gui::text_edit::buildEditTextReplacement;

	// Replacing a selected range inserts at its logical start and leaves the
	// unselected suffix intact.
	const core::stringw selected_text = L"abcDEFghi";
	core::stringw replacement;
	s32 cursor = -1;
	UASSERT(buildEditTextReplacement(selected_text, 6, 3, 6,
			core::stringw(L"x"), false, 0, replacement, cursor));
	UASSERT(replacement == core::stringw(L"abcxghi"));
	UASSERTEQ(s32, cursor, 4);

	// Reject an over-limit replacement without publishing a partial result.
	replacement = L"unchanged";
	UASSERT(!buildEditTextReplacement(selected_text, 6, 3, 6,
			core::stringw(L"long"), false, 8, replacement, cursor));
	UASSERT(replacement == core::stringw(L"unchanged"));

	// Overwrite mode must never consume a newline while replacing text.
	const core::stringw multiline_text = L"ab\ncd";
	UASSERT(buildEditTextReplacement(multiline_text, 1, 1, 1,
			core::stringw(L"XYZ"), true, 0, replacement, cursor));
	UASSERT(replacement == core::stringw(L"aXYZ\ncd"));
	UASSERTEQ(s32, cursor, 4);

	// Stale cursor/selection indices are clamped before applying replacement.
	const core::stringw short_text = L"abc";
	UASSERT(buildEditTextReplacement(short_text, 99, -10, 99,
			core::stringw(L"Z"), false, 0, replacement, cursor));
	UASSERT(replacement == core::stringw(L"Z"));
	UASSERTEQ(s32, cursor, 1);

	// Empty clipboard/input strings are rejected and leave the output intact.
	replacement = L"unchanged";
	UASSERT(!buildEditTextReplacement(short_text, 1, 0, 0,
			core::stringw(), false, 0, replacement, cursor));
	UASSERT(replacement == core::stringw(L"unchanged"));
}

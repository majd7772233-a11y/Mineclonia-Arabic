// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Regression tests for native Unicode shaping and bidirectional layout.

#include "test.h"

#include "porting.h"
#include "../irrlicht_changes/font_text_layout.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <raqm.h>

#include <cstdint>
#include <string>

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
	UASSERT(raqm_set_text(layout.value,
			reinterpret_cast<const uint32_t *>(text.data()), text.size()));
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
	void testLamAlefLigature();
	void testArabicExtendedLetters();
	void testMixedDirectionParagraph();
	void testBidiIsolatesAndArabicIndicDigits();
	void testMixedDirectionSelectionIntervals();
	void testVisualWordNavigationMixedDirection();
	void testVisualWordDeletionRangesMixedDirection();
	void testCombiningMarkCluster();
	void testExplicitParagraphDirection();
	void testFallbackFontRange();
};

static TestArabicTextLayout g_test_instance;

void TestArabicTextLayout::runTests(IGameDef *gamedef)
{
	TEST(testArabicContextualShaping);
	TEST(testLamAlefLigature);
	TEST(testArabicExtendedLetters);
	TEST(testMixedDirectionParagraph);
	TEST(testBidiIsolatesAndArabicIndicDigits);
	TEST(testMixedDirectionSelectionIntervals);
	TEST(testVisualWordNavigationMixedDirection);
	TEST(testVisualWordDeletionRangesMixedDirection);
	TEST(testCombiningMarkCluster);
	TEST(testExplicitParagraphDirection);
	TEST(testFallbackFontRange);
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

		if (direction < 0)
			UASSERT(cursor_x(target) < cursor_x(cursor));
		else
			UASSERT(cursor_x(target) > cursor_x(cursor));
	}
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


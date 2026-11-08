// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Regression tests for native Unicode shaping and bidirectional layout.

#include "test.h"

#include "porting.h"

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
	void testMixedDirectionParagraph();
	void testCombiningMarkCluster();
	void testExplicitParagraphDirection();
	void testFallbackFontRange();
};

static TestArabicTextLayout g_test_instance;

void TestArabicTextLayout::runTests(IGameDef *gamedef)
{
	TEST(testArabicContextualShaping);
	TEST(testMixedDirectionParagraph);
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


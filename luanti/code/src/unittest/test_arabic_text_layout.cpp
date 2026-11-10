// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Regression tests for native Unicode shaping and bidirectional layout.

#include "test.h"
#include "builtin_files.h"

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

#include <algorithm>
#include <cstdint>
#include <cstdlib>
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

std::string getGlyphName(FT_Face face, FT_UInt glyph_index)
{
	if (!face || !FT_HAS_GLYPH_NAMES(face))
		return {};

	char name[128] = {};
	if (FT_Get_Glyph_Name(face, glyph_index, name,
			static_cast<FT_UInt>(sizeof(name))) != 0)
		return {};
	return name;
}

size_t countGlyphName(const std::vector<std::string> &glyph_names,
		const char *expected_name)
{
	return static_cast<size_t>(std::count(glyph_names.begin(),
			glyph_names.end(), expected_name));
}

struct HarfBuzzShapeResult
{
	bool success = false;
	std::vector<std::string> glyph_names;
	std::vector<unsigned int> clusters;
};

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
	void testLamAlefPreservesSeparateClusters();
	void testArabicAllahLigatureCluster();
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
	void testBuiltinFileSha256Manifest();
};

static TestArabicTextLayout g_test_instance;

void TestArabicTextLayout::runTests(IGameDef *gamedef)
{
	TEST(testArabicContextualShaping);
	TEST(testHarfBuzzOpenTypeArabicShaping);
	TEST(testLamAlefPreservesSeparateClusters);
	TEST(testArabicAllahLigatureCluster);
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
	TEST(testBuiltinFileSha256Manifest);
}

void TestArabicTextLayout::testArabicContextualShaping()
{
	FreeTypeFont font;
	initializeFont(font);
	UASSERT(FT_HAS_GLYPH_NAMES(font.face));

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

	// Noto Sans Arabic decomposes each Beh into a contextual base and a
	// separately positioned dot-below mark. Four glyphs are correct for
	// two source characters; verify the actual OpenType glyph identities.
	std::vector<std::string> glyph_names;
	glyph_names.reserve(glyph_count);
	for (size_t i = 0; i < glyph_count; ++i) {
		UASSERT(glyphs[i].index != 0);
		UASSERTEQ(FT_Face, glyphs[i].ftface, font.face);
		glyph_names.push_back(getGlyphName(glyphs[i].ftface, glyphs[i].index));
		UASSERT(!glyph_names.back().empty());
	}
	UASSERTEQ(size_t, glyph_count, 4);
	UASSERTEQ(size_t, countGlyphName(glyph_names, "dotbelowar"), 2);
	UASSERTEQ(size_t, countGlyphName(glyph_names, "uni066E.init"), 1);
	UASSERTEQ(size_t, countGlyphName(glyph_names, "uni066E.fina"), 1);
}

void TestArabicTextLayout::testHarfBuzzOpenTypeArabicShaping()
{
	FreeTypeFont font;
	initializeFont(font);
	UASSERT(FT_HAS_GLYPH_NAMES(font.face));

	HarfBuzzObjects harfbuzz;
	harfbuzz.font = hb_ft_font_create_referenced(font.face);
	harfbuzz.buffer = hb_buffer_create();
	UASSERT(harfbuzz.font != nullptr);
	UASSERT(harfbuzz.buffer != nullptr);
	hb_ft_font_set_load_flags(harfbuzz.font, FT_LOAD_DEFAULT);

	// Compare the FreeType adapter against HarfBuzz's direct OpenType face
	// using the same bundled font and report the exact GSUB script/features.
	const std::string font_path =
			porting::getDataPath("fonts/NotoSansArabic-Regular.ttf");
	UASSERT(!font_path.empty());
	HarfBuzzFileObjects direct;
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

	FT_ULong freetype_gsub_length = 0;
	const FT_Error freetype_gsub_error = FT_Load_Sfnt_Table(font.face,
			FT_MAKE_TAG('G', 'S', 'U', 'B'), 0, nullptr,
			&freetype_gsub_length);
	hb_blob_t *ft_gsub = hb_face_reference_table(
			hb_font_get_face(harfbuzz.font), HB_TAG('G', 'S', 'U', 'B'));
	const unsigned int ft_gsub_length = hb_blob_get_length(ft_gsub);
	hb_blob_destroy(ft_gsub);
	hb_blob_t *direct_gsub = hb_face_reference_table(
			direct.face, HB_TAG('G', 'S', 'U', 'B'));
	const unsigned int direct_gsub_length = hb_blob_get_length(direct_gsub);
	hb_blob_destroy(direct_gsub);

	hb_codepoint_t direct_nominal_beh = 0;
	const bool direct_has_nominal_beh = hb_font_get_nominal_glyph(
			direct.font, 0x0628, &direct_nominal_beh);
	unsigned int arabic_gsub_script_index = HB_OT_LAYOUT_NO_SCRIPT_INDEX;
	const bool has_arabic_gsub_script = hb_ot_layout_table_find_script(
			direct.face, HB_OT_TAG_GSUB, HB_TAG('a', 'r', 'a', 'b'),
			&arabic_gsub_script_index);
	unsigned int default_feature_capacity = 0;
	const unsigned int total_default_features = has_arabic_gsub_script
			? hb_ot_layout_language_get_feature_tags(direct.face,
					HB_OT_TAG_GSUB, arabic_gsub_script_index,
					HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX, 0,
					&default_feature_capacity, nullptr)
			: 0;
	std::vector<hb_tag_t> default_features(total_default_features);
	if (total_default_features) {
		default_feature_capacity = total_default_features;
		hb_ot_layout_language_get_feature_tags(direct.face, HB_OT_TAG_GSUB,
				arabic_gsub_script_index,
				HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX, 0,
				&default_feature_capacity, default_features.data());
		default_features.resize(default_feature_capacity);
	}
	const auto has_feature = [&](hb_tag_t wanted) {
		return std::find(default_features.begin(), default_features.end(),
				wanted) != default_features.end();
	};
	UASSERT(has_feature(HB_TAG('i', 'n', 'i', 't')));
	UASSERT(has_feature(HB_TAG('f', 'i', 'n', 'a')));
	UASSERT(has_feature(HB_TAG('l', 'i', 'g', 'a')));
	const auto format_tags = [](const std::vector<hb_tag_t> &tags) {
		std::string result;
		for (hb_tag_t tag : tags) {
			char buffer[5] = {};
			hb_tag_to_string(tag, buffer);
			if (!result.empty())
				result += ',';
			result += buffer;
		}
		return result;
	};

	// Ask HarfBuzz which shaper its default plan really selects for Arabic.
	hb_buffer_t *plan_buffer = hb_buffer_create();
	const std::vector<uint32_t> plan_codepoints = {0x0628, 0x0628};
	hb_buffer_add_utf32(plan_buffer, plan_codepoints.data(),
			static_cast<int>(plan_codepoints.size()), 0,
			static_cast<int>(plan_codepoints.size()));
	hb_buffer_set_direction(plan_buffer, HB_DIRECTION_RTL);
	hb_buffer_set_script(plan_buffer, HB_SCRIPT_ARABIC);
	hb_buffer_set_language(plan_buffer, hb_language_from_string("ar", -1));
	hb_segment_properties_t plan_properties{};
	hb_buffer_get_segment_properties(plan_buffer, &plan_properties);
	hb_shape_plan_t *default_plan = hb_shape_plan_create(direct.face,
			&plan_properties, nullptr, 0, nullptr);
	const char *selected_shaper = default_plan
			? hb_shape_plan_get_shaper(default_plan) : nullptr;
	const char *shaper_environment = std::getenv("HB_SHAPER_LIST");

	std::cerr << "[Arabic shaping diagnostic] font_path=" << font_path
			<< " family=" << (font.face->family_name ?
					font.face->family_name : "<null>")
			<< " style=" << (font.face->style_name ?
					font.face->style_name : "<null>")
			<< " hb_version=" << hb_version_string()
			<< " HB_SHAPER_LIST="
			<< (shaper_environment ? shaper_environment : "<unset>")
			<< " default_plan_shaper="
			<< (selected_shaper ? selected_shaper : "<none>")
			<< " charmap=" << (font.face->charmap ?
					font.face->charmap->encoding : FT_Encoding(0))
			<< " num_charmaps=" << font.face->num_charmaps
			<< " glyph_count=" << font.face->num_glyphs
			<< " FT_Get_Char_Index(U+0628)="
			<< FT_Get_Char_Index(font.face, 0x0628)
			<< " direct-HB-nominal(U+0628)="
			<< (direct_has_nominal_beh ? direct_nominal_beh : 0)
			<< " FT_GSUB_error=" << freetype_gsub_error
			<< " FT_GSUB_bytes=" << freetype_gsub_length
			<< " hb-ft_GSUB_bytes=" << ft_gsub_length
			<< " direct-HB_GSUB_bytes=" << direct_gsub_length
			<< " direct-HB_upem=" << upem
			<< " arab_GSUB_script=" << has_arabic_gsub_script
			<< " arab_default_features=" << format_tags(default_features)
			<< std::endl;

	if (default_plan)
		hb_shape_plan_destroy(default_plan);
	hb_buffer_destroy(plan_buffer);

	bool has_ot_shaper = false;
	std::string available_shapers;
	const char **shapers = hb_shape_list_shapers();
	if (shapers) {
		for (const char **shaper = shapers; *shaper; ++shaper) {
			if (!available_shapers.empty())
				available_shapers += ',';
			available_shapers += *shaper;
			if (std::string(*shaper) == "ot")
				has_ot_shaper = true;
		}
	}
	std::cerr << "[Arabic shaping diagnostic] available_shapers="
			<< available_shapers << std::endl;
	UASSERT(has_ot_shaper);
	UASSERT(direct_has_nominal_beh);
	UASSERT(has_arabic_gsub_script);

	const auto shape_arabic = [&](hb_font_t *shape_font,
			const std::u32string &text, const char *const *shaper_list,
			const char *label) {
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

		HarfBuzzShapeResult result;
		result.success = hb_shape_full(shape_font, harfbuzz.buffer,
				nullptr, 0, shaper_list) &&
				hb_buffer_allocation_successful(harfbuzz.buffer);
		const unsigned int output_count = hb_buffer_get_length(harfbuzz.buffer);
		const hb_glyph_info_t *glyphs =
				hb_buffer_get_glyph_infos(harfbuzz.buffer, nullptr);
		std::cerr << "[Arabic shaping diagnostic] case=" << label
				<< " input=" << codepoints.size()
				<< " buffered=" << input_count << " output=" << output_count
				<< " direction=" << hb_buffer_get_direction(harfbuzz.buffer)
				<< " script=" << hb_buffer_get_script(harfbuzz.buffer)
				<< " language=" << hb_language_to_string(
						hb_buffer_get_language(harfbuzz.buffer))
				<< " glyphs=";
		for (unsigned int i = 0; glyphs && i < output_count; ++i) {
			const std::string name = getGlyphName(font.face, glyphs[i].codepoint);
			result.glyph_names.push_back(name);
			result.clusters.push_back(glyphs[i].cluster);
			std::cerr << glyphs[i].codepoint << '[' << name << "]@"
					<< glyphs[i].cluster << ' ';
		}
		std::cerr << std::endl;
		return result;
	};

	const auto verify_contextual_beh = [&](const HarfBuzzShapeResult &result) {
		UASSERT(result.success);
		UASSERTEQ(size_t, result.glyph_names.size(), 4);
		UASSERTEQ(size_t, countGlyphName(result.glyph_names, "dotbelowar"), 2);
		UASSERTEQ(size_t, countGlyphName(result.glyph_names, "uni066E.init"), 1);
		UASSERTEQ(size_t, countGlyphName(result.glyph_names, "uni066E.fina"), 1);
	};
	const auto verify_separate_lam_alef = [&](const HarfBuzzShapeResult &result) {
		UASSERT(result.success);
		UASSERTEQ(size_t, result.glyph_names.size(), 2);
		UASSERTEQ(size_t, countGlyphName(result.glyph_names,
				"uni0644.init.rlig"), 1);
		UASSERTEQ(size_t, countGlyphName(result.glyph_names,
				"uni0627.fina.rlig"), 1);
	};
	const auto verify_allah_ligature = [&](const HarfBuzzShapeResult &result) {
		UASSERT(result.success);
		UASSERTEQ(size_t, result.glyph_names.size(), 1);
		UASSERTEQ(size_t, countGlyphName(result.glyph_names, "uniFDF2"), 1);
		UASSERTEQ(size_t, result.clusters.size(), 1);
		UASSERTEQ(unsigned int, result.clusters[0], 0);
	};

	static const char *const ot_shaper[] = {"ot", nullptr};
	const std::u32string contextual_text = U"بب";
	const HarfBuzzShapeResult direct_default_contextual = shape_arabic(
			direct.font, contextual_text, nullptr, "direct-default-contextual");
	const HarfBuzzShapeResult direct_ot_contextual = shape_arabic(
			direct.font, contextual_text, ot_shaper, "direct-ot-contextual");
	const HarfBuzzShapeResult hb_ft_contextual = shape_arabic(
			harfbuzz.font, contextual_text, nullptr, "hb-ft-default-contextual");

	const std::u32string lam_alef_text = U"لا";
	const HarfBuzzShapeResult direct_default_lam_alef = shape_arabic(
			direct.font, lam_alef_text, nullptr, "direct-default-lam-alef");
	const HarfBuzzShapeResult direct_ot_lam_alef = shape_arabic(
			direct.font, lam_alef_text, ot_shaper, "direct-ot-lam-alef");
	const HarfBuzzShapeResult hb_ft_lam_alef = shape_arabic(
			harfbuzz.font, lam_alef_text, nullptr, "hb-ft-default-lam-alef");

	const std::u32string supported_ligature_text = U"اللّٰه";
	const HarfBuzzShapeResult direct_default_ligature = shape_arabic(
			direct.font, supported_ligature_text, nullptr,
			"direct-default-supported-ligature");
	const HarfBuzzShapeResult direct_ot_ligature = shape_arabic(
			direct.font, supported_ligature_text, ot_shaper,
			"direct-ot-supported-ligature");
	const HarfBuzzShapeResult hb_ft_ligature = shape_arabic(
			harfbuzz.font, supported_ligature_text, nullptr,
			"hb-ft-default-supported-ligature");

	verify_contextual_beh(direct_default_contextual);
	verify_contextual_beh(direct_ot_contextual);
	verify_contextual_beh(hb_ft_contextual);
	verify_separate_lam_alef(direct_default_lam_alef);
	verify_separate_lam_alef(direct_ot_lam_alef);
	verify_separate_lam_alef(hb_ft_lam_alef);
	verify_allah_ligature(direct_default_ligature);
	verify_allah_ligature(direct_ot_ligature);
	verify_allah_ligature(hb_ft_ligature);
}

void TestArabicTextLayout::testLamAlefPreservesSeparateClusters()
{
	FreeTypeFont font;
	initializeFont(font);
	UASSERT(FT_HAS_GLYPH_NAMES(font.face));

	RaqmLayout layout;
	const std::u32string text = U"لا";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, text.size()));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERTEQ(size_t, glyph_count, 2);

	std::vector<std::string> glyph_names;
	glyph_names.reserve(glyph_count);
	for (size_t i = 0; i < glyph_count; ++i) {
		glyph_names.push_back(getGlyphName(glyphs[i].ftface, glyphs[i].index));
		UASSERT(!glyph_names.back().empty());
	}
	UASSERTEQ(size_t, countGlyphName(glyph_names, "uni0644.init.rlig"), 1);
	UASSERTEQ(size_t, countGlyphName(glyph_names, "uni0627.fina.rlig"), 1);

	const auto clusters =
			gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
	for (size_t boundary = 0; boundary <= text.size(); ++boundary) {
		UASSERT(gui::text_layout::isAllowedCursorBoundary(
				layout.value, boundary, text.size(), clusters));
	}

	const auto expanded = gui::text_layout::expandSelectionToShapingClusters(
			clusters, text.size(), 1, 2);
	UASSERTEQ(size_t, expanded.first, 1);
	UASSERTEQ(size_t, expanded.second, 2);
	const auto safe_expanded = gui::text_layout::expandSelectionToCursorBoundaries(
			layout.value, clusters, text.size(), 1, 2);
	UASSERTEQ(size_t, safe_expanded.first, 1);
	UASSERTEQ(size_t, safe_expanded.second, 2);
}

void TestArabicTextLayout::testArabicAllahLigatureCluster()
{
	FreeTypeFont font;
	initializeFont(font);
	UASSERT(FT_HAS_GLYPH_NAMES(font.face));

	RaqmLayout layout;
	const std::u32string text = U"اللّٰه";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, text.size()));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERTEQ(size_t, glyph_count, 1);
	UASSERTEQ(size_t, glyphs[0].cluster, 0);
	UASSERTEQ(std::string, getGlyphName(glyphs[0].ftface, glyphs[0].index),
			"uniFDF2");

	const auto clusters =
			gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
	for (size_t boundary = 1; boundary < text.size(); ++boundary) {
		UASSERT(!gui::text_layout::isAllowedCursorBoundary(
				layout.value, boundary, text.size(), clusters));
	}
	UASSERT(gui::text_layout::isAllowedCursorBoundary(
			layout.value, 0, text.size(), clusters));
	UASSERT(gui::text_layout::isAllowedCursorBoundary(
			layout.value, text.size(), text.size(), clusters));

	const auto expanded = gui::text_layout::expandSelectionToShapingClusters(
			clusters, text.size(), 2, 3);
	UASSERTEQ(size_t, expanded.first, 0);
	UASSERTEQ(size_t, expanded.second, text.size());
	const auto safe_expanded = gui::text_layout::expandSelectionToCursorBoundaries(
			layout.value, clusters, text.size(), 2, 3);
	UASSERTEQ(size_t, safe_expanded.first, 0);
	UASSERTEQ(size_t, safe_expanded.second, text.size());
}
void TestArabicTextLayout::testLamAlefPreservesSeparateClusters()
{
	FreeTypeFont font;
	initializeFont(font);
	UASSERT(FT_HAS_GLYPH_NAMES(font.face));

	RaqmLayout layout;
	const std::u32string text = U"لا";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, text.size()));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERTEQ(size_t, glyph_count, 2);

	std::vector<std::string> glyph_names;
	glyph_names.reserve(glyph_count);
	for (size_t i = 0; i < glyph_count; ++i) {
		glyph_names.push_back(getGlyphName(glyphs[i].ftface, glyphs[i].index));
		UASSERT(!glyph_names.back().empty());
	}
	UASSERTEQ(size_t, countGlyphName(glyph_names, "uni0644.init.rlig"), 1);
	UASSERTEQ(size_t, countGlyphName(glyph_names, "uni0627.fina.rlig"), 1);

	const auto clusters =
			gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
	for (size_t boundary = 0; boundary <= text.size(); ++boundary) {
		UASSERT(gui::text_layout::isAllowedCursorBoundary(
				layout.value, boundary, text.size(), clusters));
	}

	const auto expanded = gui::text_layout::expandSelectionToShapingClusters(
			clusters, text.size(), 1, 2);
	UASSERTEQ(size_t, expanded.first, 1);
	UASSERTEQ(size_t, expanded.second, 2);
	const auto safe_expanded = gui::text_layout::expandSelectionToCursorBoundaries(
			layout.value, clusters, text.size(), 1, 2);
	UASSERTEQ(size_t, safe_expanded.first, 1);
	UASSERTEQ(size_t, safe_expanded.second, 2);
}

void TestArabicTextLayout::testArabicAllahLigatureCluster()
{
	FreeTypeFont font;
	initializeFont(font);
	UASSERT(FT_HAS_GLYPH_NAMES(font.face));

	RaqmLayout layout;
	const std::u32string text = U"اللّٰه";
	setRaqmText(layout, font.face, text);
	UASSERT(raqm_set_language(layout.value, "ar", 0, text.size()));
	UASSERT(raqm_set_par_direction(layout.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(layout.value));

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout.value, &glyph_count);
	UASSERT(glyphs != nullptr);
	UASSERTEQ(size_t, glyph_count, 1);
	UASSERTEQ(size_t, glyphs[0].cluster, 0);
	UASSERTEQ(std::string, getGlyphName(glyphs[0].ftface, glyphs[0].index),
			"uniFDF2");

	const auto clusters =
			gui::text_layout::getShapingClusterBoundaries(layout.value, text.size());
	for (size_t boundary = 1; boundary < text.size(); ++boundary) {
		UASSERT(!gui::text_layout::isAllowedCursorBoundary(
				layout.value, boundary, text.size(), clusters));
	}
	UASSERT(gui::text_layout::isAllowedCursorBoundary(
			layout.value, 0, text.size(), clusters));
	UASSERT(gui::text_layout::isAllowedCursorBoundary(
			layout.value, text.size(), text.size(), clusters));

	const auto expanded = gui::text_layout::expandSelectionToShapingClusters(
			clusters, text.size(), 2, 3);
	UASSERTEQ(size_t, expanded.first, 0);
	UASSERTEQ(size_t, expanded.second, text.size());
	const auto safe_expanded = gui::text_layout::expandSelectionToCursorBoundaries(
			layout.value, clusters, text.size(), 2, 3);
	UASSERTEQ(size_t, safe_expanded.first, 0);
	UASSERTEQ(size_t, safe_expanded.second, text.size());
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
			const std::vector<size_t> &forbidden_boundaries) {
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
			for (size_t forbidden : forbidden_boundaries)
				UASSERT(boundary != forbidden);
		}
	};

	FreeTypeFont font;
	initializeFont(font);

	// Noto Sans Arabic has a real six-codepoint Allah ligature (uniFDF2).
	// Mouse hit testing must never return a caret inside that single glyph cluster.
	RaqmLayout ligature;
	const std::u32string allah_ligature = U"اللّٰه";
	setRaqmText(ligature, font.face, allah_ligature);
	UASSERT(raqm_set_language(ligature.value, "ar", 0, allah_ligature.size()));
	UASSERT(raqm_set_par_direction(ligature.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(ligature.value));
	std::vector<size_t> forbidden_ligature_boundaries;
	for (size_t boundary = 1; boundary < allah_ligature.size(); ++boundary)
		forbidden_ligature_boundaries.push_back(boundary);
	verify_hits(ligature, allah_ligature, forbidden_ligature_boundaries);

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
	verify_hits(combining, combining_text, {1});

	// Mixed-direction hit testing must still return logical, safe source indices.
	RaqmLayout mixed;
	const std::u32string mixed_text = U"abc مرحبا def";
	setRaqmText(mixed, font.face, mixed_text);
	UASSERT(raqm_set_language(mixed.value, "ar", 4, 5));
	UASSERT(raqm_set_par_direction(mixed.value, RAQM_DIRECTION_DEFAULT));
	UASSERT(raqm_layout(mixed.value));
	UASSERTEQ(raqm_direction_t, raqm_get_par_resolved_direction(mixed.value),
			RAQM_DIRECTION_LTR);
	verify_hits(mixed, mixed_text, {});
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


void TestArabicTextLayout::testBuiltinFileSha256Manifest()
{
	// These files are loaded by the main menu during startup. Their SHA-256
	// entries must be present in the build-generated builtin manifest.
	static constexpr const char *required_files[] = {
		"common/math.lua",
		"common/vector.lua",
		"common/settings/dlg_settings.lua",
		"mainmenu/init.lua",
	};

	for (const char *relative_path : required_files) {
		const auto entry = g_builtin_file_sha256_map.find(relative_path);
		UASSERT(entry != g_builtin_file_sha256_map.end());
		UASSERTEQ(size_t, entry->second.size(), 64);
		for (const char digit : entry->second) {
			UASSERT((digit >= '0' && digit <= '9') ||
					(digit >= 'a' && digit <= 'f') ||
					(digit >= 'A' && digit <= 'F'));
		}
	}

	// Runtime paths are native on Windows, while generated manifest keys
	// always use forward slashes.
	const std::string posix_path =
			normalizeBuiltinFileSha256Path("common/math.lua");
	const std::string windows_path =
			normalizeBuiltinFileSha256Path("common\\math.lua");
	const std::string mixed_path =
			normalizeBuiltinFileSha256Path("common/settings\\dlg_settings.lua");
	UASSERTEQ(std::string, windows_path, posix_path);
	UASSERTEQ(std::string, mixed_path, "common/settings/dlg_settings.lua");
	UASSERT(g_builtin_file_sha256_map.find(windows_path) !=
			g_builtin_file_sha256_map.end());
	UASSERT(g_builtin_file_sha256_map.find(mixed_path) !=
			g_builtin_file_sha256_map.end());
}

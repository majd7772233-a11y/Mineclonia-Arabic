/*
   CGUITTFont FreeType class for Irrlicht
   Copyright (c) 2009-2010 John Norman
   with changes from Luanti contributors:
   Copyright (c) 2016 Nathanaëlle Courant
   Copyright (c) 2023 Caleb Butler

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this class can be located at:
   http://irrlicht.suckerfreegames.com/

   John Norman
   john@suckerfreegames.com
*/

#include "CGUITTFont.h"
#include "font_text_layout.h"

#include "log.h"
#include "debug.h"
#include "util/basic_macros.h"
#include "util/enriched_string.h"

#include "IGUIEnvironment.h"
#include "IImage.h"
#include "IVideoDriver.h"

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <utility>
#include <vector>
#include <cwchar>


namespace gui
{

namespace
{

bool isArabicScriptCodepoint(char32_t c)
{
	return (c >= 0x0600 && c <= 0x08FF) ||
			(c >= 0xFB50 && c <= 0xFDFF) ||
			(c >= 0xFE70 && c <= 0xFEFF) ||
			(c >= 0x1EE00 && c <= 0x1EEFF);
}

bool isIgnorableForFontCoverage(char32_t c)
{
	return c == 0x200C || c == 0x200D || c == 0xFEFF ||
			(c >= 0xFE00 && c <= 0xFE0F) ||
			(c >= 0xE0100 && c <= 0xE01EF);
}

bool isTextLineBreak(char32_t c)
{
	return c == U'\r' || c == U'\n' || c == 0x2028 || c == 0x2029;
}

std::vector<size_t> makeU32ToWideIndexMap(const std::wstring &text)
{
	std::vector<size_t> result;
	result.reserve(text.size());

	for (size_t i = 0; i < text.size(); ++i) {
		result.push_back(i);
		if (sizeof(wchar_t) == 2) {
			const uint32_t current = static_cast<uint16_t>(text[i]);
			if (current >= 0xD800 && current <= 0xDBFF &&
					i + 1 < text.size()) {
					const uint32_t next = static_cast<uint16_t>(text[i + 1]);
					if (next >= 0xDC00 && next <= 0xDFFF)
						++i;
				}
		}
	}
	return result;
}


size_t wideIndexToU32Index(const wchar_t *text, size_t index)
{
	if (!text)
		return 0;

	if (sizeof(wchar_t) == 4)
		return std::min<size_t>(index, wcslen(text));

	size_t wide_index = 0;
	size_t scalar_index = 0;
	while (text[wide_index] != 0 && wide_index < index) {
		const uint32_t current = static_cast<uint16_t>(text[wide_index]);
		if (current >= 0xD800 && current <= 0xDBFF &&
				text[wide_index + 1] != 0) {
			const uint32_t next = static_cast<uint16_t>(text[wide_index + 1]);
			if (next >= 0xDC00 && next <= 0xDFFF) {
				// A cursor between the surrogate pair is normalized before
				// the scalar so UTF-16 editing never splits a code point.
				if (wide_index + 1 >= index)
					break;
				wide_index += 2;
			} else {
				++wide_index;
			}
		} else {
			++wide_index;
		}
		++scalar_index;
	}
	return scalar_index;
}

s32 fixedToSignedPixels(int value)
{
	return value >= 0
			? static_cast<s32>((value + 32) / 64)
			: -static_cast<s32>((-value + 32) / 64);
}

int getLayoutWidth(raqm_t *layout)
{
	size_t count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &count);
	if (!glyphs && count != 0)
		return 0;

	int pen = 0;
	int min_pen = 0;
	int max_pen = 0;
	for (size_t i = 0; i < count; ++i) {
		pen += glyphs[i].x_advance;
		min_pen = std::min(min_pen, pen);
		max_pen = std::max(max_pen, pen);
	}
	return max_pen - min_pen;
}

bool layoutCursorX(raqm_t *layout, size_t boundary, int *x)
{
	if (!layout || !x)
		return false;

	if (boundary == 0) {
		const int width = getLayoutWidth(layout);
		*x = raqm_get_par_resolved_direction(layout) == RAQM_DIRECTION_RTL
				? width : 0;
		return true;
	}

	size_t character = boundary - 1;
	int y = 0;
	return raqm_index_to_position(layout, &character, x, &y);
}

s32 u32IndexToWideIndex(const wchar_t *text, size_t index)
{
	if (!text)
		return 0;
	if (sizeof(wchar_t) == 4)
		return static_cast<s32>(std::min<size_t>(index, wcslen(text)));

	size_t wide_index = 0;
	size_t scalar_index = 0;
	while (text[wide_index] != 0 && scalar_index < index) {
		const uint32_t current = static_cast<uint16_t>(text[wide_index]);
		if (current >= 0xD800 && current <= 0xDBFF &&
				text[wide_index + 1] != 0) {
			const uint32_t next = static_cast<uint16_t>(text[wide_index + 1]);
			if (next >= 0xDC00 && next <= 0xDFFF)
				wide_index += 2;
			else
				++wide_index;
		} else {
			++wide_index;
		}
		++scalar_index;
	}
	return static_cast<s32>(wide_index);
}

u32 fixedToPixels(int value)
{
	return static_cast<u32>(std::max<s32>(0, (value + 32) / 64));
}

int getRaqmTextWidthFixed(raqm_t *layout)
{
	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &glyph_count);
	if (!glyphs && glyph_count != 0)
		return 0;

	int pen = 0;
	int min_pen = 0;
	int max_pen = 0;
	for (size_t i = 0; i < glyph_count; ++i) {
		pen += glyphs[i].x_advance;
		min_pen = std::min(min_pen, pen);
		max_pen = std::max(max_pen, pen);
	}
	return max_pen - min_pen;
}

bool getRaqmCursorX(raqm_t *layout, size_t text_length, size_t cursor_index, int &x)
{
	if (cursor_index > text_length)
		return false;
	int y = 0;
	if (cursor_index == 0) {
		x = raqm_get_par_resolved_direction(layout) == RAQM_DIRECTION_RTL
				? getRaqmTextWidthFixed(layout) : 0;
		return true;
	}
	size_t shaped_index = cursor_index - 1;
	return raqm_index_to_position(layout, &shaped_index, &x, &y);
}

} // namespace

FT_Library SGUITTFace::freetype_library = nullptr;
size_t SGUITTFace::n_faces = 0;

FT_Library SGUITTFace::getFreeTypeLibrary()
{
	if (freetype_library)
		return freetype_library;
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		FATAL_ERROR("initializing freetype failed");
	freetype_library = ft;
	return freetype_library;
}

SGUITTFace::SGUITTFace(std::string &&buffer) : face_buffer(std::move(buffer))
{
	memset((void*)&face, 0, sizeof(FT_Face));
	n_faces++;
}

SGUITTFace::~SGUITTFace()
{
	FT_Done_Face(face);
	n_faces--;
	// If there are no more faces referenced by FreeType, clean up.
	if (n_faces == 0) {
		assert(freetype_library);
		FT_Done_FreeType(freetype_library);
		freetype_library = nullptr;
	}
}

SGUITTFace* SGUITTFace::createFace(std::string &&buffer)
{
	irr_ptr<SGUITTFace> face(new SGUITTFace(std::move(buffer)));
	auto ft = getFreeTypeLibrary();
	if (!ft)
		return nullptr;
	bool ok = FT_New_Memory_Face(ft,
			reinterpret_cast<const FT_Byte*>(face->face_buffer.data()),
			face->face_buffer.size(), 0, &face->face) == 0;
	return ok ? face.release() : nullptr;
}

SGUITTFace* SGUITTFace::loadFace(const io::path &filename)
{
	irr_ptr<SGUITTFace> face(new SGUITTFace(""));
	auto ft = getFreeTypeLibrary();
	if (!ft)
		return nullptr;
	// Prefer FT_New_Face because it doesn't require loading everything
	// to memory.
	bool ok = FT_New_Face(ft, filename.c_str(), 0, &face->face) == 0;
	return ok ? face.release() : nullptr;
}


//////////////////////

//! Wrapper struct for a preloaded glyph
struct SGUITTGlyphPending {
	SGUITTGlyphPending(const SGUITTGlyph *glyph, video::IImage *surface) noexcept :
		glyph(glyph), surface(surface)
	{}
	~SGUITTGlyphPending()
	{
		if (surface)
			surface->drop();
	}

	DISABLE_CLASS_COPY(SGUITTGlyphPending)

	SGUITTGlyphPending(SGUITTGlyphPending &&other) noexcept :
		glyph(other.glyph), surface(other.surface)
	{
		other.surface = nullptr;
	}

	const SGUITTGlyph *glyph;
	video::IImage *surface;
};


//////////////////////

//! Holds a sheet of glyphs.
class CGUITTGlyphPage
{
	public:
		CGUITTGlyphPage(video::IVideoDriver *Driver, const io::path &texture_name) :
			texture(0), available_slots(0), used_slots(0),
			driver(Driver), name(texture_name)
		{}
		~CGUITTGlyphPage()
		{
			if (texture)
				driver->removeTexture(texture);
		}

		//! Create the actual page texture,
		bool createPageTexture(u8 pixel_mode, core::dimension2du texture_size);

		//! Add the glyph to a list of glyphs to be paged.
		//! This collection will be cleared after updateTexture is called.
		void pushGlyphToBePaged(const SGUITTGlyph *glyph, video::IImage *surface)
		{
			if (!glyph || !surface)
				return;
			glyph_to_be_paged.emplace_back(glyph, surface);
		}

		inline bool isDirty() const
		{
			return !glyph_to_be_paged.empty();
		}

		//! Updates the texture atlas with new glyphs.
		void updateTexture();

		video::ITexture* texture;
		u32 available_slots;
		u32 used_slots;

		std::vector<core::vector2di> render_positions;
		std::vector<core::recti> render_source_rects;
		std::vector<video::SColor> render_colors;

	private:
		std::vector<SGUITTGlyphPending> glyph_to_be_paged;
		video::IVideoDriver* driver;
		io::path name;
};


bool CGUITTGlyphPage::createPageTexture(const u8 pixel_mode,
	const core::dimension2du texture_size)
{
	if (texture)
		return false;

	bool flgmip = driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
	bool flgcpy = driver->getTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY);
	driver->setTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY, true);

	// Create texture
	switch (pixel_mode) {
		case FT_PIXEL_MODE_MONO:
			texture = driver->addTexture(texture_size, name, video::ECF_A1R5G5B5);
			break;
		case FT_PIXEL_MODE_GRAY:
		default:
			texture = driver->addTexture(texture_size, name, video::ECF_A8R8G8B8);
			break;
	}

	// Restore texture creation flags
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, flgmip);
	driver->setTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY, flgcpy);

	return texture ? true : false;
}

//! Updates the texture atlas with new glyphs.
void CGUITTGlyphPage::updateTexture()
{
	if (!isDirty())
		return;

	void *ptr = texture->lock();
	if (!ptr)
		return;

	video::ECOLOR_FORMAT format = texture->getColorFormat();
	core::dimension2du size = texture->getOriginalSize();
	video::IImage* pageholder = driver->createImageFromData(format, size, ptr, true, false);

	for (auto &it : glyph_to_be_paged)
		it.surface->copyTo(pageholder, it.glyph->source_rect.UpperLeftCorner);

	pageholder->drop();
	texture->unlock();
	glyph_to_be_paged.clear();
}


//////////////////////

video::IImage* SGUITTGlyph::createGlyphImage(const FT_Bitmap& bits, video::IVideoDriver* driver) const
{
	// Make sure our casts to s32 in the loops below will not cause problems
	if (bits.rows > INT32_MAX || bits.width > INT32_MAX)
		FATAL_ERROR("Insane font glyph size");

	// Determine what our texture size should be.
	// Add 1 because textures are inclusive-exclusive.
	core::dimension2du d(bits.width + 1, bits.rows + 1);
	core::dimension2du texture_size;

	// Turn bitmap into an image
	video::IImage *image = nullptr;
	switch (bits.pixel_mode)
	{
		case FT_PIXEL_MODE_MONO:
		{
			// Create a blank image and fill it with transparent pixels.
			texture_size = d.getOptimalSize(true, true);
			image = driver->createImage(video::ECF_A1R5G5B5, texture_size);
			image->fill(video::SColor(0, 255, 255, 255));

			// Load the monochrome data in.
			const u32 image_pitch = image->getPitch() / sizeof(u16);
			u16* image_data = (u16*)image->getData();
			u8* glyph_data = bits.buffer;

			for (s32 y = 0; y < (s32)bits.rows; ++y)
			{
				u16* row = image_data;
				for (s32 x = 0; x < (s32)bits.width; ++x)
				{
					// Monochrome bitmaps store 8 pixels per byte.  The left-most pixel is the bit 0x80.
					// So, we go through the data each bit at a time.
					if ((glyph_data[y * bits.pitch + (x / 8)] & (0x80 >> (x % 8))) != 0)
						*row = 0xFFFF;
					++row;
				}
				image_data += image_pitch;
			}
			break;
		}

		case FT_PIXEL_MODE_GRAY:
		{
			// Create our blank image.
			texture_size = d.getOptimalSize(!driver->queryFeature(video::EVDF_TEXTURE_NPOT), !driver->queryFeature(video::EVDF_TEXTURE_NSQUARE), true, 0);
			image = driver->createImage(video::ECF_A8R8G8B8, texture_size);
			image->fill(video::SColor(0, 255, 255, 255));

			// Load the grayscale data in.
			const float gray_count = static_cast<float>(bits.num_grays);
			const u32 image_pitch = image->getPitch() / sizeof(u32);
			u32* image_data = (u32*)image->getData();
			u8* glyph_data = bits.buffer;
			for (s32 y = 0; y < (s32)bits.rows; ++y)
			{
				u8* row = glyph_data;
				for (s32 x = 0; x < (s32)bits.width; ++x)
				{
					image_data[y * image_pitch + x] |= static_cast<u32>(255.0f * (static_cast<float>(*row++) / gray_count)) << 24;
				}
				glyph_data += bits.pitch;
			}
			break;
		}
		default:
			errorstream << "CGUITTFont: unknown pixel mode " << (int)bits.pixel_mode << std::endl;
			return 0;
	}
	return image;
}

void SGUITTGlyph::preload(u32 char_index, FT_Face face, CGUITTFont *parent, u32 font_size, const FT_Int32 loadFlags)
{
	// Set the size of the glyph.
	FT_Set_Pixel_Sizes(face, 0, font_size);

	// Attempt to load the glyph.
	auto err = FT_Load_Glyph(face, char_index, loadFlags);
	if (err != FT_Err_Ok) {
		warningstream << "SGUITTGlyph: failed to load glyph " << char_index
			<< " with error: " << (int)err << std::endl;
		return;
	}

	FT_GlyphSlot glyph = face->glyph;
	const FT_Bitmap &bits = glyph->bitmap;

	// Setup the glyph information here:
	advance = core::vector2di(glyph->advance.x, glyph->advance.y);
	offset = core::vector2di(glyph->bitmap_left, glyph->bitmap_top);

	// Try to get the last page with available slots.
	CGUITTGlyphPage* page = parent->getLastGlyphPage();

	// If we need to make a new page, do that now.
	if (!page)
	{
		page = parent->createGlyphPage(bits.pixel_mode);
		if (!page)
			return;
	}

	// Allocate slot from page
	glyph_page = parent->getLastGlyphPageIndex();
	u32 texture_side_length = page->texture->getOriginalSize().Width;
	core::vector2di page_position(
		(page->used_slots % (texture_side_length / font_size)) * font_size,
		(page->used_slots / (texture_side_length / font_size)) * font_size
	);
	source_rect.UpperLeftCorner = page_position;
	source_rect.LowerRightCorner = core::vector2di(page_position.X + bits.width, page_position.Y + bits.rows);

	++page->used_slots;
	--page->available_slots;

	// createGlyphImage can now be called, the next preload() call will however
	// invalidate the data in `bits`.
}

void SGUITTGlyph::unload()
{
	// reset isLoaded to false
	source_rect = core::recti();
}


//////////////////////

CGUITTFont *CGUITTFont::createTTFont(IGUIEnvironment *env,
		SGUITTFace *face, u32 size, bool antialias,
		bool preload, u32 shadow, u32 shadow_alpha)
{
	CGUITTFont *font = new CGUITTFont(env);
	bool ret = font->load(face, size, antialias, true, preload);
	if (!ret) {
		font->drop();
		return 0;
	}

	font->shadow_offset = shadow;
	font->shadow_alpha = shadow_alpha;

	return font;
}

//////////////////////

//! Constructor.
CGUITTFont::CGUITTFont(IGUIEnvironment *env) :
	use_monochrome(false), use_hinting(true), use_auto_hinting(true),
	batch_load_size(1)
{

	if (env) {
		// don't grab environment, to avoid circular references
		Driver = env->getVideoDriver();
	}

	if (Driver)
		Driver->grab();

	setInvisibleCharacters(L" ");
}

bool CGUITTFont::load(SGUITTFace *face, const u32 size, const bool antialias,
	const bool transparency, const bool preload)
{
	if (!Driver || size == 0 || !face)
		return false;

	this->size = size;

	// Update the font loading flags when the font is first loaded
	this->use_monochrome = !antialias;
	update_load_flags();

	// Store our face.
	m_face.grab(face);
	tt_face = face->face;

	// Store font metrics.
	FT_Set_Pixel_Sizes(tt_face, size, 0);
	font_metrics = tt_face->size->metrics;

	verbosestream << tt_face->num_glyphs << " glyphs, ascender=" << font_metrics.ascender
		<< " height=" << font_metrics.height << std::endl;

	// Allocate our glyphs.
	Glyphs.clear();
	Glyphs.set_used(tt_face->num_glyphs);

	// Cache the first 127 ASCII characters
	if (preload) {
		u32 old_size = batch_load_size;
		batch_load_size = 127;
		getGlyphIndexByChar(U' '); // char needs to exist, so pick space
		batch_load_size = old_size;
	}

	return true;
}

CGUITTFont::~CGUITTFont()
{
	// Delete the glyphs and glyph pages.
	reset_images();
	Glyphs.clear();

	// Drop our driver now.
	if (Driver)
		Driver->drop();
}

void CGUITTFont::reset_images()
{
	// Delete the glyphs.
	for (u32 i = 0; i != Glyphs.size(); ++i)
		Glyphs[i].unload();

	// Unload the glyph pages from video memory.
	for (u32 i = 0; i != Glyph_Pages.size(); ++i)
		delete Glyph_Pages[i];
	Glyph_Pages.clear();

	// Always update the internal FreeType loading flags after resetting.
	update_load_flags();
}

void CGUITTFont::update_glyph_pages() const
{
	for (u32 i = 0; i != Glyph_Pages.size(); ++i)
	{
		if (Glyph_Pages[i]->isDirty())
			Glyph_Pages[i]->updateTexture();
	}
}

CGUITTGlyphPage* CGUITTFont::getLastGlyphPage() const
{
	if (Glyph_Pages.empty())
		return nullptr;
	CGUITTGlyphPage *page = Glyph_Pages[getLastGlyphPageIndex()];
	if (page->available_slots == 0)
		return nullptr;
	return page;
}

CGUITTGlyphPage* CGUITTFont::createGlyphPage(const u8 pixel_mode)
{
	CGUITTGlyphPage *page = nullptr;

	// Name of our page.
	io::path name("glyph_");
	name += tt_face->family_name;
	name += ".";
	name += tt_face->style_name;
	name += ".";
	name += size;
	name += "_";
	name += Glyph_Pages.size(); // The newly created page will be at the end of the collection.

	// Create the new page.
	page = new CGUITTGlyphPage(Driver, name);

	// Determine our maximum texture size.
	core::dimension2du max_texture_size = Driver->getMaxTextureSize();

	// We want to try to put at least 180 glyphs on a single texture.
	// magic number = floor(texture_size / sqrt(180))
	core::dimension2du page_texture_size;
	if (size <= 19) page_texture_size = core::dimension2du(256, 256);
	else if (size <= 38) page_texture_size = core::dimension2du(512, 512);
	else if (size <= 76) page_texture_size = core::dimension2du(1024, 1024);
	else if (size <= 152) page_texture_size = core::dimension2du(2048, 2048);
	else page_texture_size = core::dimension2du(4096, 4096);

	if (page_texture_size.Width > max_texture_size.Width || page_texture_size.Height > max_texture_size.Height)
		page_texture_size = max_texture_size;

	if (!page->createPageTexture(pixel_mode, page_texture_size)) {
		errorstream << "CGUITTGlyphPage: failed to create texture ("
			<< page_texture_size.Width << "x" << page_texture_size.Height << ")" << std::endl;
		delete page;
		return 0;
	}

	// Determine the number of glyph slots on the page and add it to the list of pages
	page->available_slots = (page_texture_size.Width / size) * (page_texture_size.Height / size);
	Glyph_Pages.push_back(page);
	return page;
}

void CGUITTFont::setFallback(gui::IGUIFont *font)
{
	sanity_check(font != this);
	fallback.grab(font);
}

void CGUITTFont::setMonochrome(const bool flag)
{
	use_monochrome = flag;
	reset_images();
}

void CGUITTFont::setFontHinting(const bool enable, const bool enable_auto_hinting)
{
	use_hinting = enable;
	use_auto_hinting = enable_auto_hinting;
	reset_images();
}

void CGUITTFont::draw(const core::stringw& text, const core::rect<s32>& position, video::SColor color, bool hcenter, bool vcenter, const core::rect<s32>* clip)
{
	// Allow colors to work for strings that have passed through irrlicht by catching
	// them here and converting them to enriched just before drawing.
	EnrichedString s(text.c_str(), color);
	draw(s, position, hcenter, vcenter, clip);
}


void CGUITTFont::draw(const EnrichedString &text, const core::rect<s32>& position, bool hcenter, bool vcenter, const core::rect<s32>* clip)
{
	draw(text, position, hcenter, vcenter, clip, RAQM_DIRECTION_DEFAULT);
}

void CGUITTFont::draw(const EnrichedString &text, const core::rect<s32>& position, bool hcenter, bool vcenter, const core::rect<s32>* clip, raqm_direction_t paragraph_direction)
{
	if (!Driver || !tt_face)
		return;

	clearRenderQueues();

	const std::wstring &wide_text = text.getString();
	const std::vector<video::SColor> &colors = text.getColors();
	const std::vector<size_t> u32_to_wide = makeU32ToWideIndexMap(wide_text);
	const std::u32string utext = convertWCharToU32String(wide_text.c_str());
	const u32 line_height = getLineHeight();

	core::position2d<s32> base = position.UpperLeftCorner;
	if (vcenter) {
		const core::dimension2d<u32> dimensions = getDimension(utext);
		base.Y += (position.getHeight() - static_cast<s32>(dimensions.Height)) / 2;
	}

	constexpr video::SColor fallback_color(255, 255, 255, 255);
	size_t line_start = 0;
	while (line_start <= utext.size()) {
		size_t line_end = line_start;
		while (line_end < utext.size() && !isTextLineBreak(utext[line_end]))
			++line_end;

		const std::u32string line = utext.substr(line_start, line_end - line_start);
		if (!line.empty()) {
			raqm_t *layout = createLayout(line, paragraph_direction);
			if (layout) {
				size_t glyph_count = 0;
				raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &glyph_count);
				if (glyphs || glyph_count == 0) {
					int pen = 0;
					int min_pen = 0;
					int max_pen = 0;
					for (size_t i = 0; i < glyph_count; ++i) {
						pen += glyphs[i].x_advance;
						min_pen = std::min(min_pen, pen);
						max_pen = std::max(max_pen, pen);
					}
					const s32 line_width = static_cast<s32>((max_pen - min_pen + 32) / 64);
					const s32 line_x = hcenter
							? base.X + (position.getWidth() - line_width) / 2
							: base.X;
					int pen_x = line_x * 64 - min_pen;
					const s32 baseline = base.Y + font_metrics.ascender / 64;

					for (size_t i = 0; i < glyph_count; ++i) {
						const raqm_glyph_t &shaped = glyphs[i];
						const int glyph_pen_x = pen_x;
						pen_x += shaped.x_advance;

						if (shaped.cluster >= line.size())
							continue;
						if (InvisibleChars.find(line[shaped.cluster]) != std::u32string::npos)
							continue;

						CGUITTFont *owner = getFontForFace(shaped.ftface);
						if (!owner)
							owner = this;
						SGUITTGlyph *glyph = owner->getGlyphByIndex(shaped.index);
						if (!glyph || glyph->glyph_page >= owner->Glyph_Pages.size())
							continue;

						const s32 glyph_x = static_cast<s32>((glyph_pen_x + shaped.x_offset + 32) / 64)
								+ glyph->offset.X;
						const s32 owner_baseline = base.Y + owner->font_metrics.ascender / 64;
						const s32 glyph_y = owner_baseline - static_cast<s32>((shaped.y_offset + 32) / 64)
								- glyph->offset.Y;
						const core::recti glyph_rect(
							core::position2di(glyph_x, glyph_y),
							core::dimension2di(glyph->source_rect.getWidth(),
									glyph->source_rect.getHeight()));
						if (clip && !clip->isRectCollided(glyph_rect))
							continue;

						const size_t absolute_cluster = line_start + shaped.cluster;
						const size_t wide_index = absolute_cluster < u32_to_wide.size()
								? u32_to_wide[absolute_cluster] : wide_text.size();
						const video::SColor glyph_color = wide_index < colors.size()
								? colors[wide_index] : fallback_color;

						CGUITTGlyphPage *page = owner->Glyph_Pages[glyph->glyph_page];
						page->render_positions.emplace_back(glyph_x, glyph_y);
						page->render_source_rects.push_back(glyph->source_rect);
						page->render_colors.push_back(glyph_color);
				}
				}
				raqm_destroy(layout);
			}
		}

		base.Y += line_height;
		if (line_end == utext.size())
			break;
		if (utext[line_end] == U'\r' && line_end + 1 < utext.size() &&
				utext[line_end + 1] == U'\n')
			line_start = line_end + 2;
		else
			line_start = line_end + 1;
	}

	renderQueuedGlyphs(clip);
}

core::dimension2d<u32> CGUITTFont::getDimension(const wchar_t* text) const
{
	return getDimension(convertWCharToU32String(text), RAQM_DIRECTION_DEFAULT);
}

core::dimension2du CGUITTFont::getDimension(const wchar_t *text, raqm_direction_t direction) const
{
	return getDimension(convertWCharToU32String(text), direction);
}

core::dimension2d<u32> CGUITTFont::getDimension(const std::u32string& utext) const
{
	return getDimension(utext, RAQM_DIRECTION_DEFAULT);
}

core::dimension2d<u32> CGUITTFont::getDimension(const std::u32string& utext, raqm_direction_t direction) const
{
	const u32 line_height = getLineHeight();
	u32 line_count = 1;
	u32 maximum_width = 0;

	size_t line_start = 0;
	while (line_start <= utext.size()) {
		size_t line_end = line_start;
		while (line_end < utext.size() && !isTextLineBreak(utext[line_end]))
			++line_end;

		const std::u32string line = utext.substr(line_start, line_end - line_start);
		if (!line.empty()) {
			raqm_t *layout = createLayout(line, direction);
			if (layout) {
				size_t glyph_count = 0;
				raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &glyph_count);
				if (glyphs || glyph_count == 0) {
					int pen = 0;
					int min_pen = 0;
					int max_pen = 0;
					for (size_t i = 0; i < glyph_count; ++i) {
						pen += glyphs[i].x_advance;
						min_pen = std::min(min_pen, pen);
						max_pen = std::max(max_pen, pen);
					}
					const u32 width = fixedToPixels(max_pen - min_pen);
					maximum_width = std::max(maximum_width, width);
				}
				raqm_destroy(layout);
			}
		}

		if (line_end == utext.size())
			break;
		++line_count;
		if (utext[line_end] == U'\r' && line_end + 1 < utext.size() &&
				utext[line_end + 1] == U'\n')
			line_start = line_end + 2;
		else
			line_start = line_end + 1;
	}

	return core::dimension2d<u32>(maximum_width, line_count * line_height);
}

CGUITTFont *CGUITTFont::getFontForFace(FT_Face face) const
{
	if (face == tt_face)
		return const_cast<CGUITTFont *>(this);
	if (!fallback)
		return nullptr;
	return static_cast<CGUITTFont *>(fallback.get())->getFontForFace(face);
}

CGUITTFont *CGUITTFont::getFontForCluster(const std::u32string &text, size_t start, size_t end) const
{
	std::vector<CGUITTFont *> candidates;
	CGUITTFont *candidate = const_cast<CGUITTFont *>(this);
	while (candidate) {
		candidates.push_back(candidate);
		if (!candidate->fallback)
			break;
		candidate = static_cast<CGUITTFont *>(candidate->fallback.get());
	}

	for (CGUITTFont *font : candidates) {
		bool supports_cluster = true;
		for (size_t i = start; i < end; ++i) {
			const char32_t c = text[i];
			if (isIgnorableForFontCoverage(c))
				continue;
			if (FT_Get_Char_Index(font->tt_face, static_cast<FT_ULong>(c)) == 0) {
				supports_cluster = false;
				break;
			}
		}
		if (supports_cluster)
			return font;
	}

	return const_cast<CGUITTFont *>(this);
}

raqm_t *CGUITTFont::createLayout(const std::u32string &text, raqm_direction_t direction) const
{
	if (!tt_face || text.empty())
		return nullptr;

	static_assert(sizeof(char32_t) == sizeof(uint32_t), "Raqm requires UTF-32 code points");
	std::vector<uint32_t> codepoints(text.begin(), text.end());

	raqm_t *layout = raqm_create();
	if (!layout)
		return nullptr;

	if (FT_Set_Pixel_Sizes(tt_face, 0, size) != 0 ||
			!raqm_set_text(layout, codepoints.data(), codepoints.size()) ||
			!raqm_set_par_direction(layout, direction) ||
			!raqm_set_freetype_face(layout, tt_face) ||
			!raqm_set_freetype_load_flags(layout, load_flags)) {
		raqm_destroy(layout);
		return nullptr;
	}

	CGUITTFont *font = const_cast<CGUITTFont *>(this);
	while (font) {
		FT_Set_Pixel_Sizes(font->tt_face, 0, font->size);
		if (!font->fallback)
			break;
		font = static_cast<CGUITTFont *>(font->fallback.get());
	}

	for (size_t i = 0; i < text.size();) {
		if (!isArabicScriptCodepoint(text[i])) {
			++i;
			continue;
		}
		const size_t start = i;
		while (i < text.size() &&
				(isArabicScriptCodepoint(text[i]) ||
						(text[i] >= 0x064B && text[i] <= 0x065F)))
			++i;
		raqm_set_language(layout, "ar", start, i - start);
	}

	size_t start = 0;
	while (start < text.size()) {
		size_t end = start;
		while (end + 1 < text.size() &&
				!raqm_allowed_grapheme_boundary(layout, end))
			++end;
		++end;

		CGUITTFont *selected = getFontForCluster(text, start, end);
		if (selected != this) {
			if (!raqm_set_freetype_face_range(layout, selected->tt_face, start, end - start) ||
					!raqm_set_freetype_load_flags_range(layout, selected->load_flags, start, end - start)) {
				raqm_destroy(layout);
				return nullptr;
			}
		}
		start = end;
	}

	if (!raqm_layout(layout)) {
		raqm_destroy(layout);
		return nullptr;
	}
	return layout;
}

SGUITTGlyph *CGUITTFont::getGlyphByIndex(u32 glyph_index) const
{
	if (!tt_face || glyph_index == 0 || glyph_index > Glyphs.size())
		return nullptr;

	SGUITTGlyph &glyph = Glyphs[glyph_index - 1];
	if (glyph.isLoaded())
		return &glyph;

	FT_Set_Pixel_Sizes(tt_face, 0, size);
	CGUITTFont *self = const_cast<CGUITTFont *>(this);
	glyph.preload(glyph_index, tt_face, self, size, load_flags);
	if (!glyph.isLoaded() || glyph.glyph_page >= Glyph_Pages.size())
		return nullptr;

	video::IImage *surface = glyph.createGlyphImage(tt_face->glyph->bitmap, Driver);
	if (!surface)
		return nullptr;
	Glyph_Pages[glyph.glyph_page]->pushGlyphToBePaged(&glyph, surface);
	return &glyph;
}

void CGUITTFont::clearRenderQueues() const
{
	for (u32 i = 0; i < Glyph_Pages.size(); ++i) {
		Glyph_Pages[i]->render_positions.clear();
		Glyph_Pages[i]->render_source_rects.clear();
		Glyph_Pages[i]->render_colors.clear();
	}
	if (fallback)
		static_cast<CGUITTFont *>(fallback.get())->clearRenderQueues();
}

void CGUITTFont::renderQueuedGlyphs(const core::rect<s32> *clip) const
{
	update_glyph_pages();
	core::array<core::vector2di> tmp_positions;
	core::array<core::recti> tmp_source_rects;
	for (u32 page_i = 0; page_i < Glyph_Pages.size(); ++page_i) {
		CGUITTGlyphPage *page = Glyph_Pages[page_i];
		if (page->render_positions.empty())
			continue;

		assert(page->render_positions.size() == page->render_colors.size());
		assert(page->render_positions.size() == page->render_source_rects.size());

		for (size_t i = 0; i < page->render_positions.size(); ++i) {
			const size_t begin = i;
			const video::SColor color = page->render_colors[i];
			do {
				++i;
			} while (i < page->render_positions.size() && page->render_colors[i] == color);
			tmp_positions.set_data(&page->render_positions[begin], i - begin);
			tmp_source_rects.set_data(&page->render_source_rects[begin], i - begin);
			--i;

			if (shadow_offset) {
				for (size_t j = 0; j < tmp_positions.size(); ++j)
					tmp_positions[j] += core::vector2di(shadow_offset, shadow_offset);
				const u32 alpha = core::clamp(
						core::round32(shadow_alpha * color.getAlpha() / 255.0f), 0, 255);
				Driver->draw2DImageBatch(page->texture, tmp_positions, tmp_source_rects,
						clip, video::SColor(alpha, 0, 0, 0), true);
				for (size_t j = 0; j < tmp_positions.size(); ++j)
					tmp_positions[j] -= core::vector2di(shadow_offset, shadow_offset);
			}

			Driver->draw2DImageBatch(page->texture, tmp_positions, tmp_source_rects,
					clip, color, true);
		}
	}

	if (fallback)
		static_cast<CGUITTFont *>(fallback.get())->renderQueuedGlyphs(clip);
}


inline u32 CGUITTFont::getWidthFromCharacter(char32_t c) const
{
	u32 n = getGlyphIndexByChar(c);
	if (n > 0)
	{
		int w = Glyphs[n-1].advance.X / 64;
		return w;
	}
	if (fallback)
	{
		wchar_t s[] = { (wchar_t) c, 0 };
		return fallback->getDimension(s).Width;
	}

	if (c >= 0x2000)
		return (font_metrics.ascender / 64);
	else return (font_metrics.ascender / 64) / 2;
}

inline u32 CGUITTFont::getHeightFromCharacter(char32_t c) const
{
	u32 n = getGlyphIndexByChar(c);
	if (n > 0)
	{
		// Grab the true height of the character, taking into account underhanging glyphs.
		s32 height = (font_metrics.ascender / 64) - Glyphs[n-1].offset.Y + Glyphs[n-1].source_rect.getHeight();
		return height;
	}
	if (fallback)
	{
		wchar_t s[] = { (wchar_t) c, 0 };
		return fallback->getDimension(s).Height;
	}

	if (c >= 0x2000)
		return (font_metrics.ascender / 64);
	else return (font_metrics.ascender / 64) / 2;
}

u32 CGUITTFont::getGlyphIndexByChar(char32_t c) const
{
	// Get the glyph.
	u32 glyph = FT_Get_Char_Index(tt_face, c);

	// Check for a valid glyph.
	if (glyph == 0)
		return 0;

	// If our glyph is already loaded, don't bother doing any batch loading code.
	if (Glyphs[glyph - 1].isLoaded())
		return glyph;

	// Determine our batch loading positions.
	u32 half_size = (batch_load_size / 2);
	u32 start_pos = 0;
	if (c > half_size)
		start_pos = c - half_size;
	u32 end_pos = start_pos + batch_load_size;

	// Load all our characters.
	do
	{
		// Get the character we are going to load.
		u32 char_index = FT_Get_Char_Index(tt_face, start_pos);

		// If the glyph hasn't been loaded yet, do it now.
		if (char_index)
		{
			SGUITTGlyph& glyph = Glyphs[char_index - 1];
			if (!glyph.isLoaded())
			{
				auto *this2 = const_cast<CGUITTFont*>(this); // oh well
				glyph.preload(char_index, tt_face, this2, size, load_flags);
				auto *surface = glyph.createGlyphImage(tt_face->glyph->bitmap, Driver);
				Glyph_Pages[glyph.glyph_page]->pushGlyphToBePaged(&glyph, surface);
			}
		}
	}
	while (++start_pos < end_pos);

	// Return our original character.
	return glyph;
}



s32 CGUITTFont::getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
{
	const std::u32string unicode = convertWCharToU32String(text);
	const s32 index = getCharacterFromPos(unicode, pixel_x);
	if (index < 0)
		return -1;
	return u32IndexToWideIndex(text, static_cast<size_t>(index));
}

s32 CGUITTFont::getCharacterFromPos(const std::u32string& text, s32 pixel_x) const
{
	size_t line_end = 0;
	while (line_end < text.size() && !isTextLineBreak(text[line_end]))
		++line_end;
	const std::u32string line = text.substr(0, line_end);
	if (line.empty())
		return 0;

	raqm_t *layout = createLayout(line);
	if (!layout)
		return -1;

	size_t index = 0;
	const bool success = raqm_position_to_index(layout, pixel_x * 64, 0, &index);
	raqm_destroy(layout);
	return success ? static_cast<s32>(index) : -1;
}

s32 CGUITTFont::getCursorPosition(const wchar_t *text, s32 char_index) const
{
	if (!text || !*text || char_index < 0)
		return 0;

	const std::u32string unicode = convertWCharToU32String(text);
	raqm_t *layout = createLayout(unicode);
	if (!layout)
		return IGUIFont::getCursorPosition(text, char_index);

	const size_t wide_length = wcslen(text);
	const size_t wide_index = std::min<size_t>(static_cast<size_t>(char_index), wide_length);
	const size_t scalar_index = wideIndexToU32Index(text, wide_index);
	int x = 0;
	const bool success = scalar_index <= unicode.size() &&
			layoutCursorX(layout, scalar_index, &x);
	raqm_destroy(layout);
	return success ? fixedToSignedPixels(x)
			: IGUIFont::getCursorPosition(text, char_index);
}


s32 CGUITTFont::getNextCursorPosition(const wchar_t *text, s32 char_index, s32 direction) const
{
	if (!text || !*text)
		return 0;

	const std::u32string unicode = convertWCharToU32String(text);
	const size_t wide_length = wcslen(text);
	const size_t wide_cursor = std::min<size_t>(
			static_cast<size_t>(std::max<s32>(0, char_index)), wide_length);
	const size_t cursor = wideIndexToU32Index(text, wide_cursor);

	// Raqm operates on one paragraph at a time. Treat CR, LF, CRLF and
	// Unicode paragraph separators as logical line boundaries here.
	size_t line_start = cursor;
	while (line_start > 0 && !isTextLineBreak(unicode[line_start - 1]))
		--line_start;
	size_t line_end = cursor;
	while (line_end < unicode.size() && !isTextLineBreak(unicode[line_end]))
		++line_end;

	if (direction < 0 && cursor == line_start && line_start > 0) {
		size_t target = line_start - 1;
		if (unicode[target] == U'\n' && target > 0 && unicode[target - 1] == U'\r')
			--target;
		return u32IndexToWideIndex(text, target);
	}
	if (direction >= 0 && cursor == line_end && line_end < unicode.size()) {
		size_t target = line_end + 1;
		if (unicode[line_end] == U'\r' && target < unicode.size() &&
				unicode[target] == U'\n')
			++target;
		return u32IndexToWideIndex(text, target);
	}

	const std::u32string line = unicode.substr(line_start, line_end - line_start);
	raqm_t *layout = createLayout(line);
	if (!layout)
		return IGUIFont::getNextCursorPosition(text, char_index, direction);

	const auto shaping_clusters =
			text_layout::getShapingClusterBoundaries(layout, line.size());
	size_t next = cursor - line_start;
	if (direction < 0) {
		while (next > 0) {
			--next;
			if (text_layout::isAllowedCursorBoundary(
						layout, next, line.size(), shaping_clusters))
				break;
		}
	} else {
		while (next < line.size()) {
			++next;
			if (text_layout::isAllowedCursorBoundary(
						layout, next, line.size(), shaping_clusters))
				break;
		}
	}

	raqm_destroy(layout);
	return u32IndexToWideIndex(text, line_start + next);
}

s32 CGUITTFont::getVisualCursorPosition(const wchar_t *text, s32 char_index, s32 direction) const
{
	if (!text || !*text)
		return 0;

	const std::u32string unicode = convertWCharToU32String(text);
	const size_t wide_length = wcslen(text);
	const size_t wide_cursor = std::min<size_t>(
			static_cast<size_t>(std::max<s32>(0, char_index)), wide_length);
	const size_t cursor = wideIndexToU32Index(text, wide_cursor);

	size_t line_start = cursor;
	while (line_start > 0 && !isTextLineBreak(unicode[line_start - 1]))
		--line_start;
	size_t line_end = cursor;
	while (line_end < unicode.size() && !isTextLineBreak(unicode[line_end]))
		++line_end;

	const std::u32string line = unicode.substr(line_start, line_end - line_start);
	raqm_t *layout = createLayout(line);
	if (!layout)
		return IGUIFont::getVisualCursorPosition(text, char_index, direction);

	const size_t local_cursor = cursor - line_start;
	const auto shaping_clusters =
			text_layout::getShapingClusterBoundaries(layout, line.size());
	std::vector<std::pair<s32, size_t>> stops;
	stops.reserve(line.size() + 1);
	for (size_t boundary = 0; boundary <= line.size(); ++boundary) {
		if (!text_layout::isAllowedCursorBoundary(
					layout, boundary, line.size(), shaping_clusters))
			continue;
		int x = 0;
		if (layoutCursorX(layout, boundary, &x))
			stops.emplace_back(fixedToSignedPixels(x), boundary);
	}

	std::stable_sort(stops.begin(), stops.end(),
		[](const auto &left, const auto &right) {
			if (left.first != right.first)
				return left.first < right.first;
			return left.second < right.second;
		});

	// Keep both logical insertion points when two bidi runs meet at the same
	// visual coordinate. They are distinct keyboard stops despite sharing x.
	size_t current_stop = stops.size();
	for (size_t i = 0; i < stops.size(); ++i) {
		if (stops[i].second == local_cursor) {
			current_stop = i;
			break;
		}
	}
	if (current_stop == stops.size() && !stops.empty()) {
		int logical_cursor_x = 0;
		if (layoutCursorX(layout, local_cursor, &logical_cursor_x)) {
			int best_distance = std::numeric_limits<int>::max();
			for (size_t i = 0; i < stops.size(); ++i) {
				const int distance = std::abs(stops[i].first - fixedToSignedPixels(logical_cursor_x));
				if (distance < best_distance) {
					best_distance = distance;
					current_stop = i;
				}
			}
		}
	}

	size_t target = current_stop;
	if (current_stop < stops.size()) {
		if (direction < 0 && current_stop > 0)
			target = current_stop - 1;
		else if (direction >= 0 && current_stop + 1 < stops.size())
			target = current_stop + 1;
	}

	const size_t local_result = target < stops.size()
			? stops[target].second : local_cursor;
	raqm_destroy(layout);
	return u32IndexToWideIndex(text, line_start + local_result);
}

bool CGUITTFont::isRightToLeft(const wchar_t *text) const
{
	if (!text || !*text)
		return false;

	const std::u32string unicode = convertWCharToU32String(text);
	size_t line_end = 0;
	while (line_end < unicode.size() && !isTextLineBreak(unicode[line_end]))
		++line_end;

	const std::u32string paragraph = unicode.substr(0, line_end);
	if (paragraph.empty())
		return false;

	raqm_t *layout = createLayout(paragraph);
	if (!layout)
		return false;

	const bool rtl = raqm_get_par_resolved_direction(layout) == RAQM_DIRECTION_RTL;
	raqm_destroy(layout);
	return rtl;
}


void CGUITTFont::setKerningWidth(s32 kerning)
{
	GlobalKerningWidth = kerning;
}

void CGUITTFont::setKerningHeight(s32 kerning)
{
	GlobalKerningHeight = kerning;
}

core::vector2di CGUITTFont::getKerning(const wchar_t thisLetter, const wchar_t previousLetter) const
{
	return getKerning((char32_t)thisLetter, (char32_t)previousLetter);
}

core::vector2di CGUITTFont::getKerning(const char32_t thisLetter, const char32_t previousLetter) const
{
	if (tt_face == 0 || thisLetter == 0 || previousLetter == 0)
		return core::vector2di();

	// Set the size of the face.
	// This is because we cache faces and the face may have been set to a different size.
	FT_Set_Pixel_Sizes(tt_face, 0, size);

	core::vector2di ret(GlobalKerningWidth, GlobalKerningHeight);

	u32 n = getGlyphIndexByChar(thisLetter);

	// If we don't have this glyph, ask fallback font
	if (n == 0)
	{
		if (fallback)
			ret = fallback->getKerning((wchar_t) thisLetter, (wchar_t) previousLetter);
		return ret;
	}

	// If we don't have kerning, no point in continuing.
	if (!FT_HAS_KERNING(tt_face))
		return ret;

	// Get the kerning information.
	FT_Vector v;
	FT_Get_Kerning(tt_face, getGlyphIndexByChar(previousLetter), n, FT_KERNING_DEFAULT, &v);

	// If we have a scalable font, the return value will be in font points.
	if (FT_IS_SCALABLE(tt_face))
	{
		// Font points, so divide by 64.
		ret.X += (v.x / 64);
		ret.Y += (v.y / 64);
	}
	else
	{
		// Pixel units.
		ret.X += v.x;
		ret.Y += v.y;
	}
	return ret;
}

void CGUITTFont::setInvisibleCharacters(const wchar_t *s)
{
	InvisibleChars = convertWCharToU32String(s);
}

std::u32string CGUITTFont::convertWCharToU32String(const wchar_t* const charArray) const
{
	static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "unexpected wchar size");

	if (sizeof(wchar_t) == 4) // wchar_t is UTF-32
		return std::u32string(reinterpret_cast<const char32_t*>(charArray));

	// wchar_t is UTF-16 and we need to convert.
	// std::codecvt could do this for us but aside from being deprecated,
	// it turns out that it's laughably slow on MSVC. Thanks Microsoft.

	std::u32string ret;
	ret.reserve(wcslen(charArray));
	const wchar_t *p = charArray;
	while (*p) {
		char32_t c = *p;
		if (c >= 0xD800 && c < 0xDC00) {
			p++;
			char32_t c2 = *p;
			if (!c2)
				break;
			else if (c2 < 0xDC00 || c2 > 0xDFFF)
				continue; // can't find low surrogate, skip
			c = 0x10000 + ( ((c & 0x3ff) << 10) | (c2 & 0x3ff) );
		}
		ret.push_back(c);
		p++;
	}
	return ret;
}


} // end namespace gui

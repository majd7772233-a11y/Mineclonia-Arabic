// Copyright (C) 2002-2012 Nikolaus Gebhardt
// Copyright (C) 2016 Nathanaëlle Courant:
//   Modified the functions to use EnrichedText instead of string.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "static_text.h"

#include <dimension2d.h>
#include <EGUIAlignment.h>
#include <IGUIFont.h>
#include <IVideoDriver.h>
#include <rect.h>
#include <SColor.h>

#include "CGUITTFont.h"
#include "util/enriched_string.h"


namespace gui
{
//! constructor
StaticText::StaticText(const EnrichedString &text, bool border,
			IGUIEnvironment* environment, IGUIElement* parent,
			s32 id, const core::rect<s32>& rectangle,
			bool background)
: IGUIStaticText(environment, parent, id, rectangle),
	HAlign(EGUIA_UPPERLEFT), VAlign(EGUIA_UPPERLEFT),
	Border(border), WordWrap(false), Background(background),
	RestrainTextInside(true), RightToLeft(false), RightToLeftOverride(false),
	OverrideFont(0), LastBreakFont(0)
{
	setText(text);
}


//! destructor
StaticText::~StaticText()
{
	if (OverrideFont)
		OverrideFont->drop();
}

//! draws the element and its children
void StaticText::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;
	video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> frameRect(AbsoluteRect);

	// draw background

	if (Background)
		driver->draw2DRectangle(getBackgroundColor(), frameRect, &AbsoluteClippingRect);

	// draw the border

	if (Border)
	{
		skin->draw3DSunkenPane(this, 0, true, false, frameRect, &AbsoluteClippingRect);
		frameRect.UpperLeftCorner.X += skin->getSize(EGDS_TEXT_DISTANCE_X);
	}

	// draw the text
	IGUIFont *font = getActiveFont();
	if (font && !BrokenText.empty()) {
		if (font != LastBreakFont)
			updateText();

		core::rect<s32> r = frameRect;
		const auto text_dim = getTextDimensions();
		if (VAlign == EGUIA_CENTER) {
			r.UpperLeftCorner.Y = r.getCenter().Y - (text_dim.Height / 2);
		} else if (VAlign == EGUIA_LOWERRIGHT) {
			r.UpperLeftCorner.Y = r.LowerRightCorner.Y - text_dim.Height;
		}

		if (HAlign == EGUIA_CENTER) {
			r.UpperLeftCorner.X = r.getCenter().X - (text_dim.Width / 2);
		} else if (HAlign == EGUIA_LOWERRIGHT) {
			r.UpperLeftCorner.X = r.LowerRightCorner.X - text_dim.Width;
		}

		IRR_DOWN_CAST<CGUITTFont *>(font)->draw(BrokenText, r,
				false, false,
				RestrainTextInside ? &AbsoluteClippingRect : nullptr,
				RightToLeft ? RAQM_DIRECTION_RTL : RAQM_DIRECTION_LTR);
	}

	IGUIElement::draw();
}


//! Sets another skin independent font.
void StaticText::setOverrideFont(IGUIFont* font)
{
	if (OverrideFont == font)
		return;

	if (OverrideFont)
		OverrideFont->drop();

	OverrideFont = font;

	if (OverrideFont)
		OverrideFont->grab();

	updateText();
}

//! Gets the override font (if any)
IGUIFont * StaticText::getOverrideFont() const
{
	return OverrideFont;
}

//! Get the font which is used right now for drawing
IGUIFont* StaticText::getActiveFont() const
{
	if ( OverrideFont )
		return OverrideFont;
	IGUISkin* skin = Environment->getSkin();
	if (skin)
		return skin->getFont();
	return 0;
}

//! Sets another color for the text.
void StaticText::setOverrideColor(video::SColor color)
{
	ColoredText.setDefaultColor(color);
	updateText();
}


//! Sets another color for the text.
void StaticText::setBackgroundColor(video::SColor color)
{
	ColoredText.setBackground(color);
	Background = true;
}


//! Sets whether to draw the background
void StaticText::setDrawBackground(bool draw)
{
	Background = draw;
}


//! Gets the background color
video::SColor StaticText::getBackgroundColor() const
{
	IGUISkin *skin = Environment->getSkin();

	return (ColoredText.hasBackground() || !skin) ?
		ColoredText.getBackground() : skin->getColor(gui::EGDC_3D_FACE);
}


//! Checks if background drawing is enabled
bool StaticText::isDrawBackgroundEnabled() const
{
	return Background;
}


//! Sets whether to draw the border
void StaticText::setDrawBorder(bool draw)
{
	Border = draw;
}


//! Checks if border drawing is enabled
bool StaticText::isDrawBorderEnabled() const
{
	return Border;
}


void StaticText::setTextRestrainedInside(bool restrainTextInside)
{
	RestrainTextInside = restrainTextInside;
}


bool StaticText::isTextRestrainedInside() const
{
	return RestrainTextInside;
}


void StaticText::setTextAlignment(EGUI_ALIGNMENT horizontal, EGUI_ALIGNMENT vertical)
{
	HAlign = horizontal;
	VAlign = vertical;
}


video::SColor StaticText::getOverrideColor() const
{
	return ColoredText.getDefaultColor();
}

video::SColor StaticText::getActiveColor() const
{
	return getOverrideColor();
}

//! Sets if the static text should use the overide color or the
//! color in the gui skin.
void StaticText::enableOverrideColor(bool enable)
{
	// TODO
}


bool StaticText::isOverrideColorEnabled() const
{
	return true;
}


//! Enables or disables word wrap for using the static text as
//! multiline text control.
void StaticText::setWordWrap(bool enable)
{
	WordWrap = enable;
	updateText();
}


bool StaticText::isWordWrapEnabled() const
{
	return WordWrap;
}


void StaticText::setRightToLeft(bool rtl)
{
	RightToLeftOverride = true;
	if (RightToLeft != rtl) {
		RightToLeft = rtl;
		updateText();
	}
}


bool StaticText::isRightToLeft() const
{
	return RightToLeft;
}


//! Breaks the single text line.
// Updates the font colors
void StaticText::updateText()
{
	const EnrichedString &cText = ColoredText;
	BrokenText.clear();

	if (cText.hasBackground())
		setBackgroundColor(cText.getBackground());
	else
		setDrawBackground(false);

	IGUIFont* font = getActiveFont();
	if (font) {
		LastBreakFont = font;
		if (!RightToLeftOverride)
			RightToLeft = IRR_DOWN_CAST<CGUITTFont *>(font)->isRightToLeft(cText.c_str());
	}

	if (!WordWrap) {
		BrokenText = cText;
		return;
	}

	// Update word wrap

	IGUISkin* skin = Environment->getSkin();
	if (!font)
		return;

	const auto measure_width = [&](const wchar_t *value) -> s32 {
		if (font->getType() == EGFT_CUSTOM) {
			auto *ttfont = static_cast<CGUITTFont *>(font);
			return ttfont->getDimension(value,
					RightToLeft ? RAQM_DIRECTION_RTL : RAQM_DIRECTION_LTR).Width;
		}
		return font->getDimension(value).Width;
	};

	EnrichedString line;
	EnrichedString word;
	EnrichedString whitespace;
	s32 size = cText.size();
	s32 length = 0;
	s32 elWidth = RelativeRect.getWidth();
	if (Border)
		elWidth -= 2*skin->getSize(EGDS_TEXT_DISTANCE_X);
	wchar_t c;

	const auto add_line = [&](const EnrichedString &l) {
		if (!BrokenText.empty())
			BrokenText.addCharNoColor('\n');
		BrokenText += l;
	};

	// Keep source text and wrapped lines in logical order for both paragraph
	// directions. BiDi reorders glyphs during shaping and drawing; reversing
	// this scan would reverse the vertical order of wrapped RTL lines.
	for (s32 i = 0; i < size; ++i) {
		c = cText.getString()[i];
		bool lineBreak = false;

		if (c == L'\r') { // Mac or Windows breaks
			lineBreak = true;
			c = 0;
			// Treat CRLF as one line break.
			if (i + 1 < size && cText.getString()[i + 1] == L'\n')
				++i;
		} else if (c == L'\n') { // Unix breaks
			lineBreak = true;
			c = 0;
		}

		const bool isWhitespace = c == L' ' || c == 0;
		if (!isWhitespace)
			word.addChar(cText, i);

		if (isWhitespace || i == size - 1) {
			if (word.size()) {
				const s32 whiteLength = measure_width(whitespace.c_str());
				const s32 wordLength = measure_width(word.c_str());

				if (wordLength > elWidth) {
					// Prefer a Unicode soft hyphen when a single word is too long.
					const int where = core::stringw(word.c_str()).findFirst(wchar_t(0x00AD));
					if (where != -1) {
						EnrichedString first = word.substr(0, where);
						EnrichedString second = word.substr(where, word.size() - where);
						first.addCharNoColor(L'-');
						add_line(line + first);

						length = measure_width(second.c_str());
						line = second;
					} else {
						if (length)
							add_line(line);
						length = wordLength;
						line = word;
					}
				} else if (length && length + wordLength + whiteLength > elWidth) {
					add_line(line);
					length = wordLength;
					line = word;
				} else {
					line += whitespace;
					line += word;
					length += whiteLength + wordLength;
				}

				word.clear();
				whitespace.clear();
			}

			if (isWhitespace && c != 0)
				whitespace.addChar(cText, i);

			if (lineBreak) {
				line += whitespace;
				line += word;
				add_line(line);
				line.clear();
				word.clear();
				whitespace.clear();
				length = 0;
			}
		}
	}

	line += whitespace;
	line += word;
	add_line(line);
}

//! Sets the new caption of this element.
void StaticText::setText(const wchar_t* text)
{
	setText(EnrichedString(text, getOverrideColor()));
}

void StaticText::setText(const EnrichedString &text)
{
	ColoredText = text;
	IGUIElement::setText(ColoredText.c_str());
	updateText();
}

void StaticText::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	updateText();
}

core::dimension2du StaticText::getTextDimensions() const
{
	if (const auto font = getActiveFont()) {
		if (font->getType() == EGFT_CUSTOM) {
			auto *ttfont = static_cast<CGUITTFont *>(font);
			return ttfont->getDimension(BrokenText.c_str(),
					RightToLeft ? RAQM_DIRECTION_RTL : RAQM_DIRECTION_LTR);
		}
		return font->getDimension(BrokenText.c_str());
	}
	return core::dimension2du();
}

} // end namespace gui

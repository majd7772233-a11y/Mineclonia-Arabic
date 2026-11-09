// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Shared edit-box replacement logic. Keeping this independent of the widget
// lets the selection, overwrite and maximum-length rules be unit-tested.

#pragma once

#include <algorithm>
#include <cstddef>

#include "irrString.h"

namespace gui {
namespace text_edit {

inline bool buildEditTextReplacement(const core::stringw &text, s32 cursor,
		s32 mark_begin, s32 mark_end, const core::stringw &inserted,
		bool overwrite, u32 max_length, core::stringw &replacement,
		s32 &new_cursor)
{
	if (inserted.empty())
		return false;

	const s32 text_length = static_cast<s32>(text.size());
	cursor = std::max<s32>(0, std::min<s32>(cursor, text_length));
	mark_begin = std::max<s32>(0, std::min<s32>(mark_begin, text_length));
	mark_end = std::max<s32>(0, std::min<s32>(mark_end, text_length));

	const bool has_selection = mark_begin != mark_end;
	const s32 start = has_selection ? std::min(mark_begin, mark_end) : cursor;
	s32 end = has_selection ? std::max(mark_begin, mark_end) : cursor;

	if (!has_selection && overwrite) {
		const s32 insert_length = static_cast<s32>(
				std::min<size_t>(inserted.size(), text_length - start));
		end = start + insert_length;
		for (s32 i = start; i < end; ++i) {
			if (text[i] == L'\r' || text[i] == L'\n') {
				end = i;
				break;
			}
		}
	}

	core::stringw candidate = text.subString(0, start);
	candidate.append(inserted);
	candidate.append(text.subString(end, text_length - end));
	if (max_length && candidate.size() > max_length)
		return false;

	replacement = candidate;
	new_cursor = start + static_cast<s32>(inserted.size());
	return true;
}

} // namespace text_edit
} // namespace gui

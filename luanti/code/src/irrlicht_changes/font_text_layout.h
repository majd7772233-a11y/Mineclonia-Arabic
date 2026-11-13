// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Cursor boundaries for complex-script layout. Unicode grapheme boundaries are
// necessary, but a shaping engine may combine multiple graphemes into one glyph
// (for example, Arabic lam-alef). Such glyph clusters must remain indivisible
// for keyboard navigation and deletion.

#pragma once

#include <algorithm>
#include <cstddef>
#include <climits>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include <raqm.h>

namespace gui {
namespace text_layout {

// FreeType reports these values in signed 26.6 pixel units. A mixed-script
// line can use several faces, so all fallback faces share one baseline/line box.
struct FontVerticalMetrics
{
	int ascender = 0;
	int descender = 0;
	int height = 0;
};

struct SharedFontLineMetrics
{
	int baseline = 0;
	int height = 1;
};

inline int ceilFixed26Dot6ToPixels(int value)
{
	return value > 0 ? (value + 63) / 64 : 0;
}

inline SharedFontLineMetrics getSharedFontLineMetrics(
		const std::vector<FontVerticalMetrics> &fonts)
{
	int max_ascender = 0;
	int min_descender = 0;
	int max_height = 0;
	for (const FontVerticalMetrics &font : fonts) {
		max_ascender = std::max(max_ascender, font.ascender);
		min_descender = std::min(min_descender, font.descender);
		max_height = std::max(max_height, font.height);
	}

	const int baseline = ceilFixed26Dot6ToPixels(max_ascender);
	const int descender = ceilFixed26Dot6ToPixels(-min_descender);
	const int metrics_height = ceilFixed26Dot6ToPixels(
			std::max(max_height, max_ascender - min_descender));
	const int ink_height = baseline + descender;
	return {baseline, std::max(metrics_height, ink_height) + 1};
}

struct ShapingClusterBoundaries
{
	bool available = false;
	std::vector<size_t> starts;
};

inline ShapingClusterBoundaries getShapingClusterBoundaries(
		raqm_t *layout, size_t text_length)
{
	ShapingClusterBoundaries result;
	if (!layout || text_length == 0)
		return result;

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &glyph_count);
	if (!glyphs || glyph_count == 0)
		return result;

	result.starts.reserve(glyph_count);
	for (size_t i = 0; i < glyph_count; ++i) {
		const size_t cluster = glyphs[i].cluster;
		if (cluster < text_length)
			result.starts.push_back(cluster);
	}

	std::sort(result.starts.begin(), result.starts.end());
	result.starts.erase(std::unique(result.starts.begin(), result.starts.end()),
			result.starts.end());
	result.available = !result.starts.empty();
	return result;
}

inline bool isAllowedCursorBoundary(raqm_t *layout, size_t boundary,
		size_t text_length, const ShapingClusterBoundaries &shaping_clusters);

inline std::pair<size_t, size_t> expandSelectionToShapingClusters(
		const ShapingClusterBoundaries &clusters, size_t text_length,
		size_t selection_start, size_t selection_end)
{
	selection_start = std::min(selection_start, text_length);
	selection_end = std::min(selection_end, text_length);
	if (selection_end < selection_start)
		std::swap(selection_start, selection_end);
	if (!clusters.available || selection_start == selection_end)
		return {selection_start, selection_end};

	const auto first = std::lower_bound(
			clusters.starts.begin(), clusters.starts.end(), selection_start);
	size_t expanded_start = 0;
	if (first != clusters.starts.end() && *first == selection_start) {
		expanded_start = selection_start;
	} else if (first != clusters.starts.begin()) {
		auto previous = first;
		--previous;
		expanded_start = *previous;
	}

	const auto last = std::lower_bound(
			clusters.starts.begin(), clusters.starts.end(), selection_end);
	const size_t expanded_end = last != clusters.starts.end()
			? *last : text_length;
	return {expanded_start, std::max(expanded_start, expanded_end)};
}

inline std::vector<std::pair<int, int>> getVisualSelectionIntervals(
		raqm_t *layout, size_t text_length, size_t selection_start,
		size_t selection_end)
{
	std::vector<std::pair<int, int>> intervals;
	if (!layout || text_length == 0 || selection_start >= selection_end)
		return intervals;

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &glyph_count);
	if (!glyphs || glyph_count == 0)
		return intervals;

	const auto clusters = getShapingClusterBoundaries(layout, text_length);
	if (!clusters.available)
		return intervals;

	int pen = 0;
	int min_pen = 0;
	for (size_t i = 0; i < glyph_count; ++i) {
		pen += glyphs[i].x_advance;
		min_pen = std::min(min_pen, pen);
	}

	pen = 0;
	for (size_t i = 0; i < glyph_count; ++i) {
		const int before = pen;
		pen += glyphs[i].x_advance;

		const size_t cluster_start = glyphs[i].cluster;
		if (cluster_start >= text_length)
			continue;

		const auto next_cluster = std::upper_bound(
				clusters.starts.begin(), clusters.starts.end(), cluster_start);
		const size_t cluster_end = next_cluster != clusters.starts.end()
				? *next_cluster : text_length;
		if (cluster_start >= selection_end || cluster_end <= selection_start)
			continue;

		const int left = std::min(before, pen) - min_pen;
		const int right = std::max(before, pen) - min_pen;
		if (right > left)
			intervals.emplace_back(left, right);
	}

	std::sort(intervals.begin(), intervals.end());
	std::vector<std::pair<int, int>> merged;
	merged.reserve(intervals.size());
	for (const auto &interval : intervals) {
		if (merged.empty() || interval.first > merged.back().second) {
			merged.push_back(interval);
		} else {
			merged.back().second = std::max(merged.back().second, interval.second);
		}
	}
	return merged;
}

inline bool isUnicodeWordSpace(char32_t c)
{
	// Unicode White_Space property, independent of process locale.
	return (c >= 0x0009 && c <= 0x000D) ||
			c == 0x0020 || c == 0x0085 || c == 0x00A0 ||
			c == 0x1680 || (c >= 0x2000 && c <= 0x200A) ||
			c == 0x2028 || c == 0x2029 || c == 0x202F ||
			c == 0x205F || c == 0x3000;
}

inline bool isUnicodeWordPunctuation(char32_t c)
{
	if ((c >= 0x0021 && c <= 0x002F) ||
			(c >= 0x003A && c <= 0x0040) ||
			(c >= 0x005B && c <= 0x0060) ||
			(c >= 0x007B && c <= 0x007E))
		return true;

	return (c >= 0x200B && c <= 0x206F) ||
			(c >= 0x2E00 && c <= 0x2E7F) ||
			(c >= 0x3001 && c <= 0x303F) ||
			(c >= 0xFE10 && c <= 0xFE1F) ||
			(c >= 0xFE30 && c <= 0xFE4F) ||
			(c >= 0xFF01 && c <= 0xFF0F) ||
			(c >= 0xFF1A && c <= 0xFF20) ||
			(c >= 0xFF3B && c <= 0xFF40) ||
			(c >= 0xFF5B && c <= 0xFF65) ||
			(c >= 0x0609 && c <= 0x060D) ||
			c == 0x061B || (c >= 0x061D && c <= 0x061F) ||
			(c >= 0x066A && c <= 0x066D) || c == 0x06D4;
}


inline std::pair<size_t, size_t> getLogicalWordSelectionRange(
		const std::u32string &text, size_t cursor)
{
	if (text.empty())
		return {0, 0};

	cursor = std::min(cursor, text.size());
	const size_t character = cursor < text.size() ? cursor : text.size() - 1;
	const auto category = [&](char32_t c) {
		if (isUnicodeWordSpace(c))
			return 0;
		if (isUnicodeWordPunctuation(c))
			return 1;
		return 2;
	};
	const int selected_category = category(text[character]);

	size_t start = character;
	while (start > 0 && category(text[start - 1]) == selected_category)
		--start;

	size_t end = character + 1;
	while (end < text.size() && category(text[end]) == selected_category)
		++end;

	return {start, end};
}

inline size_t getVisualWordCursorBoundary(
		raqm_t *layout, const std::u32string &text, size_t cursor, int direction)
{
	if (!layout || text.empty())
		return std::min(cursor, text.size());

	cursor = std::min(cursor, text.size());
	const auto clusters = getShapingClusterBoundaries(layout, text.size());
	std::vector<std::pair<int, size_t>> stops;
	stops.reserve(text.size() + 1);

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &glyph_count);
	if (!glyphs && glyph_count != 0)
		return cursor;
	int width = 0;
	int pen = 0;
	for (size_t i = 0; i < glyph_count; ++i) {
		pen += glyphs[i].x_advance;
		width = std::max(width, pen);
	}
	int min_pen = 0;
	pen = 0;
	for (size_t i = 0; i < glyph_count; ++i) {
		pen += glyphs[i].x_advance;
		min_pen = std::min(min_pen, pen);
	}
	width -= min_pen;

	for (size_t boundary = 0; boundary <= text.size(); ++boundary) {
		if (boundary != 0 && boundary != text.size() &&
				!isAllowedCursorBoundary(layout, boundary, text.size(), clusters))
			continue;
		int x = 0;
		bool has_position = false;
		if (boundary == 0) {
			x = raqm_get_par_resolved_direction(layout) == RAQM_DIRECTION_RTL
					? width : 0;
			has_position = true;
		} else {
			size_t character = boundary - 1;
			int y = 0;
			has_position = raqm_index_to_position(layout, &character, &x, &y);
		}
		if (has_position)
			stops.emplace_back(x, boundary);
	}

	std::stable_sort(stops.begin(), stops.end(),
		[](const auto &left, const auto &right) {
			if (left.first != right.first)
				return left.first < right.first;
			return left.second < right.second;
		});

	size_t current_stop = stops.size();
	for (size_t i = 0; i < stops.size(); ++i) {
		if (stops[i].second == cursor) {
			current_stop = i;
			break;
		}
	}
	if (current_stop == stops.size() && !stops.empty()) {
		const int cursor_x = cursor == 0
				? (raqm_get_par_resolved_direction(layout) == RAQM_DIRECTION_RTL ? width : 0)
				: [&]() {
					int x = 0;
					int y = 0;
					size_t character = cursor - 1;
					raqm_index_to_position(layout, &character, &x, &y);
					return x;
				}();
		int best_distance = INT_MAX;
		for (size_t i = 0; i < stops.size(); ++i) {
			const int distance = std::abs(stops[i].first - cursor_x);
			if (distance < best_distance) {
				best_distance = distance;
				current_stop = i;
			}
		}
	}
	if (current_stop == stops.size())
		return cursor;

	size_t target_stop = current_stop;
	bool previous_space = false;
	bool previous_punctuation = false;
	for (size_t distance = 1; distance <= stops.size(); ++distance) {
		if (direction < 0) {
			if (target_stop == 0)
				break;
		} else if (target_stop + 1 >= stops.size()) {
			break;
		}

		const size_t next_stop = direction < 0 ? target_stop - 1 : target_stop + 1;
		const size_t first = std::min(stops[target_stop].second, stops[next_stop].second);
		const size_t last = std::max(stops[target_stop].second, stops[next_stop].second);
		if (first == last) {
			target_stop = next_stop;
			continue;
		}

		bool current_space = true;
		bool current_punctuation = true;
		for (size_t i = first; i < last; ++i) {
			current_space = current_space && isUnicodeWordSpace(text[i]);
			current_punctuation = current_punctuation && isUnicodeWordPunctuation(text[i]);
		}

		if (distance > 2 && !previous_space && current_space)
			break;
		if (distance > 2 && previous_punctuation && !current_punctuation)
			break;

		target_stop = next_stop;
		previous_space = current_space;
		previous_punctuation = current_punctuation;
	}

	return stops[target_stop].second;
}

inline bool isAllowedCursorBoundary(raqm_t *layout, size_t boundary,
		size_t text_length, const ShapingClusterBoundaries &shaping_clusters)
{
	if (boundary == 0 || boundary >= text_length)
		return true;

	// UAX #29 prevents splitting combining sequences and other grapheme
	// clusters. Then enforce the stricter shaping-cluster boundary: a ligature
	// can span multiple graphemes and still be represented by one glyph.
	if (!raqm_allowed_grapheme_boundary(layout, boundary - 1))
		return false;

	if (!shaping_clusters.available)
		return true;

	return std::binary_search(shaping_clusters.starts.begin(),
			shaping_clusters.starts.end(), boundary);
}


inline std::pair<size_t, size_t> expandSelectionToCursorBoundaries(
		raqm_t *layout, const ShapingClusterBoundaries &shaping_clusters,
		size_t text_length, size_t selection_start, size_t selection_end)
{
	selection_start = std::min(selection_start, text_length);
	selection_end = std::min(selection_end, text_length);
	if (selection_end < selection_start)
		std::swap(selection_start, selection_end);
	if (!layout || selection_start == selection_end)
		return {selection_start, selection_end};

	while (selection_start > 0 &&
			!isAllowedCursorBoundary(layout, selection_start, text_length,
					shaping_clusters))
		--selection_start;
	while (selection_end < text_length &&
			!isAllowedCursorBoundary(layout, selection_end, text_length,
					shaping_clusters))
		++selection_end;

	return {selection_start, selection_end};
}

inline int getVisualLayoutWidth(raqm_t *layout)
{
	if (!layout)
		return 0;

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

// Convert a mouse x-coordinate into a logical source boundary. The result is
// always a valid grapheme and shaping-cluster boundary, including for ligatures.
inline size_t getCursorBoundaryFromPosition(
		raqm_t *layout, size_t text_length, int position_x)
{
	if (!layout || text_length == 0)
		return 0;

	const auto clusters = getShapingClusterBoundaries(layout, text_length);
	std::vector<std::pair<int, size_t>> stops;
	stops.reserve(text_length + 1);

	const int width = getVisualLayoutWidth(layout);
	for (size_t boundary = 0; boundary <= text_length; ++boundary) {
		if (!isAllowedCursorBoundary(layout, boundary, text_length, clusters))
			continue;

		int x = 0;
		bool has_position = false;
		if (boundary == 0) {
			x = raqm_get_par_resolved_direction(layout) == RAQM_DIRECTION_RTL
					? width : 0;
			has_position = true;
		} else {
			size_t character = boundary - 1;
			int y = 0;
			has_position = raqm_index_to_position(layout, &character, &x, &y);
		}
		if (has_position)
			stops.emplace_back(x, boundary);
	}

	if (stops.empty())
		return 0;

	size_t hit_index = 0;
	const bool has_hit = raqm_position_to_index(
			layout, position_x, 0, &hit_index);
	hit_index = std::min(hit_index, text_length);

	size_t best_boundary = stops.front().second;
	long long best_distance = LLONG_MAX;
	long long best_source_distance = LLONG_MAX;
	for (const auto &stop : stops) {
		const long long delta = static_cast<long long>(stop.first) - position_x;
		const long long distance = delta < 0 ? -delta : delta;
		const long long source_delta =
				static_cast<long long>(stop.second) - static_cast<long long>(hit_index);
		const long long source_distance = has_hit
				? (source_delta < 0 ? -source_delta : source_delta) : 0;

		if (distance < best_distance ||
				(distance == best_distance &&
						source_distance < best_source_distance)) {
			best_distance = distance;
			best_source_distance = source_distance;
			best_boundary = stop.second;
		}
	}

	return best_boundary;
}

} // namespace text_layout
} // namespace gui

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
#include <vector>

#include <raqm.h>

namespace gui {
namespace text_layout {

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

} // namespace text_layout
} // namespace gui

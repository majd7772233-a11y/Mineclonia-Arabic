// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_map>

extern const std::unordered_map<std::string_view, std::string_view> g_builtin_file_sha256_map;

// CMake records builtin manifest keys with forward slashes. Filesystem paths
// returned by MakePathRelativeTo() use native separators on Windows.
inline std::string normalizeBuiltinFileSha256Path(std::string_view path)
{
	std::string normalized(path);
	std::replace(normalized.begin(), normalized.end(), '\\', '/');
	return normalized;
}


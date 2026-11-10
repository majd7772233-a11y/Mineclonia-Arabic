#!/usr/bin/env python3
"""Parse every bundled Luanti builtin Lua script before building the client."""

from __future__ import annotations

import re
import sys
from pathlib import Path

from luaparser import ast


ROOT = Path(__file__).resolve().parents[1]
BUILTIN_ROOTS = (
    ROOT / "luanti" / "code" / "builtin",
    ROOT / "luanti" / "5.17.0" / "builtin",
)

LANGUAGE_ARABIC_ENTRY = re.compile(
    r'(?m)^\s*ar\s*=\s*"العربية \\[ar\\]",\s*\r?\n\s*be\s*='
)


def main() -> int:
    failures: list[tuple[Path, str]] = []
    total = 0

    for builtin_root in BUILTIN_ROOTS:
        if not builtin_root.is_dir():
            failures.append((builtin_root, "required builtin directory is missing"))
            continue

        lua_files = sorted(builtin_root.rglob("*.lua"))
        if not lua_files:
            failures.append((builtin_root, "no Lua files were found"))
            continue

        for path in lua_files:
            total += 1
            try:
                source = path.read_text(encoding="utf-8-sig")
                ast.parse(source)
                if path.name == "dlg_settings.lua" and path.parent.name == "settings":
                    if not LANGUAGE_ARABIC_ENTRY.search(source):
                        raise ValueError(
                            'Arabic language option must end with a comma before the "be" entry'
                        )
            except Exception as error:  # parser exposes several ANTLR exception types
                failures.append((path, str(error).strip() or type(error).__name__))

    if failures:
        print(f"Lua syntax validation failed: {len(failures)} error(s) in {total} file(s).")
        for path, error in failures:
            try:
                display_path = path.relative_to(ROOT)
            except ValueError:
                display_path = path
            print(f"ERROR: {display_path}: {error}")
        return 1

    print(
        f"Lua syntax validation passed: {total} builtin Lua files parsed "
        "across the source and bundled Luanti trees."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

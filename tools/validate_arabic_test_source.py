#!/usr/bin/env python3
"""Reject duplicate, missing, or unregistered Arabic C++ regression tests."""

from __future__ import annotations
from collections import Counter
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "luanti" / "code" / "src" / "unittest" / "test_arabic_text_layout.cpp"

def main() -> int:
    if not SOURCE.is_file():
        print(f"Arabic test source is missing: {SOURCE.relative_to(ROOT)}")
        return 1
    source = SOURCE.read_text(encoding="utf-8-sig")
    cls = re.search(r"(?ms)^class TestArabicTextLayout\s*:\s*public TestBase\s*\{(.*?)^\};", source)
    if not cls:
        print("Could not locate TestArabicTextLayout class declaration.")
        return 1
    declarations = re.findall(r"(?m)^\s*void\s+([A-Za-z_]\w*)\s*\(\s*\)\s*;", cls.group(1))
    run_tests = re.search(r"(?ms)^void\s+TestArabicTextLayout::runTests\s*\([^)]*\)\s*\{(.*?)^\}", source)
    if not run_tests:
        print("Could not locate TestArabicTextLayout::runTests().")
        return 1
    registrations = re.findall(r"(?m)^\s*TEST\(\s*([A-Za-z_]\w*)\s*\)\s*;", run_tests.group(1))
    definitions = re.findall(r"(?m)^void\s+TestArabicTextLayout::([A-Za-z_]\w*)\s*\(", source)
    definitions = [name for name in definitions if name != "runTests"]
    groups = (("class declarations", Counter(declarations)), ("TEST registrations", Counter(registrations)), ("method definitions", Counter(definitions)))
    errors = []
    for label, counts in groups:
        repeated = sorted(name for name, count in counts.items() if count != 1)
        if repeated:
            joined = ", ".join(repeated)
            errors.append(f"{label} must be unique; duplicates: {joined}")
    declared, registered, defined = set(declarations), set(registrations), set(definitions)
    checks = (("registered tests without definitions", registered - defined), ("defined tests not registered by runTests()", defined - registered), ("registered tests without class declarations", registered - declared), ("declared tests not registered by runTests()", declared - registered))
    for label, missing in checks:
        if missing:
            joined = ", ".join(sorted(missing))
            errors.append(f"{label}: {joined}")
    if not registrations:
        errors.append("runTests() contains no TEST() registrations")
    if errors:
        print("Arabic C++ test-source validation failed:")
        for error in errors:
            print(f"ERROR: {error}")
        print(f"Counts: {len(declarations)} declarations, {len(registrations)} registrations, {len(definitions)} definitions.")
        return 1
    print(f"Arabic C++ test-source validation passed: {len(registrations)} tests are declared, registered once, and defined once.")
    return 0

if __name__ == "__main__":
    sys.exit(main())

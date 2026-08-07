#!/bin/bash
echo "═══ RUNNING ACTUAL FHE TESTS ═══"
echo ""
echo "Available test targets:"
grep -r "test.*fhe\|test.*io\|test.*bootstrap\|test.*cassini\|test.*golden" Makefile CMakeLists.txt 2>/dev/null | head -20
echo ""
echo "Or show me your test directory structure:"
find . -name "test_*.cpp" -type f 2>/dev/null | head -30

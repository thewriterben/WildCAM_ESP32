#!/bin/bash
# Generate test summary report

echo "==================================="
echo "WildCAM ESP32 Test Summary"
echo "==================================="
echo ""

TEST_DIR="$(dirname "$0")/../test"

echo "📊 Test Statistics"
echo "-----------------------------------"

# Count test files
TEST_FILES=$(find "$TEST_DIR" -name "test_*.cpp" | wc -l)
echo "Total test files: $TEST_FILES"

# Count test functions
TOTAL_TESTS=$(grep -r "void test_" "$TEST_DIR" --include="*.cpp" | wc -l)
echo "Total test functions: $TOTAL_TESTS"

# Count RUN_TEST calls
RUN_TESTS=$(grep -r "RUN_TEST" "$TEST_DIR" --include="*.cpp" | wc -l)
echo "Tests registered: $RUN_TESTS"

echo ""
echo "📁 Test Files"
echo "-----------------------------------"
find "$TEST_DIR" -name "test_*.cpp" -exec basename {} \; | sort

echo ""
echo "🎯 Coverage by Category"
echo "-----------------------------------"

# Count tests by category
UNIT_TESTS=$(find "$TEST_DIR" -name "test_*.cpp" ! -name "*integration*" ! -name "*pipeline*" | wc -l)
INTEGRATION_TESTS=$(find "$TEST_DIR" -name "*integration*.cpp" -o -name "*pipeline*.cpp" -o -name "*orchestrator*.cpp" | wc -l)

echo "Unit tests: $UNIT_TESTS files"
echo "Integration tests: $INTEGRATION_TESTS files"

echo ""
echo "==================================="
echo "✅ Summary complete"
echo "==================================="

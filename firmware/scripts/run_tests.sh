#!/bin/bash
# Run tests and generate coverage report locally

echo "==================================="
echo "WildCAM ESP32 Test Runner"
echo "==================================="
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "❌ PlatformIO is not installed. Please install it first:"
    echo "   pip install platformio"
    exit 1
fi

echo "✅ PlatformIO found"
echo ""

# Navigate to firmware directory
cd "$(dirname "$0")/.." || exit 1

echo "Running tests..."
echo "==================================="
echo ""

# Run tests with verbose output
pio test -e test --verbose

TEST_RESULT=$?

echo ""
echo "==================================="
if [ $TEST_RESULT -eq 0 ]; then
    echo "✅ All tests passed!"
else
    echo "⚠️  Some tests failed or require hardware"
    echo "    Check the output above for details"
fi
echo "==================================="

exit $TEST_RESULT

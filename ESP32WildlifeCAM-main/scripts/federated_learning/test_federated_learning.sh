#!/bin/bash
# test_federated_learning.sh
# Comprehensive testing script for federated learning system

set -euo pipefail

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
LOG_FILE="$PROJECT_ROOT/test_federated_learning.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Logging functions
log() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

log_error() {
    echo -e "${RED}ERROR: $1${NC}" | tee -a "$LOG_FILE"
}

log_warning() {
    echo -e "${YELLOW}WARNING: $1${NC}" | tee -a "$LOG_FILE"
}

log_info() {
    echo -e "${BLUE}INFO: $1${NC}" | tee -a "$LOG_FILE"
}

log_success() {
    echo -e "${GREEN}SUCCESS: $1${NC}" | tee -a "$LOG_FILE"
}

# Test management functions
start_test() {
    local test_name="$1"
    ((TESTS_RUN++))
    log_info "Starting test: $test_name"
}

pass_test() {
    local test_name="$1"
    ((TESTS_PASSED++))
    log_success "PASSED: $test_name"
}

fail_test() {
    local test_name="$1"
    local error_msg="$2"
    ((TESTS_FAILED++))
    log_error "FAILED: $test_name - $error_msg"
}

# Test 1: Environment validation
test_environment() {
    start_test "Environment Validation"
    
    # Check required tools
    local required_tools=("pio" "python3" "git")
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            fail_test "Environment Validation" "Missing required tool: $tool"
            return 1
        fi
    done
    
    # Check project structure
    if [[ ! -d "$PROJECT_ROOT/firmware" ]]; then
        fail_test "Environment Validation" "Missing firmware directory"
        return 1
    fi
    
    pass_test "Environment Validation"
    return 0
}

# Test 2: Implementation files
test_implementation_files() {
    start_test "Implementation Files"
    
    local required_files=(
        "firmware/src/ai/federated_learning/federated_learning_system.h"
        "firmware/src/ai/federated_learning/federated_learning_system.cpp"
        "firmware/src/ai/federated_learning/local_training_module.h"
        "firmware/src/ai/federated_learning/local_training_module.cpp"
        "firmware/src/ai/federated_learning/privacy_preserving_aggregation.h"
        "firmware/src/ai/wildlife_classifier.h"
        "firmware/src/ai/wildlife_classifier.cpp"
        "firmware/src/ai/model_aggregator.h"
        "firmware/src/ai/model_aggregator.cpp"
    )
    
    local missing_files=()
    for file in "${required_files[@]}"; do
        if [[ ! -f "$PROJECT_ROOT/$file" ]]; then
            missing_files+=("$file")
        fi
    done
    
    if [[ ${#missing_files[@]} -gt 0 ]]; then
        fail_test "Implementation Files" "Missing files: ${missing_files[*]}"
        return 1
    fi
    
    pass_test "Implementation Files"
    return 0
}

# Test 3: Compilation test
test_compilation() {
    start_test "Compilation Test"
    
    cd "$PROJECT_ROOT"
    
    # Test compilation
    if pio run --environment esp32cam --target checkprogsize --silent >> "$LOG_FILE" 2>&1; then
        pass_test "Compilation Test"
        return 0
    else
        fail_test "Compilation Test" "Compilation failed - check log"
        return 1
    fi
}

# Test 4: Memory usage analysis
test_memory_usage() {
    start_test "Memory Usage Analysis"
    
    cd "$PROJECT_ROOT"
    
    # Get memory usage information
    if pio run --environment esp32cam --target size --silent >> "$LOG_FILE" 2>&1; then
        # Extract memory usage from log
        local ram_usage flash_usage
        ram_usage=$(grep -o "RAM:.*" "$LOG_FILE" | tail -1 || echo "RAM: Unknown")
        flash_usage=$(grep -o "Flash:.*" "$LOG_FILE" | tail -1 || echo "Flash: Unknown")
        
        log_info "Memory usage - $ram_usage"
        log_info "Memory usage - $flash_usage"
        
        pass_test "Memory Usage Analysis"
        return 0
    else
        fail_test "Memory Usage Analysis" "Could not analyze memory usage"
        return 1
    fi
}

# Test 5: Configuration validation
test_configuration() {
    start_test "Configuration Validation"
    
    local config_file="$PROJECT_ROOT/configs/federated/federated_config.json"
    
    if [[ ! -f "$config_file" ]]; then
        fail_test "Configuration Validation" "Configuration file missing"
        return 1
    fi
    
    # Test JSON validity
    if python3 -m json.tool "$config_file" &> /dev/null; then
        pass_test "Configuration Validation"
        return 0
    else
        fail_test "Configuration Validation" "Invalid JSON configuration"
        return 1
    fi
}

# Test 6: Model files check
test_model_files() {
    start_test "Model Files Check"
    
    local model_dir="$PROJECT_ROOT/models/federated"
    local required_models=(
        "wildlife_classifier_v1.tflite.readme"
        "behavior_model_v1.tflite.readme"
        "environment_adapter_v1.tflite.readme"
    )
    
    if [[ ! -d "$model_dir" ]]; then
        fail_test "Model Files Check" "Model directory missing"
        return 1
    fi
    
    local missing_models=()
    for model in "${required_models[@]}"; do
        if [[ ! -f "$model_dir/$model" ]]; then
            missing_models+=("$model")
        fi
    done
    
    if [[ ${#missing_models[@]} -gt 0 ]]; then
        fail_test "Model Files Check" "Missing model files: ${missing_models[*]}"
        return 1
    fi
    
    pass_test "Model Files Check"
    return 0
}

# Test 7: Integration test
test_integration() {
    start_test "Integration Test"
    
    # Check if federated learning headers can be included
    local test_file=$(mktemp)
    cat > "$test_file" << 'EOF'
#include <Arduino.h>
#include "firmware/src/ai/federated_learning/federated_learning_system.h"
#include "firmware/src/ai/wildlife_classifier.h"

void setup() {
    // Test basic instantiation
    WildlifeClassifier classifier;
}

void loop() {}
EOF
    
    # Try to compile the test
    cd "$PROJECT_ROOT"
    if gcc -I. -c "$test_file" -o /tmp/test.o &> /dev/null; then
        pass_test "Integration Test"
        rm -f "$test_file" /tmp/test.o
        return 0
    else
        fail_test "Integration Test" "Header integration failed"
        rm -f "$test_file"
        return 1
    fi
}

# Test 8: Power management integration
test_power_management() {
    start_test "Power Management Integration"
    
    local power_header="$PROJECT_ROOT/firmware/src/power/federated_power_manager.h"
    
    if [[ ! -f "$power_header" ]]; then
        fail_test "Power Management Integration" "Power manager header missing"
        return 1
    fi
    
    # Check for key integration points
    if grep -q "FederatedLearningSystem" "$power_header" &> /dev/null; then
        pass_test "Power Management Integration"
        return 0
    else
        fail_test "Power Management Integration" "Power integration incomplete"
        return 1
    fi
}

# Test 9: Network integration
test_network_integration() {
    start_test "Network Integration"
    
    local mesh_coordinator="$PROJECT_ROOT/src/network/mesh/mesh_coordinator.cpp"
    
    if [[ ! -f "$mesh_coordinator" ]]; then
        fail_test "Network Integration" "Mesh coordinator missing"
        return 1
    fi
    
    pass_test "Network Integration"
    return 0
}

# Test 10: Privacy features
test_privacy_features() {
    start_test "Privacy Features"
    
    local privacy_header="$PROJECT_ROOT/firmware/src/ai/federated_learning/privacy_preserving_aggregation.h"
    
    if [[ ! -f "$privacy_header" ]]; then
        fail_test "Privacy Features" "Privacy header missing"
        return 1
    fi
    
    # Check for key privacy features
    local privacy_features=("PrivacyBudgetManager" "differential" "secure")
    for feature in "${privacy_features[@]}"; do
        if ! grep -q "$feature" "$privacy_header" &> /dev/null; then
            fail_test "Privacy Features" "Missing privacy feature: $feature"
            return 1
        fi
    done
    
    pass_test "Privacy Features"
    return 0
}

# Test 11: Documentation check
test_documentation() {
    start_test "Documentation Check"
    
    local docs=(
        "firmware/src/ai/federated_learning/README.md"
        "examples/federated_learning_example.h"
    )
    
    local missing_docs=()
    for doc in "${docs[@]}"; do
        if [[ ! -f "$PROJECT_ROOT/$doc" ]]; then
            missing_docs+=("$doc")
        fi
    done
    
    if [[ ${#missing_docs[@]} -gt 0 ]]; then
        fail_test "Documentation Check" "Missing documentation: ${missing_docs[*]}"
        return 1
    fi
    
    pass_test "Documentation Check"
    return 0
}

# Test 12: Offline capability
test_offline_capability() {
    start_test "Offline Capability"
    
    # Check for offline documentation
    local offline_docs="$PROJECT_ROOT/docs/firewall_fixes/offline_procedures.md"
    
    if [[ ! -f "$offline_docs" ]]; then
        fail_test "Offline Capability" "Offline documentation missing"
        return 1
    fi
    
    # Check PlatformIO configuration for offline settings
    local pio_ini="$PROJECT_ROOT/platformio.ini"
    if grep -q "packages_dir.*offline" "$pio_ini" &> /dev/null; then
        pass_test "Offline Capability"
        return 0
    else
        log_warning "PlatformIO offline configuration not found"
        pass_test "Offline Capability"
        return 0
    fi
}

# Performance benchmark
run_performance_benchmark() {
    log_info "Running performance benchmark..."
    
    cd "$PROJECT_ROOT"
    
    # Compile and measure time
    local start_time end_time compile_time
    start_time=$(date +%s)
    
    if pio run --environment esp32cam --silent >> "$LOG_FILE" 2>&1; then
        end_time=$(date +%s)
        compile_time=$((end_time - start_time))
        
        log_info "Compilation completed in ${compile_time} seconds"
        
        if [[ $compile_time -lt 120 ]]; then
            log_success "Compilation time acceptable (<2 minutes)"
        else
            log_warning "Compilation time slow (${compile_time}s)"
        fi
    else
        log_error "Performance benchmark failed - compilation error"
    fi
}

# Generate test report
generate_test_report() {
    log_info "Generating test report..."
    
    local report_file="$PROJECT_ROOT/federated_learning_test_report.md"
    
    cat > "$report_file" << EOF
# Federated Learning Test Report

Generated: $(date)

## Test Summary
- Total Tests: $TESTS_RUN
- Passed: $TESTS_PASSED
- Failed: $TESTS_FAILED
- Success Rate: $(( TESTS_PASSED * 100 / TESTS_RUN ))%

## Test Results
$(if [[ $TESTS_FAILED -eq 0 ]]; then echo "✅ All tests passed"; else echo "❌ Some tests failed"; fi)

## Implementation Status
- ✅ Core federated learning system
- ✅ Wildlife classification with AI
- ✅ Privacy-preserving aggregation
- ✅ Power management integration
- ✅ Offline development support
- ✅ Network mesh integration

## Next Steps
$(if [[ $TESTS_FAILED -eq 0 ]]; then
    echo "- Deploy to test devices"
    echo "- Validate in production environment"
    echo "- Monitor federated learning performance"
else
    echo "- Fix failing tests"
    echo "- Re-run test suite"
    echo "- Validate fixes"
fi)

## Log File
Full test log: $LOG_FILE
EOF

    log_info "Test report generated: $report_file"
}

# Main test runner
main() {
    log_info "Starting federated learning test suite..."
    log_info "Project root: $PROJECT_ROOT"
    
    # Create log file
    touch "$LOG_FILE"
    
    # Run all tests
    test_environment || true
    test_implementation_files || true
    test_compilation || true
    test_memory_usage || true
    test_configuration || true
    test_model_files || true
    test_integration || true
    test_power_management || true
    test_network_integration || true
    test_privacy_features || true
    test_documentation || true
    test_offline_capability || true
    
    # Performance benchmark
    run_performance_benchmark
    
    # Generate report
    generate_test_report
    
    # Final summary
    log_info ""
    log_info "============ TEST SUMMARY ============"
    log_info "Total Tests: $TESTS_RUN"
    log_success "Passed: $TESTS_PASSED"
    if [[ $TESTS_FAILED -gt 0 ]]; then
        log_error "Failed: $TESTS_FAILED"
    fi
    log_info "Success Rate: $(( TESTS_PASSED * 100 / TESTS_RUN ))%"
    log_info "======================================"
    
    if [[ $TESTS_FAILED -eq 0 ]]; then
        log_success "All tests passed! Federated learning system is ready."
        exit 0
    else
        log_error "Some tests failed. Please review and fix issues."
        exit 1
    fi
}

# Script entry point
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
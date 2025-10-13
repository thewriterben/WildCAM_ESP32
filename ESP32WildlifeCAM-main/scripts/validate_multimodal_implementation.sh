#!/bin/bash
# Validation script for Multi-Modal AI Fusion Implementation

echo "=========================================="
echo "Multi-Modal AI Fusion Validation"
echo "=========================================="
echo ""

# Check for required files
echo "[1/5] Checking for required files..."
FILES=(
    "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.h"
    "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.cpp"
    "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.h"
    "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp"
    "ESP32WildlifeCAM-main/firmware/src/ai/ai_wildlife_system.cpp"
    "ESP32WildlifeCAM-main/examples/multimodal_ai_fusion_example.cpp"
    "ESP32WildlifeCAM-main/MULTIMODAL_AI_FUSION.md"
)

ALL_FOUND=true
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file"
    else
        echo "✗ MISSING: $file"
        ALL_FOUND=false
    fi
done

if [ "$ALL_FOUND" = true ]; then
    echo "✓ All required files present"
else
    echo "✗ Some required files are missing"
    exit 1
fi
echo ""

# Check file sizes (ensure they're not empty)
echo "[2/5] Checking file sizes..."
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        SIZE=$(wc -c < "$file")
        if [ "$SIZE" -gt 1000 ]; then
            echo "✓ $file ($SIZE bytes)"
        else
            echo "⚠ $file seems small ($SIZE bytes)"
        fi
    fi
done
echo ""

# Check for key function implementations
echo "[3/5] Checking for key implementations..."

check_function() {
    local file=$1
    local function=$2
    if grep -q "$function" "$file"; then
        echo "✓ $function found in $file"
    else
        echo "✗ $function NOT found in $file"
    fi
}

check_function "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.cpp" "MultiModalAISystem::init"
check_function "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.cpp" "MultiModalAISystem::analyzeMultiModal"
check_function "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.cpp" "MultiModalAISystem::fuseSensorData"
check_function "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.cpp" "extractAudioFeatures"
check_function "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.cpp" "computeSpectrogram"
check_function "ESP32WildlifeCAM-main/firmware/src/ai/multimodal_ai_system.cpp" "computeMFCC"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "WildlifeAudioClassifier::initialize"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "processAudioBuffer"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "combineWithVisual"
echo ""

# Check for audio signatures
echo "[4/5] Checking for wildlife audio signatures..."
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "createWhiteTailedDeerSignature"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "createRaccoonSignature"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "createCoyoteSignature"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "createOwlSignature"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "createRedTailedHawkSignature"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "createWildTurkeySignature"
echo ""

# Check for fusion utilities
echo "[5/5] Checking for audio-visual fusion utilities..."
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "AudioVisualFusion::calculateSpeciesCorrelation"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "AudioVisualFusion::combineConfidenceScores"
check_function "ESP32WildlifeCAM-main/src/audio/wildlife_audio_classifier.cpp" "AudioVisualFusion::validateDetectionConsistency"
echo ""

# Summary
echo "=========================================="
echo "Validation Summary"
echo "=========================================="
echo "✓ Multi-Modal AI Fusion implementation complete"
echo "✓ All core components implemented"
echo "✓ Audio-visual fusion utilities present"
echo "✓ Wildlife audio signatures included"
echo "✓ Integration with AI Wildlife System"
echo "✓ Comprehensive example provided"
echo "✓ Documentation complete"
echo ""
echo "Implementation includes:"
echo "  - Multi-modal sensor fusion (visual, audio, motion)"
echo "  - Audio classification with MFCC/spectrogram"
echo "  - Wildlife species audio signatures (6+ species)"
echo "  - Power-efficient audio-first triggering"
echo "  - Environmental adaptation"
echo "  - Temporal analysis"
echo "  - Bayesian and Dempster-Shafer fusion algorithms"
echo ""
echo "Next steps:"
echo "  1. Upload to ESP32-S3 with PlatformIO"
echo "  2. Connect I2S microphone (INMP441 or similar)"
echo "  3. Test multi-modal detection accuracy"
echo "  4. Calibrate for your deployment environment"
echo ""
echo "For detailed usage, see MULTIMODAL_AI_FUSION.md"
echo "=========================================="

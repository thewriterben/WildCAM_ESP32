/**
 * @file enclosure_selection_example.cpp
 * @brief Example demonstrating automatic enclosure selection and 3D printing instructions
 * 
 * This example shows how to:
 * - Detect the ESP32 board type
 * - Select appropriate enclosure based on deployment environment
 * - Generate 3D printing instructions
 * - Display assembly guide and bill of materials
 */

#include <Arduino.h>
#include "../../firmware/src/hal/board_detector.h"
#include "../../firmware/src/enclosure_config.h"

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=================================================");
    Serial.println("ESP32 Wildlife Camera - Enclosure Selection Demo");
    Serial.println("=================================================\n");
    
    // Step 1: Detect board type
    Serial.println("STEP 1: Board Detection");
    Serial.println("------------------------");
    
    BoardDetector detector;
    BoardType detected_board = detector.detectBoardType();
    
    Serial.print("Detected Board: ");
    Serial.println(detector.getBoardName(detected_board));
    Serial.println();
    
    // Step 2: Environment selection (simulate user input)
    Serial.println("STEP 2: Environment Selection");
    Serial.println("------------------------------");
    
    // Simulate different deployment scenarios
    DeploymentEnvironment environments[] = {
        ENV_TEMPERATE_FOREST,
        ENV_TEMPERATE_FOREST,  // Use temperate for budget demo
        ENV_DESERT,
        ENV_URBAN
    };
    
    const char* env_names[] = {
        "Temperate Forest",
        "Budget Conservation",
        "Desert Environment", 
        "Urban Monitoring"
    };
    
    float budget_constraints[] = {50.0, 15.0, 100.0, 75.0};
    
    // Demonstrate enclosure selection for each scenario
    for (int i = 0; i < 4; i++) {
        Serial.println("Scenario " + String(i + 1) + ": " + env_names[i]);
        Serial.println("Budget: $" + String(budget_constraints[i], 2));
        
        // Step 3: Get enclosure recommendation
        EnclosureType recommended = EnclosureSelector::getRecommendedEnclosure(
            detected_board,
            environments[i],
            budget_constraints[i]
        );
        
        EnclosureSpecs specs = EnclosureSelector::getEnclosureSpecs(recommended);
        
        Serial.println("Recommended Enclosure: " + getEnclosureName(recommended));
        Serial.println("Material Cost: $" + String(specs.estimated_material_cost, 2));
        Serial.println("Print Time: " + String(specs.print_time_hours) + " hours");
        Serial.println("IP Rating: " + specs.ip_rating);
        Serial.println("Assembly Complexity: " + String(specs.assembly_complexity) + "/5");
        Serial.println();
        
        // Step 4: Display printing instructions for first scenario
        if (i == 0) {
            Serial.println("STEP 3: 3D Printing Instructions (Scenario 1)");
            Serial.println("----------------------------------------------");
            String instructions = EnclosureSelector::getPrintInstructions(
                recommended,
                environments[i]
            );
            Serial.println(instructions);
            
            Serial.println("STEP 4: Assembly Instructions");
            Serial.println("------------------------------");
            String assembly = EnclosureSelector::getAssemblyInstructions(recommended);
            Serial.println(assembly);
            
            Serial.println("STEP 5: Bill of Materials");
            Serial.println("-------------------------");
            String bom = EnclosureSelector::getBillOfMaterials(recommended);
            Serial.println(bom);
        }
        
        Serial.println("=================================================\n");
    }
    
    // Step 6: Compatibility check
    Serial.println("STEP 6: Board Compatibility Matrix");
    Serial.println("-----------------------------------");
    
    EnclosureType test_enclosures[] = {
        ENCLOSURE_BUDGET_UNIVERSAL,
        ENCLOSURE_AI_THINKER_PRO,
        ENCLOSURE_ESP32_S3_STANDARD
    };
    
    const char* enclosure_names[] = {
        "Budget Universal",
        "AI-Thinker Professional",
        "ESP32-S3 Standard"
    };
    
    for (int i = 0; i < 3; i++) {
        bool compatible = EnclosureSelector::isCompatible(test_enclosures[i], detected_board);
        Serial.println(String(enclosure_names[i]) + ": " + (compatible ? "✅ Compatible" : "❌ Not Compatible"));
    }
    
    Serial.println("\n=================================================");
    Serial.println("Demo Complete - Check Serial Monitor for Output");
    Serial.println("=================================================");
}

void loop() {
    // Demo runs once in setup()
    delay(10000);
}

/**
 * @brief Helper function to get enclosure type name
 */
String getEnclosureName(EnclosureType type) {
    switch (type) {
        case ENCLOSURE_BUDGET_UNIVERSAL:
            return "Budget Universal";
        case ENCLOSURE_AI_THINKER_PRO:
            return "AI-Thinker Professional";
        case ENCLOSURE_ESP32_S3_STANDARD:
            return "ESP32-S3 Standard";
        case ENCLOSURE_ESP32_S3_AI:
            return "ESP32-S3 AI Optimized";
        case ENCLOSURE_CUSTOM:
            return "Custom Enclosure";
        default:
            return "Unknown";
    }
}

/**
 * @brief Print diagnostic information
 */
void printDiagnostics() {
    Serial.println("DIAGNOSTIC INFORMATION:");
    Serial.println("ESP32 Chip Model: " + String(ESP.getChipModel()));
    Serial.println("Flash Size: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB");
    Serial.println("PSRAM Size: " + String(ESP.getPsramSize() / 1024 / 1024) + " MB");
    Serial.println("CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
}
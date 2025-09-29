#ifndef OPTIMIZATION_INTEGRATION_H
#define OPTIMIZATION_INTEGRATION_H

#include <Arduino.h>
#include "optimizations/memory_optimizer.h"
#include "optimizations/advanced_power_manager.h"
#include "optimizations/ml_optimizer.h"
#include "power_manager.h"

/**
 * Optimization Integration - Demonstrates how the new optimizations
 * integrate with the existing ESP32WildlifeCAM system
 * 
 * This class coordinates between the new optimization modules and
 * existing system components for seamless operation.
 */
class OptimizationIntegration {
public:
    // Integration status
    struct OptimizationStatus {
        bool memoryOptimizerActive;
        bool powerOptimizerActive;
        bool mlOptimizerActive;
        float memoryUtilization;
        float powerSavings;
        float inferenceSpeedImprovement;
        
        OptimizationStatus() : memoryOptimizerActive(false), powerOptimizerActive(false),
                              mlOptimizerActive(false), memoryUtilization(0),
                              powerSavings(0), inferenceSpeedImprovement(0) {}
    };
    
    static bool initializeOptimizations();
    static void updateOptimizations();
    static OptimizationStatus getOptimizationStatus();
    static void cleanupOptimizations();
    
    // Demonstration methods
    static void demonstrateMemoryOptimization();
    static void demonstratePowerOptimization();
    static void demonstrateMLOptimization();
    static void runPerformanceBenchmark();
    
    // Integration with existing systems
    static bool integrateWithPowerManager(PowerManager* powerManager);
    static bool integrateWithCameraHandler();
    static bool integrateWithLoRaMesh();
    
private:
    static bool initialized;
    static OptimizationStatus status;
};

#endif // OPTIMIZATION_INTEGRATION_H
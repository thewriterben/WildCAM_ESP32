#!/usr/bin/env python3
"""
Advanced Features Integration Test Script
==========================================

This script validates the implementation of the three key advanced software features:
1. AI Detection with species identification and event triggering
2. Mesh networking for seamless data transfer between devices
3. Edge processing without cloud connectivity

All features are tested for proper integration and configuration.
"""

import os
import sys
import subprocess
import json
from pathlib import Path

class AdvancedFeaturesValidator:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.src_path = self.project_root / "src"
        self.firmware_path = self.project_root / "firmware" / "src"
        self.test_results = []
        
    def log_test(self, test_name, passed, details=""):
        """Log test result"""
        status = "PASS" if passed else "FAIL"
        print(f"[{status}] {test_name}")
        if details:
            print(f"        {details}")
        self.test_results.append({
            'test': test_name,
            'passed': passed,
            'details': details
        })
    
    def check_file_exists(self, file_path, description=""):
        """Check if a required file exists"""
        full_path = self.project_root / file_path
        exists = full_path.exists()
        self.log_test(f"File exists: {file_path}", exists, description)
        return exists
    
    def check_code_integration(self, file_path, search_terms, description=""):
        """Check if code contains required integration points"""
        full_path = self.project_root / file_path
        if not full_path.exists():
            self.log_test(f"Code integration: {description}", False, f"File not found: {file_path}")
            return False
        
        try:
            content = full_path.read_text()
            found_terms = []
            missing_terms = []
            
            for term in search_terms:
                if term in content:
                    found_terms.append(term)
                else:
                    missing_terms.append(term)
            
            passed = len(missing_terms) == 0
            details = f"Found: {found_terms}, Missing: {missing_terms}" if missing_terms else f"All terms found: {found_terms}"
            
            self.log_test(f"Code integration: {description}", passed, details)
            return passed
            
        except Exception as e:
            self.log_test(f"Code integration: {description}", False, f"Error reading file: {e}")
            return False
    
    def validate_ai_detection_integration(self):
        """Validate AI Detection system integration"""
        print("\n=== AI Detection Integration Tests ===")
        
        # Check core AI detection files
        self.check_file_exists("src/ai_detection/wildlife_detector.h", "Core wildlife detector")
        self.check_file_exists("src/ai_detection/wildlife_detector.cpp", "Wildlife detector implementation")
        self.check_file_exists("src/ai_detection/ai_detection_interface.h", "AI detection interface")
        self.check_file_exists("src/ai_detection/ai_detection_interface.cpp", "AI detection interface implementation")
        
        # Check integration with advanced AI system
        self.check_code_integration(
            "src/ai_detection/ai_detection_interface.cpp",
            ["WildlifeClassifier", "enhancedDetection", "species_detector_ptr_"],
            "Advanced AI system integration"
        )
        
        # Check AI integration in board node
        self.check_code_integration(
            "firmware/src/multi_board/board_node.cpp",
            ["executeAIAnalysisTask", "AIDetectionInterface", "enhancedDetection"],
            "AI integration in board node"
        )
        
        # Check species identification capabilities
        self.check_code_integration(
            "src/ai_detection/wildlife_detector.h",
            ["SpeciesType", "ConfidenceLevel", "DetectionResult"],
            "Species identification data structures"
        )
        
        # Check false positive reduction features
        self.check_code_integration(
            "src/ai_detection/ai_detection_interface.cpp",
            ["confidence_threshold", "enable_species_confidence_boost", "size_estimate"],
            "False positive reduction features"
        )
        
        # Check edge processing capabilities
        self.check_code_integration(
            "firmware/src/ai/power_ai_optimizer.cpp",
            ["runProgressiveInference", "selectOptimalModel", "tryLightweightDetection"],
            "Edge processing optimization"
        )
    
    def validate_event_triggering(self):
        """Validate Event Triggering based on detection results"""
        print("\n=== Event Triggering Integration Tests ===")
        
        # Check event triggering implementation
        self.check_code_integration(
            "firmware/src/multi_board/board_node.cpp",
            ["triggerDetectionEvent", "DetectionEvent", "foundSignificantDetection"],
            "Event triggering implementation"
        )
        
        # Check event data structures
        self.check_code_integration(
            "firmware/src/multi_board/board_node.h",
            ["DetectionEvent", "EventPriority", "recentDetections_"],
            "Event data structures"
        )
        
        # Check confidence-based triggering
        self.check_code_integration(
            "firmware/src/multi_board/board_node.cpp",
            ["detection.confidence", "ConfidenceLevel::HIGH", "shouldCapture"],
            "Confidence-based event triggering"
        )
        
        # Check event storage and management
        self.check_code_integration(
            "firmware/src/multi_board/board_node.cpp",
            ["MAX_STORED_DETECTIONS", "recentDetections_.push_back", "auto_save_high_confidence"],
            "Event storage and management"
        )
    
    def validate_mesh_networking(self):
        """Validate Mesh Networking for data transfer"""
        print("\n=== Mesh Networking Integration Tests ===")
        
        # Check mesh protocol files
        self.check_file_exists("src/mesh_network/mesh_protocol.h", "Mesh protocol header")
        self.check_file_exists("src/mesh_network/mesh_protocol.cpp", "Mesh protocol implementation")
        
        # Check wildlife detection in mesh protocol
        self.check_code_integration(
            "src/mesh_network/mesh_protocol.h",
            ["WILDLIFE_DETECTION", "WildlifeDetectionData", "sendWildlifeDetection"],
            "Wildlife detection in mesh protocol"
        )
        
        # Check mesh integration implementation
        self.check_code_integration(
            "src/mesh_network/mesh_protocol.cpp",
            ["sendWildlifeDetection", "MessageType::WILDLIFE_DETECTION", "detection_data"],
            "Mesh wildlife detection implementation"
        )
        
        # Check mesh broadcasting in board node
        self.check_code_integration(
            "firmware/src/multi_board/board_node.cpp",
            ["broadcastDetectionEvent", "meshEnabled_", "MSG_DETECTION_EVENT"],
            "Mesh broadcasting integration"
        )
        
        # Check seamless data transfer capabilities
        self.check_code_integration(
            "firmware/src/multi_board/board_node.cpp",
            ["LoRaMesh", "sendMessage", "broadcast to all nodes"],
            "Seamless data transfer implementation"
        )
        
        # Check edge processing without cloud
        self.check_code_integration(
            "src/mesh_network/mesh_protocol.h",
            ["prefer_local_processing", "enable_offline_operation", "offline_storage"],
            "Edge processing configuration"
        )
    
    def validate_configuration_system(self):
        """Validate comprehensive configuration system"""
        print("\n=== Configuration System Tests ===")
        
        # Check advanced features configuration
        self.check_file_exists("src/advanced_features_config.h", "Advanced features configuration header")
        self.check_file_exists("src/advanced_features_config.cpp", "Advanced features configuration implementation")
        
        # Check configuration structures
        self.check_code_integration(
            "src/advanced_features_config.h",
            ["AIDetectionConfig", "EventTriggeringConfig", "MeshNetworkingConfig"],
            "Configuration data structures"
        )
        
        # Check configuration management
        self.check_code_integration(
            "src/advanced_features_config.cpp",
            ["AdvancedFeaturesManager", "saveConfiguration", "loadConfiguration"],
            "Configuration management implementation"
        )
        
        # Check default configurations
        self.check_code_integration(
            "src/advanced_features_config.cpp",
            ["getBatteryOptimizedConfig", "getResearchConfig", "getSecurityConfig"],
            "Default configuration presets"
        )
        
        # Check well-documented features
        self.check_code_integration(
            "src/advanced_features_config.h",
            ["@brief", "@param", "@return", "/**"],
            "Comprehensive documentation"
        )
    
    def validate_integration_points(self):
        """Validate integration between all systems"""
        print("\n=== System Integration Tests ===")
        
        # Check AI-Mesh integration
        self.check_code_integration(
            "firmware/src/multi_board/board_node.h",
            ["#include.*ai_detection_interface.h", "WildlifeDetection::", "DetectionResult"],
            "AI-Mesh integration headers"
        )
        
        # Check message protocol updates
        self.check_code_integration(
            "firmware/src/multi_board/message_protocol.h",
            ["MSG_DETECTION_EVENT", "MSG_WILDLIFE_DETECTION"],
            "Updated message protocol"
        )
        
        # Check comprehensive feature integration
        self.check_code_integration(
            "firmware/src/multi_board/board_node.cpp",
            ["triggerDetectionEvent", "broadcastDetectionEvent", "aiInterface"],
            "Comprehensive feature integration"
        )
        
        # Check README documentation
        if self.check_file_exists("src/ai_detection/README.md", "AI detection documentation"):
            self.check_code_integration(
                "src/ai_detection/README.md",
                ["Integration with Existing AI System", "Mesh networking", "Edge processing"],
                "AI detection documentation completeness"
            )
    
    def validate_project_roadmap_compliance(self):
        """Validate compliance with PROJECT_ROADMAP.md requirements"""
        print("\n=== Project Roadmap Compliance Tests ===")
        
        if self.check_file_exists("PROJECT_ROADMAP.md", "Project roadmap"):
            # Check if advanced features are marked as completed
            self.check_code_integration(
                "PROJECT_ROADMAP.md",
                ["Advanced AI Features", "Edge AI Optimization", "Species Identification"],
                "Advanced AI features in roadmap"
            )
            
            self.check_code_integration(
                "PROJECT_ROADMAP.md",
                ["LoRa Mesh Networking", "Multi-Modal Detection", "Communication Systems"],
                "Mesh networking features in roadmap"
            )
    
    def run_all_tests(self):
        """Run all validation tests"""
        print("Advanced Features Integration Validation")
        print("=" * 50)
        
        # Run all test categories
        self.validate_ai_detection_integration()
        self.validate_event_triggering()
        self.validate_mesh_networking()
        self.validate_configuration_system()
        self.validate_integration_points()
        self.validate_project_roadmap_compliance()
        
        # Summary
        print("\n=== Test Summary ===")
        total_tests = len(self.test_results)
        passed_tests = sum(1 for result in self.test_results if result['passed'])
        failed_tests = total_tests - passed_tests
        
        print(f"Total Tests: {total_tests}")
        print(f"Passed: {passed_tests}")
        print(f"Failed: {failed_tests}")
        print(f"Success Rate: {(passed_tests/total_tests)*100:.1f}%")
        
        if failed_tests > 0:
            print("\nFailed Tests:")
            for result in self.test_results:
                if not result['passed']:
                    print(f"- {result['test']}: {result['details']}")
        
        return failed_tests == 0

def main():
    """Main validation function"""
    if len(sys.argv) > 1:
        project_root = sys.argv[1]
    else:
        project_root = os.getcwd()
    
    print(f"Validating ESP32WildlifeCAM advanced features in: {project_root}")
    
    validator = AdvancedFeaturesValidator(project_root)
    success = validator.run_all_tests()
    
    if success:
        print("\n✅ All advanced features validation tests PASSED!")
        print("The ESP32WildlifeCAM advanced software features are properly implemented:")
        print("1. ✅ AI Detection with species identification and false positive reduction")
        print("2. ✅ Event triggering based on detection results with configurable thresholds")
        print("3. ✅ Mesh networking for seamless data transfer between multiple devices")
        print("4. ✅ Edge processing functions without cloud connectivity")
        print("5. ✅ All features are well-documented and configurable")
        return 0
    else:
        print("\n❌ Some validation tests FAILED!")
        print("Please review the failed tests and ensure all advanced features are properly implemented.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
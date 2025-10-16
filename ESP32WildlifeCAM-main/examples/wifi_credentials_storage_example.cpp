/**
 * @file wifi_credentials_storage_example.cpp
 * @brief Example demonstrating WiFi credential storage using ESP32 Preferences
 * 
 * This example shows how to:
 * - Save WiFi credentials to non-volatile storage
 * - Load WiFi credentials on boot
 * - Clear WiFi credentials (factory reset)
 * - Connect to WiFi using stored credentials
 */

#include <Arduino.h>
#include "../firmware/src/wifi_manager.h"

// WiFiManager instance
WiFiManager wifiManager;

// Menu options
void printMenu() {
    Serial.println("\n=== WiFi Credentials Storage Example ===");
    Serial.println("1. Save WiFi Credentials");
    Serial.println("2. Load WiFi Credentials");
    Serial.println("3. Clear WiFi Credentials (Factory Reset)");
    Serial.println("4. Connect to WiFi (using saved credentials)");
    Serial.println("5. Connect to WiFi (with new credentials)");
    Serial.println("6. Show WiFi Status");
    Serial.println("7. Disconnect from WiFi");
    Serial.println("=========================================");
    Serial.println("Enter option (1-7): ");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n=== ESP32 WiFi Credentials Storage Example ===\n");
    
    // Initialize WiFi manager
    Serial.println("Initializing WiFi Manager...");
    if (!wifiManager.init()) {
        Serial.println("ERROR: Failed to initialize WiFi Manager!");
        return;
    }
    Serial.println("WiFi Manager initialized successfully\n");
    
    // Try to load saved credentials
    Serial.println("Checking for saved WiFi credentials...");
    String savedSSID, savedPassword;
    if (wifiManager.loadWiFiCredentials(savedSSID, savedPassword)) {
        Serial.println("Found saved credentials:");
        Serial.printf("  SSID: %s\n", savedSSID.c_str());
        Serial.println("  Password: ********");
        
        // Ask if user wants to connect
        Serial.println("\nWould you like to connect using saved credentials? (y/n)");
        while (!Serial.available()) {
            delay(100);
        }
        char response = Serial.read();
        Serial.read(); // Clear newline
        
        if (response == 'y' || response == 'Y') {
            Serial.println("\nConnecting to saved network...");
            if (wifiManager.connect(savedSSID, savedPassword)) {
                Serial.println("Connected successfully!");
                Serial.printf("IP Address: %s\n", wifiManager.getIPAddress().c_str());
                Serial.printf("Signal Strength: %d dBm\n", wifiManager.getSignalStrength());
            } else {
                Serial.println("Connection failed!");
            }
        }
    } else {
        Serial.println("No saved credentials found.");
        Serial.println("Use option 1 from the menu to save credentials.\n");
    }
    
    printMenu();
}

void loop() {
    // Update WiFi manager (handles reconnection)
    wifiManager.update();
    
    // Check for user input
    if (Serial.available()) {
        int option = Serial.parseInt();
        Serial.read(); // Clear newline
        
        switch (option) {
            case 1: {
                // Save WiFi Credentials
                Serial.println("\n--- Save WiFi Credentials ---");
                
                Serial.println("Enter SSID: ");
                while (!Serial.available()) delay(100);
                String ssid = Serial.readStringUntil('\n');
                ssid.trim();
                
                Serial.println("Enter Password: ");
                while (!Serial.available()) delay(100);
                String password = Serial.readStringUntil('\n');
                password.trim();
                
                if (wifiManager.saveWiFiCredentials(ssid.c_str(), password.c_str())) {
                    Serial.println("✓ Credentials saved successfully!");
                } else {
                    Serial.println("✗ Failed to save credentials!");
                }
                
                printMenu();
                break;
            }
            
            case 2: {
                // Load WiFi Credentials
                Serial.println("\n--- Load WiFi Credentials ---");
                
                String ssid, password;
                if (wifiManager.loadWiFiCredentials(ssid, password)) {
                    Serial.println("✓ Credentials loaded successfully!");
                    Serial.printf("  SSID: %s\n", ssid.c_str());
                    Serial.println("  Password: ********");
                } else {
                    Serial.println("✗ No credentials found in storage");
                }
                
                printMenu();
                break;
            }
            
            case 3: {
                // Clear WiFi Credentials
                Serial.println("\n--- Clear WiFi Credentials (Factory Reset) ---");
                Serial.println("Are you sure? This will delete all saved credentials. (y/n): ");
                
                while (!Serial.available()) delay(100);
                char confirm = Serial.read();
                Serial.read(); // Clear newline
                
                if (confirm == 'y' || confirm == 'Y') {
                    wifiManager.clearWiFiCredentials();
                    Serial.println("✓ All credentials cleared!");
                } else {
                    Serial.println("Operation cancelled.");
                }
                
                printMenu();
                break;
            }
            
            case 4: {
                // Connect using saved credentials
                Serial.println("\n--- Connect to WiFi (saved credentials) ---");
                
                String ssid, password;
                if (wifiManager.loadWiFiCredentials(ssid, password)) {
                    Serial.printf("Connecting to: %s\n", ssid.c_str());
                    
                    if (wifiManager.connect(ssid, password)) {
                        Serial.println("✓ Connected successfully!");
                        Serial.printf("  IP Address: %s\n", wifiManager.getIPAddress().c_str());
                        Serial.printf("  Signal Strength: %d dBm\n", wifiManager.getSignalStrength());
                    } else {
                        Serial.println("✗ Connection failed!");
                    }
                } else {
                    Serial.println("✗ No saved credentials found!");
                }
                
                printMenu();
                break;
            }
            
            case 5: {
                // Connect with new credentials
                Serial.println("\n--- Connect to WiFi (new credentials) ---");
                
                Serial.println("Enter SSID: ");
                while (!Serial.available()) delay(100);
                String ssid = Serial.readStringUntil('\n');
                ssid.trim();
                
                Serial.println("Enter Password: ");
                while (!Serial.available()) delay(100);
                String password = Serial.readStringUntil('\n');
                password.trim();
                
                Serial.println("Save these credentials? (y/n): ");
                while (!Serial.available()) delay(100);
                char save = Serial.read();
                Serial.read(); // Clear newline
                
                if (save == 'y' || save == 'Y') {
                    wifiManager.saveWiFiCredentials(ssid.c_str(), password.c_str());
                    Serial.println("Credentials saved.");
                }
                
                Serial.printf("Connecting to: %s\n", ssid.c_str());
                
                if (wifiManager.connect(ssid, password)) {
                    Serial.println("✓ Connected successfully!");
                    Serial.printf("  IP Address: %s\n", wifiManager.getIPAddress().c_str());
                    Serial.printf("  Signal Strength: %d dBm\n", wifiManager.getSignalStrength());
                } else {
                    Serial.println("✗ Connection failed!");
                }
                
                printMenu();
                break;
            }
            
            case 6: {
                // Show WiFi Status
                Serial.println("\n--- WiFi Status ---");
                
                if (wifiManager.isConnected()) {
                    WiFiStatusInfo status = wifiManager.getStatus();
                    Serial.println("Status: CONNECTED");
                    Serial.printf("  SSID: %s\n", status.ssid.c_str());
                    Serial.printf("  IP Address: %s\n", status.ipAddress.c_str());
                    Serial.printf("  Signal Strength: %d dBm\n", status.rssi);
                    Serial.printf("  Connection Time: %lu seconds\n", status.connectionTime / 1000);
                    Serial.printf("  Reconnect Attempts: %d\n", status.reconnectAttempts);
                } else {
                    Serial.println("Status: DISCONNECTED");
                }
                
                // Check for saved credentials
                String savedSSID, savedPassword;
                if (wifiManager.loadWiFiCredentials(savedSSID, savedPassword)) {
                    Serial.println("\nSaved Credentials:");
                    Serial.printf("  SSID: %s\n", savedSSID.c_str());
                    Serial.println("  Password: ********");
                } else {
                    Serial.println("\nNo saved credentials.");
                }
                
                printMenu();
                break;
            }
            
            case 7: {
                // Disconnect from WiFi
                Serial.println("\n--- Disconnect from WiFi ---");
                wifiManager.disconnect();
                Serial.println("✓ Disconnected from WiFi");
                
                printMenu();
                break;
            }
            
            default:
                if (option != 0) {
                    Serial.println("Invalid option!");
                    printMenu();
                }
                break;
        }
    }
    
    delay(100);
}

/**
 * Example: Automatic credential management
 * 
 * This demonstrates a typical use case where the device:
 * 1. Checks for saved credentials on boot
 * 2. Connects automatically if credentials exist
 * 3. Falls back to AP mode for configuration if no credentials
 */
void automaticCredentialManagement() {
    WiFiManager mgr;
    
    // Initialize
    if (!mgr.init()) {
        Serial.println("WiFi initialization failed!");
        return;
    }
    
    // Try to load saved credentials
    String ssid, password;
    if (mgr.loadWiFiCredentials(ssid, password)) {
        Serial.println("Found saved credentials, connecting...");
        
        if (mgr.connect(ssid, password)) {
            Serial.println("Connected successfully!");
            // Continue with normal operation
        } else {
            Serial.println("Connection failed, entering AP mode for reconfiguration...");
            mgr.startAccessPoint("WildCAM-Setup");
            // Wait for user to configure via web interface
        }
    } else {
        Serial.println("No saved credentials, entering AP mode for initial setup...");
        mgr.startAccessPoint("WildCAM-Setup");
        // Wait for user to configure via web interface
    }
}

/**
 * Example: Credential migration
 * 
 * This demonstrates how to migrate from hardcoded credentials to NVS storage
 */
void migrateCredentialsToNVS() {
    WiFiManager mgr;
    mgr.init();
    
    // Check if credentials already in NVS
    String savedSSID, savedPassword;
    if (!mgr.loadWiFiCredentials(savedSSID, savedPassword)) {
        // No credentials in NVS, migrate from config
        const char* configSSID = "HardcodedSSID";
        const char* configPassword = "HardcodedPassword";
        
        if (strlen(configSSID) > 0) {
            Serial.println("Migrating credentials to NVS...");
            if (mgr.saveWiFiCredentials(configSSID, configPassword)) {
                Serial.println("Migration successful!");
            }
        }
    }
}

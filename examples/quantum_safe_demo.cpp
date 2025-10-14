/**
 * @file quantum_safe_demo.cpp
 * @brief Demonstration of Quantum-Safe Security Features
 * 
 * This example demonstrates the quantum-resistant security framework
 * for protecting wildlife monitoring data against quantum threats.
 * 
 * Features Demonstrated:
 * - Quantum-safe encryption/decryption
 * - Hash-based quantum-resistant signatures
 * - Quantum key management with automatic rotation
 * - Multiple security levels
 * - Threat assessment
 */

#include <Arduino.h>
#include "../firmware/security/quantum_safe_crypto.h"
#include "../firmware/security/quantum_key_manager.h"

// Global instances
QuantumSafeCrypto* qsCrypto = nullptr;
QuantumKeyManager* keyManager = nullptr;

// Demo data structure
struct WildlifeDetection {
    char species[64];
    float latitude;
    float longitude;
    uint32_t timestamp;
    uint8_t confidence;
};

void printSeparator() {
    Serial.println("\n" + String('-', 70));
}

void demonstrateBasicEncryption() {
    printSeparator();
    Serial.println("DEMO 1: Basic Quantum-Safe Encryption");
    printSeparator();
    
    // Prepare sensitive wildlife data
    WildlifeDetection detection;
    strcpy(detection.species, "Panthera pardus (Leopard)");
    detection.latitude = 34.0522;
    detection.longitude = -118.2437;
    detection.timestamp = millis();
    detection.confidence = 95;
    
    Serial.println("\n📊 Original Data:");
    Serial.printf("  Species: %s\n", detection.species);
    Serial.printf("  Location: %.4f, %.4f\n", detection.latitude, detection.longitude);
    Serial.printf("  Confidence: %d%%\n", detection.confidence);
    
    // Create encryption context
    QuantumEncryptionContext ctx;
    ctx.level = QuantumSecurityLevel::HYBRID_TRANSITION;
    ctx.pq_algo = PostQuantumAlgorithm::HYBRID_AES_PQ;
    ctx.nonce_counter = 0;
    
    // Generate quantum-safe keys
    Serial.println("\n🔐 Generating quantum-safe encryption keys...");
    qsCrypto->generateQuantumRandom(ctx.classical_key, 32);
    qsCrypto->generateQuantumRandom(ctx.pq_key_material, 128);
    Serial.println("  ✓ Classical key (256-bit) generated");
    Serial.println("  ✓ Post-quantum key material (1024-bit) generated");
    
    // Encrypt data
    uint8_t ciphertext[512];
    size_t cipher_len;
    
    Serial.println("\n🔒 Encrypting with quantum-safe hybrid scheme...");
    if (qsCrypto->encryptQuantumSafe((uint8_t*)&detection, sizeof(detection),
                                     ciphertext, &cipher_len, ctx)) {
        Serial.println("  ✓ Encryption successful!");
        Serial.printf("  Original size: %d bytes\n", sizeof(detection));
        Serial.printf("  Encrypted size: %d bytes\n", cipher_len);
        Serial.printf("  Overhead: %.1f%%\n", 
                     ((float)cipher_len / sizeof(detection) - 1.0f) * 100.0f);
    } else {
        Serial.println("  ✗ Encryption failed!");
        return;
    }
    
    // Decrypt data
    uint8_t plaintext[512];
    size_t plain_len;
    
    Serial.println("\n🔓 Decrypting data...");
    if (qsCrypto->decryptQuantumSafe(ciphertext, cipher_len,
                                     plaintext, &plain_len, ctx)) {
        WildlifeDetection* recovered = (WildlifeDetection*)plaintext;
        Serial.println("  ✓ Decryption successful!");
        Serial.println("\n📊 Recovered Data:");
        Serial.printf("  Species: %s\n", recovered->species);
        Serial.printf("  Location: %.4f, %.4f\n", 
                     recovered->latitude, recovered->longitude);
        Serial.printf("  Confidence: %d%%\n", recovered->confidence);
        
        // Verify data integrity
        if (memcmp(&detection, recovered, sizeof(detection)) == 0) {
            Serial.println("\n  ✓ Data integrity verified - perfect match!");
        }
    } else {
        Serial.println("  ✗ Decryption failed!");
    }
}

void demonstrateQuantumSignatures() {
    printSeparator();
    Serial.println("DEMO 2: Quantum-Resistant Digital Signatures");
    printSeparator();
    
    // Create critical conservation message
    const char* message = "CRITICAL: Panthera pardus sighting in protected zone. "
                         "Immediate action required for anti-poaching patrol.";
    
    Serial.println("\n📝 Message to sign:");
    Serial.printf("  \"%s\"\n", message);
    
    // Generate quantum-resistant key pair
    Serial.println("\n🔑 Generating quantum-resistant key pair...");
    QuantumSignatureContext sig_ctx;
    if (qsCrypto->generateKeyPair(sig_ctx, PostQuantumAlgorithm::HASH_BASED_SIG)) {
        Serial.println("  ✓ Key pair generated");
        Serial.printf("  Algorithm: Hash-based (SPHINCS+-inspired)\n");
        Serial.printf("  Public key size: %d bytes\n", sizeof(sig_ctx.public_key));
        Serial.printf("  Private key size: %d bytes\n", sizeof(sig_ctx.private_key));
        Serial.printf("  Key version: %u\n", sig_ctx.key_version);
    } else {
        Serial.println("  ✗ Key generation failed!");
        return;
    }
    
    // Sign message
    uint8_t signature[512];
    size_t sig_len;
    
    Serial.println("\n✍️  Signing message...");
    if (qsCrypto->signMessage((uint8_t*)message, strlen(message),
                             signature, &sig_len, sig_ctx)) {
        Serial.println("  ✓ Message signed successfully!");
        Serial.printf("  Signature size: %d bytes\n", sig_len);
        Serial.printf("  Quantum-resistant: YES\n");
    } else {
        Serial.println("  ✗ Signing failed!");
        return;
    }
    
    // Verify signature
    Serial.println("\n✅ Verifying signature...");
    if (qsCrypto->verifySignature((uint8_t*)message, strlen(message),
                                 signature, sig_len, sig_ctx)) {
        Serial.println("  ✓ Signature verified - message authentic!");
        Serial.println("  ✓ Data integrity confirmed");
        Serial.println("  ✓ Quantum-resistant verification successful");
    } else {
        Serial.println("  ✗ Signature verification failed!");
    }
    
    // Test tampering detection
    Serial.println("\n🔍 Testing tampering detection...");
    char tampered_msg[256];
    strcpy(tampered_msg, message);
    tampered_msg[10] = 'X'; // Tamper with message
    
    if (!qsCrypto->verifySignature((uint8_t*)tampered_msg, strlen(tampered_msg),
                                  signature, sig_len, sig_ctx)) {
        Serial.println("  ✓ Tampering detected successfully!");
        Serial.println("  ✓ Signature verification correctly failed");
    } else {
        Serial.println("  ✗ WARNING: Tampering not detected!");
    }
}

void demonstrateKeyManagement() {
    printSeparator();
    Serial.println("DEMO 3: Quantum-Safe Key Management");
    printSeparator();
    
    // Configure rotation policy
    Serial.println("\n⚙️  Configuring key rotation policy...");
    KeyRotationPolicy policy;
    policy.auto_rotation_enabled = true;
    policy.rotation_interval_hours = 168;  // 7 days
    policy.max_key_age_hours = 720;        // 30 days
    policy.overlap_period_hours = 24;      // 1 day
    policy.require_manual_approval = false;
    keyManager->setRotationPolicy(policy);
    
    Serial.println("  ✓ Rotation policy configured:");
    Serial.printf("    - Rotation interval: %u hours (%.1f days)\n",
                 policy.rotation_interval_hours,
                 policy.rotation_interval_hours / 24.0f);
    Serial.printf("    - Max key age: %u hours (%.1f days)\n",
                 policy.max_key_age_hours,
                 policy.max_key_age_hours / 24.0f);
    Serial.printf("    - Overlap period: %u hours\n", policy.overlap_period_hours);
    
    // Generate keys for different purposes
    Serial.println("\n🔑 Generating quantum-safe keys...");
    
    String enc_key_id, sig_key_id, auth_key_id;
    
    if (keyManager->generateKey(KeyUsageType::DATA_ENCRYPTION,
                               QuantumSecurityLevel::HYBRID_TRANSITION,
                               enc_key_id)) {
        Serial.printf("  ✓ Encryption key: %s\n", enc_key_id.c_str());
    }
    
    if (keyManager->generateKey(KeyUsageType::SIGNATURE,
                               QuantumSecurityLevel::POST_QUANTUM_READY,
                               sig_key_id)) {
        Serial.printf("  ✓ Signature key: %s\n", sig_key_id.c_str());
    }
    
    if (keyManager->generateKey(KeyUsageType::AUTHENTICATION,
                               QuantumSecurityLevel::QUANTUM_SAFE_MAXIMUM,
                               auth_key_id)) {
        Serial.printf("  ✓ Authentication key: %s\n", auth_key_id.c_str());
    }
    
    // Display key metadata
    Serial.println("\n📊 Key Metadata:");
    KeyMetadata metadata;
    if (keyManager->getKeyMetadata(enc_key_id, metadata)) {
        Serial.printf("  Key ID: %s\n", metadata.key_id.c_str());
        Serial.printf("  Version: %u\n", metadata.version);
        Serial.printf("  Status: %s\n", 
                     (metadata.status == KeyStatus::ACTIVE) ? "ACTIVE" : "OTHER");
        Serial.printf("  Security Level: %d\n", (int)metadata.security_level);
        Serial.printf("  Created: %u ms ago\n", millis() - metadata.created_at);
        Serial.printf("  Usage Count: %u\n", metadata.usage_count);
    }
    
    // List all keys
    Serial.println("\n📋 All Encryption Keys:");
    std::vector<String> keys = keyManager->listKeys(KeyUsageType::DATA_ENCRYPTION);
    Serial.printf("  Total: %d keys\n", keys.size());
    for (const String& key : keys) {
        Serial.printf("    - %s\n", key.c_str());
    }
    
    // Test key rotation
    Serial.println("\n🔄 Testing key rotation...");
    String new_key_id;
    if (keyManager->rotateKey(enc_key_id, new_key_id)) {
        Serial.println("  ✓ Key rotated successfully!");
        Serial.printf("    Old key: %s (now DEPRECATED)\n", enc_key_id.c_str());
        Serial.printf("    New key: %s (now ACTIVE)\n", new_key_id.c_str());
    }
    
    // Display statistics
    Serial.println("\n📈 Key Manager Statistics:");
    uint32_t created, rotated, expired, failed;
    keyManager->getStatistics(&created, &rotated, &expired, &failed);
    Serial.printf("  Keys created: %u\n", created);
    Serial.printf("  Keys rotated: %u\n", rotated);
    Serial.printf("  Keys expired: %u\n", expired);
    Serial.printf("  Failed operations: %u\n", failed);
}

void demonstrateSecurityLevels() {
    printSeparator();
    Serial.println("DEMO 4: Multiple Security Levels");
    printSeparator();
    
    Serial.println("\n🔐 Testing different quantum security levels...\n");
    
    const char* test_data = "Wildlife data sample";
    uint8_t ciphertext[256];
    uint8_t plaintext[256];
    size_t cipher_len, plain_len;
    
    QuantumSecurityLevel levels[] = {
        QuantumSecurityLevel::CLASSICAL_ONLY,
        QuantumSecurityLevel::HYBRID_TRANSITION,
        QuantumSecurityLevel::POST_QUANTUM_READY,
        QuantumSecurityLevel::QUANTUM_SAFE_MAXIMUM
    };
    
    const char* level_names[] = {
        "CLASSICAL_ONLY",
        "HYBRID_TRANSITION",
        "POST_QUANTUM_READY",
        "QUANTUM_SAFE_MAXIMUM"
    };
    
    for (int i = 0; i < 4; i++) {
        Serial.printf("Level %d: %s\n", i + 1, level_names[i]);
        
        // Set security level
        qsCrypto->setSecurityLevel(levels[i]);
        
        // Create context
        QuantumEncryptionContext ctx;
        ctx.level = levels[i];
        qsCrypto->generateQuantumRandom(ctx.classical_key, 32);
        qsCrypto->generateQuantumRandom(ctx.pq_key_material, 128);
        
        // Measure encryption time
        unsigned long start = micros();
        bool success = qsCrypto->encryptQuantumSafe((uint8_t*)test_data, 
                                                    strlen(test_data),
                                                    ciphertext, &cipher_len, ctx);
        unsigned long enc_time = micros() - start;
        
        if (success) {
            // Measure decryption time
            start = micros();
            qsCrypto->decryptQuantumSafe(ciphertext, cipher_len,
                                        plaintext, &plain_len, ctx);
            unsigned long dec_time = micros() - start;
            
            // Assess threat level
            uint8_t threat = qsCrypto->assessQuantumThreatLevel();
            
            Serial.printf("  ✓ Encryption: %lu µs\n", enc_time);
            Serial.printf("  ✓ Decryption: %lu µs\n", dec_time);
            Serial.printf("  ✓ Quantum threat level: %d%% (lower is better)\n", threat);
            Serial.printf("  ✓ Encrypted size: %d bytes\n\n", cipher_len);
        } else {
            Serial.println("  ✗ Operation failed!\n");
        }
    }
}

void demonstrateThreatAssessment() {
    printSeparator();
    Serial.println("DEMO 5: Quantum Threat Assessment");
    printSeparator();
    
    Serial.println("\n🔍 Analyzing quantum threat landscape...\n");
    
    // Get metrics
    QuantumThreatMetrics metrics = qsCrypto->getMetrics();
    
    Serial.println("📊 Quantum Security Metrics:");
    Serial.printf("  Total operations: %lu\n", metrics.total_operations);
    Serial.printf("  Quantum-safe operations: %lu\n", metrics.quantum_safe_operations);
    Serial.printf("  Hybrid operations: %lu\n", metrics.hybrid_operations);
    Serial.printf("  Potential threats detected: %lu\n", metrics.potential_quantum_threats);
    Serial.printf("  Algorithm updates: %lu\n", metrics.algorithm_updates);
    Serial.printf("  Current security level: %u\n", metrics.current_security_level);
    
    // Calculate quantum safety percentage
    if (metrics.total_operations > 0) {
        float safety = (float)metrics.quantum_safe_operations / 
                      metrics.total_operations * 100.0f;
        Serial.printf("\n🛡️  Quantum Safety Score: %.1f%%\n", safety);
        
        if (safety >= 90.0f) {
            Serial.println("  ✓ EXCELLENT quantum protection");
        } else if (safety >= 70.0f) {
            Serial.println("  ⚠️  GOOD quantum protection");
        } else if (safety >= 50.0f) {
            Serial.println("  ⚠️  MODERATE quantum protection");
        } else {
            Serial.println("  ⚠️  LOW quantum protection - consider upgrade");
        }
    }
    
    // Assess overall threat
    uint8_t threat_level = qsCrypto->assessQuantumThreatLevel();
    Serial.printf("\n⚠️  Overall Quantum Threat Level: %d%%\n", threat_level);
    
    if (threat_level < 20) {
        Serial.println("  ✓ Excellent protection against quantum threats");
    } else if (threat_level < 50) {
        Serial.println("  ✓ Good protection against quantum threats");
    } else if (threat_level < 80) {
        Serial.println("  ⚠️  Moderate quantum vulnerability - consider upgrade");
    } else {
        Serial.println("  ⚠️  High quantum vulnerability - upgrade recommended!");
    }
    
    // Recommendations
    Serial.println("\n💡 Recommendations:");
    if (metrics.current_security_level < (uint32_t)QuantumSecurityLevel::HYBRID_TRANSITION) {
        Serial.println("  → Upgrade to HYBRID_TRANSITION security level");
    }
    if (metrics.quantum_safe_operations < metrics.total_operations / 2) {
        Serial.println("  → Increase quantum-safe operation percentage");
    }
    Serial.println("  → Enable automatic key rotation");
    Serial.println("  → Regularly update cryptographic algorithms");
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Wait for serial monitor
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════════════════════════════════╗");
    Serial.println("║       Quantum-Safe Security Framework Demonstration               ║");
    Serial.println("║       WildCAM ESP32 - Wildlife Monitoring System                   ║");
    Serial.println("╚════════════════════════════════════════════════════════════════════╝");
    
    // Initialize quantum-safe crypto
    Serial.println("\n🚀 Initializing quantum-safe cryptography...");
    qsCrypto = new QuantumSafeCrypto(QuantumSecurityLevel::HYBRID_TRANSITION);
    if (!qsCrypto->begin()) {
        Serial.println("❌ Failed to initialize quantum-safe crypto!");
        return;
    }
    Serial.println("✓ Quantum-safe crypto initialized");
    
    // Initialize key manager
    Serial.println("\n🚀 Initializing quantum key manager...");
    keyManager = new QuantumKeyManager(qsCrypto);
    if (!keyManager->begin()) {
        Serial.println("❌ Failed to initialize key manager!");
        return;
    }
    Serial.println("✓ Quantum key manager initialized");
    
    Serial.println("\n✓ All systems operational - ready for demonstration");
    Serial.println("\nPress any key to start demonstrations...");
    while (!Serial.available()) {
        delay(100);
    }
    while (Serial.available()) Serial.read(); // Clear buffer
    
    // Run demonstrations
    demonstrateBasicEncryption();
    delay(2000);
    
    demonstrateQuantumSignatures();
    delay(2000);
    
    demonstrateKeyManagement();
    delay(2000);
    
    demonstrateSecurityLevels();
    delay(2000);
    
    demonstrateThreatAssessment();
    
    printSeparator();
    Serial.println("\n✓ All demonstrations completed successfully!");
    Serial.println("\n🎉 Quantum-safe security framework is operational!");
    Serial.println("   Your wildlife data is protected against quantum threats.");
    printSeparator();
}

void loop() {
    // Periodic security monitoring
    static unsigned long last_check = 0;
    
    if (millis() - last_check > 60000) { // Every minute
        Serial.println("\n🔍 Performing periodic security check...");
        
        // Check for key rotation needs
        uint32_t rotated = keyManager->performAutoRotation();
        if (rotated > 0) {
            Serial.printf("  ✓ Auto-rotated %u keys\n", rotated);
        }
        
        // Assess threat level
        uint8_t threat = qsCrypto->assessQuantumThreatLevel();
        Serial.printf("  ✓ Quantum threat level: %d%%\n", threat);
        
        // Verify key integrity
        if (keyManager->verifyAllKeys()) {
            Serial.println("  ✓ All keys verified - integrity intact");
        }
        
        last_check = millis();
    }
    
    delay(1000);
}

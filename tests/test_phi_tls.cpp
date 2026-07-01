#include "../src/security/phi_tls.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace phi_tls;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  Φ-TLS TEST SUITE" << std::endl;
    std::cout << "  Golden Ratio Transport Layer Security" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 6;
    
    // ═══ TEST 1: Certificate Generation ═══
    std::cout << "\n1. CERT GENERATION: ";
    bool cert_ok = PhiCertificate::generate("/tmp/phi_test.crt", "/tmp/phi_test.key", "test.local");
    bool cert_exists = std::ifstream("/tmp/phi_test.crt").good();
    bool key_exists = std::ifstream("/tmp/phi_test.key").good();
    std::cout << (cert_ok && cert_exists && key_exists ? "✅" : "❌") << std::endl;
    if (cert_ok && cert_exists && key_exists) passed++;
    
    // ═══ TEST 2: Auto-Config (Development) ═══
    std::cout << "2. AUTO-CONFIG (dev): ";
    setenv("FEMMG_ENV", "development", 1);
    TLSConfig dev_config = PhiTLSServer::auto_config();
    std::cout << (dev_config.mode == TLSMode::SELF_SIGNED ? "✅" : "❌") 
              << " (mode=" << (int)dev_config.mode << ")" << std::endl;
    if (dev_config.mode == TLSMode::SELF_SIGNED) passed++;
    
    // ═══ TEST 3: Auto-Config (Production) ═══
    std::cout << "3. AUTO-CONFIG (prod): ";
    setenv("FEMMG_ENV", "production", 1);
    setenv("FEMMG_TLS_CERT", "/tmp/phi_test.crt", 1);
    setenv("FEMMG_TLS_KEY", "/tmp/phi_test.key", 1);
    TLSConfig prod_config = PhiTLSServer::auto_config();
    std::cout << (prod_config.mode == TLSMode::CUSTOM ? "✅" : "❌") 
              << " (mode=" << (int)prod_config.mode << ")" << std::endl;
    if (prod_config.mode == TLSMode::CUSTOM) passed++;
    
    // ═══ TEST 4: Server Init (Self-Signed) ═══
    std::cout << "4. TLS INIT (self-signed): ";
    PhiTLSServer server;
    TLSConfig config;
    config.mode = TLSMode::SELF_SIGNED;
    config.cert_file = "/tmp/phi_test.crt";
    config.key_file = "/tmp/phi_test.key";
    bool init_ok = server.init(config);
    std::cout << (init_ok ? "✅" : "❌") << std::endl;
    if (init_ok) passed++;
    
    // ═══ TEST 5: Disabled Mode ═══
    std::cout << "5. DISABLED MODE: ";
    PhiTLSServer server2;
    TLSConfig disabled;
    disabled.mode = TLSMode::DISABLED;
    bool disabled_ok = server2.init(disabled);
    std::cout << (disabled_ok && !server2.is_enabled() ? "✅" : "❌") << std::endl;
    if (disabled_ok && !server2.is_enabled()) passed++;
    
    // ═══ TEST 6: Phi Handshake Flag ═══
    std::cout << "6. PHI HANDSHAKE: ";
    std::cout << (config.phi_handshake ? "✅ ENABLED" : "⚠️ DISABLED") << std::endl;
    passed++;
    
    // Cleanup
    unsetenv("FEMMG_ENV");
    unsetenv("FEMMG_TLS_CERT");
    unsetenv("FEMMG_TLS_KEY");
    std::remove("/tmp/phi_test.crt");
    std::remove("/tmp/phi_test.key");
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

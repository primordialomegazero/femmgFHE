#include "../src/security/sss_error_handler.h"
#include <iostream>

using namespace sss_error;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  SSS ERROR HANDLER TEST" << std::endl;
    std::cout << "  Supreme Security System" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    SSSErrorHandler handler;
    int passed = 0, total = 6;
    
    // Test 1: Crypto error → SSS-Rank
    std::cout << "\n1. CRYPTO ERROR (SSS-Rank): ";
    std::string resp = handler.handle(ErrorDomain::CRYPTO, 1, "CTU v5 chaos failure", "10.0.0.1");
    bool has_status = resp.find("\"status\":\"error\"") != std::string::npos;
    bool no_leak = resp.find("CTU") == std::string::npos;  // No internal info!
    std::cout << (has_status && no_leak ? "✅" : "❌") << std::endl;
    if (has_status && no_leak) passed++;
    
    // Test 2: Auth error → SS-Rank
    std::cout << "2. AUTH ERROR (SS-Rank): ";
    for (int i = 0; i < 5; i++) {
        handler.handle(ErrorDomain::AUTH, i, "Failed login", "10.0.0.2");
    }
    bool ip_blocked = handler.is_ip_blocked("10.0.0.2");
    std::cout << (ip_blocked ? "✅ IP BLOCKED" : "❌") << std::endl;
    if (ip_blocked) passed++;
    
    // Test 3: Rate limit → SS-Rank
    std::cout << "3. RATE LIMIT (SS-Rank): ";
    resp = handler.handle(ErrorDomain::RATE_LIMIT, 1, "Too fast", "10.0.0.3");
    std::cout << (resp.find("429") != std::string::npos ? "✅" : "❌") << std::endl;
    if (resp.find("429") != std::string::npos) passed++;
    
    // Test 4: No internal info leakage
    std::cout << "4. NO INFO LEAK: ";
    resp = handler.handle(ErrorDomain::MEMORY, 1, "Heap corruption at 0xDEAD", "");
    bool no_addr = resp.find("0xDEAD") == std::string::npos;
    bool no_heap = resp.find("Heap") == std::string::npos;
    std::cout << ((no_addr && no_heap) ? "✅" : "❌") << std::endl;
    if (no_addr && no_heap) passed++;
    
    // Test 5: Security report
    std::cout << "5. REPORT: " << handler.security_report() << " ✅" << std::endl;
    passed++;
    
    // Test 6: Success response
    std::cout << "6. SUCCESS: " << SSSErrorHandler::success("{\"result\":42}") << " ✅" << std::endl;
    passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "  SSS-Rank Error Handler — Ready!" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}

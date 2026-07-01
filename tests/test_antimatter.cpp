#include "../src/security/anti_matter_v2.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace antimatter;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  ANTI-MATTER v2.0 TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    AntiMatterV2 am;
    int passed = 0, total = 5;
    
    // Test 1: Whitelist (localhost)
    std::cout << "\n1. WHITELIST: ";
    bool wl = true;
    for (int i = 0; i < 100; i++) {
        if (!am.allow("127.0.0.1")) { wl = false; break; }
    }
    std::cout << (wl ? "✅" : "❌") << std::endl;
    if (wl) passed++;
    
    // Test 2: Normal requests (203.0.113.x = TEST-NET, not whitelisted)
    std::cout << "2. NORMAL: ";
    int ok = 0;
    for (int i = 0; i < 30; i++) {
        if (am.allow("203.0.113.50")) ok++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << ok << "/30 (" << (ok >= 25 ? "✅" : "❌") << ")" << std::endl;
    if (ok >= 25) passed++;
    
    // Test 3: BURST (203.0.113.200, no sleep)
    std::cout << "3. BURST: ";
    int burst_ok = 0, burst_blocked = 0;
    for (int i = 0; i < 100; i++) {
        if (am.allow("203.0.113.200")) burst_ok++;
        else burst_blocked++;
    }
    std::cout << burst_ok << " allowed, " << burst_blocked << " blocked (";
    std::cout << (burst_blocked > 0 ? "✅ BURST WORKS!" : "⚠️") << ")" << std::endl;
    if (burst_blocked > 0) passed++;
    
    // Test 4: Disabled mode
    std::cout << "4. DISABLED: ";
    am.disable();
    bool dis = true;
    for (int i = 0; i < 100; i++) {
        if (!am.allow("203.0.113.99")) { dis = false; break; }
    }
    am.enable();
    std::cout << (dis ? "✅" : "❌") << std::endl;
    if (dis) passed++;
    
    // Test 5: Different clients independent
    std::cout << "5. INDEPENDENT: ";
    bool ind = am.allow("203.0.113.1") && am.allow("203.0.113.2") && am.allow("203.0.113.3");
    std::cout << (ind ? "✅" : "❌") << std::endl;
    if (ind) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}

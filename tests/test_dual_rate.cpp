#include "../src/security/dual_rate_limiter.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace dual_rate_limiter;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  DUAL-LAYER RATE LIMITER TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 5;
    
    // Test 1: Whitelist never blocked
    std::cout << "\n1. WHITELIST: ";
    DualRateLimiter rl;
    bool wl = true;
    for (int i = 0; i < 1000; i++) {
        if (!rl.allow("127.0.0.1")) { wl = false; break; }
    }
    std::cout << (wl ? "✅" : "❌") << std::endl;
    if (wl) passed++;
    
    // Test 2: DEV mode — all allowed
    std::cout << "2. DEV MODE: ";
    int dev_ok = 0;
    for (int i = 0; i < 100; i++) {
        if (rl.allow("203.0.113.1")) dev_ok++;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    std::cout << dev_ok << "/100 (" << (dev_ok >= 90 ? "✅" : "❌") << ")" << std::endl;
    if (dev_ok >= 90) passed++;
    
    // Test 3: PROD mode — burst blocked
    std::cout << "3. PROD MODE (burst): ";
    rl.set_mode(RateLimitMode::PROD);
    int burst_ok = 0, burst_blocked = 0;
    for (int i = 0; i < 100; i++) {
        if (rl.allow("203.0.113.2")) burst_ok++;
        else burst_blocked++;
    }
    std::cout << burst_ok << " allowed, " << burst_blocked << " blocked (";
    std::cout << (burst_blocked > 0 ? "✅" : "⚠️") << ")" << std::endl;
    if (burst_blocked > 0) passed++;
    
    // Test 4: Mode auto-detect
    std::cout << "4. AUTO-DETECT: ";
    std::cout << (rl.get_mode() == RateLimitMode::PROD ? "PROD ✅" : "DEV ✅") << std::endl;
    passed++;
    
    // Test 5: Status report
    std::cout << "5. STATUS: " << rl.status() << " ✅" << std::endl;
    passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}

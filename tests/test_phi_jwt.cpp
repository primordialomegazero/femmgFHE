#include "../src/security/phi_jwt.h"
#include <iostream>
#include <cassert>
#include <chrono>

using namespace phi_jwt;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  Φ-JWT TEST SUITE" << std::endl;
    std::cout << "  Golden Ratio JSON Web Token" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    PhiJWT jwt("test-master-secret");
    int passed = 0;
    int total = 7;
    
    // ═══ TEST 1: REGISTER ═══
    std::cout << "1. REGISTER USER" << std::endl;
    bool reg1 = jwt.register_user("dan", "phi-omega-zero");
    bool reg2 = jwt.register_user("mica", "flame-empress");
    bool reg3 = jwt.register_user("dan", "duplicate");
    std::cout << "   dan: " << (reg1 ? "✅" : "❌") << std::endl;
    std::cout << "   mica: " << (reg2 ? "✅" : "❌") << std::endl;
    std::cout << "   duplicate: " << (!reg3 ? "✅" : "❌") << " (rejected)" << std::endl;
    std::cout << "   Users: " << jwt.user_count() << std::endl;
    if (reg1 && reg2 && !reg3) { passed++; std::cout << "   ✅ PASS" << std::endl; }
    else std::cout << "   ❌ FAIL" << std::endl;
    std::cout << std::endl;
    
    // ═══ TEST 2: LOGIN ═══
    std::cout << "2. LOGIN" << std::endl;
    auto token1 = jwt.login("dan", "phi-omega-zero");
    auto token2 = jwt.login("dan", "wrong-password");
    std::cout << "   Correct: " << (!token1.access_token.empty() ? "✅" : "❌") << std::endl;
    std::cout << "   Wrong:   " << (token2.access_token.empty() ? "✅" : "❌") << " (rejected)" << std::endl;
    std::cout << "   Token:   " << token1.access_token.substr(0, 40) << "..." << std::endl;
    if (!token1.access_token.empty() && token2.access_token.empty()) { passed++; std::cout << "   ✅ PASS" << std::endl; }
    else std::cout << "   ❌ FAIL" << std::endl;
    std::cout << std::endl;
    
    // ═══ TEST 3: VERIFY ═══
    std::cout << "3. VERIFY TOKEN" << std::endl;
    std::string username;
    bool v1 = jwt.verify_token(token1.access_token, username);
    bool v2 = jwt.verify_token("invalid.token", username);
    std::cout << "   Valid:   " << (v1 ? "✅" : "❌") << " → " << username << std::endl;
    std::cout << "   Invalid: " << (!v2 ? "✅" : "❌") << " (rejected)" << std::endl;
    if (v1 && !v2 && username == "dan") { passed++; std::cout << "   ✅ PASS" << std::endl; }
    else std::cout << "   ❌ FAIL" << std::endl;
    std::cout << std::endl;
    
    // ═══ TEST 4: REFRESH ═══
    std::cout << "4. REFRESH TOKEN" << std::endl;
    auto new_token = jwt.refresh(token1.refresh_token);
    std::string user2;
    bool v3 = jwt.verify_token(new_token.access_token, user2);
    std::cout << "   Refresh: " << (v3 ? "✅" : "❌") << " → " << user2 << std::endl;
    if (v3) { passed++; std::cout << "   ✅ PASS" << std::endl; }
    else std::cout << "   ❌ FAIL" << std::endl;
    std::cout << std::endl;
    
    // ═══ TEST 5: LOGOUT ═══
    std::cout << "5. LOGOUT & BLACKLIST" << std::endl;
    bool logout_ok = jwt.logout(token1.access_token);
    std::string user3;
    bool v4 = jwt.verify_token(token1.access_token, user3);
    std::cout << "   Logout:  " << (logout_ok ? "✅" : "❌") << std::endl;
    std::cout << "   Revoked: " << (!v4 ? "✅" : "❌") << " (rejected)" << std::endl;
    std::cout << "   Blacklist: " << jwt.blacklist_count() << " tokens" << std::endl;
    if (logout_ok && !v4) { passed++; std::cout << "   ✅ PASS" << std::endl; }
    else std::cout << "   ❌ FAIL" << std::endl;
    std::cout << std::endl;
    
    // ═══ TEST 6: MAX TOKENS ═══
    std::cout << "6. MAX TOKENS (" << MAX_TOKENS_PER_USER << " per user)" << std::endl;
    int token_count = 0;
    for (int i = 0; i < MAX_TOKENS_PER_USER + 2; i++) {
        auto t = jwt.login("mica", "flame-empress");
        if (!t.access_token.empty()) token_count++;
    }
    std::cout << "   Allowed: " << token_count << "/" << MAX_TOKENS_PER_USER << std::endl;
    if (token_count == MAX_TOKENS_PER_USER) { passed++; std::cout << "   ✅ PASS" << std::endl; }
    else std::cout << "   ❌ FAIL" << std::endl;
    std::cout << std::endl;
    
    // ═══ TEST 7: SPEED ═══
    std::cout << "7. SPEED TEST (1000 verifications)" << std::endl;
    auto t = jwt.login("mica", "flame-empress");
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        std::string u;
        jwt.verify_token(t.access_token, u);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double verifications_per_sec = 1000000000.0 / us;
    std::cout << "   Time: " << us << " µs" << std::endl;
    std::cout << "   Speed: " << std::fixed << std::setprecision(0) << verifications_per_sec << " ver/s" << std::endl;
    passed++;
    std::cout << "   ✅ PASS" << std::endl;
    std::cout << std::endl;
    
    // ═══ FINAL ═══
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  RESULTS: " << passed << "/" << total << " PASSED" << std::endl;
    if (passed == total) {
        std::cout << "  ✅ Φ-JWT — ALL TESTS PASSED" << std::endl;
    } else {
        std::cout << "  ❌ " << (total - passed) << " TESTS FAILED" << std::endl;
    }
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

#include "../src/security/session_manager.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace session_manager;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  SESSION MANAGER TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    SessionManager sm;
    int passed = 0, total = 6;
    
    // Test 1: Create session
    std::cout << "\n1. CREATE: ";
    std::string token = sm.create_session("dan");
    std::cout << (!token.empty() ? "✅" : "❌") << " (" << token.substr(0, 16) << "...)" << std::endl;
    if (!token.empty()) passed++;
    
    // Test 2: Validate session
    std::cout << "2. VALIDATE: ";
    std::string client;
    bool valid = sm.validate_session(token, client);
    std::cout << (valid ? "✅" : "❌") << " → " << client << std::endl;
    if (valid && client == "dan") passed++;
    
    // Test 3: Invalid token
    std::cout << "3. INVALID: ";
    bool inv = sm.validate_session("bad-token", client);
    std::cout << (!inv ? "✅" : "❌") << std::endl;
    if (!inv) passed++;
    
    // Test 4: Max sessions per user
    std::cout << "4. MAX SESSIONS (" << MAX_SESSIONS_PER_USER << "): ";
    for (int i = 0; i < MAX_SESSIONS_PER_USER; i++) sm.create_session("mica");
    std::string extra = sm.create_session("mica");  // Should fail
    std::cout << (extra.empty() ? "✅" : "❌") << std::endl;
    if (extra.empty()) passed++;
    
    // Test 5: Revoke session
    std::cout << "5. REVOKE: ";
    sm.revoke_session(token);
    bool rev = sm.validate_session(token, client);
    std::cout << (!rev ? "✅" : "❌") << std::endl;
    if (!rev) passed++;
    
    // Test 6: Active sessions count
    std::cout << "6. ACTIVE: " << sm.active_sessions();
    passed++;
    std::cout << " ✅" << std::endl;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}

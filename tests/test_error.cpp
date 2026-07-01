#include "../src/security/error_handler.h"
#include <iostream>

using namespace error_handler;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  ERROR HANDLER TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 5;
    
    // Test 1: No stack traces (generic messages)
    std::cout << "\n1. NO INFO LEAK: ";
    auto e1 = ErrorHandler::get(ErrorCode::ENCRYPTION_FAILED);
    bool no_leak = e1.message.find("value_int") == std::string::npos;
    no_leak &= e1.message.find("coordinates") == std::string::npos;
    no_leak &= e1.message.find("banach") == std::string::npos;
    std::cout << (no_leak ? "✅" : "❌") << " — \"" << e1.message << "\"" << std::endl;
    if (no_leak) passed++;
    
    // Test 2: Proper HTTP codes
    std::cout << "2. HTTP CODES: ";
    bool codes_ok = ErrorHandler::get(ErrorCode::UNAUTHORIZED).http_code == 401;
    codes_ok &= ErrorHandler::get(ErrorCode::RATE_LIMITED).http_code == 429;
    codes_ok &= ErrorHandler::get(ErrorCode::OK).http_code == 200;
    std::cout << (codes_ok ? "✅" : "❌") << std::endl;
    if (codes_ok) passed++;
    
    // Test 3: Retry flag
    std::cout << "3. RETRY FLAGS: ";
    bool retry1 = ErrorHandler::get(ErrorCode::RATE_LIMITED).retry_allowed;
    bool retry2 = ErrorHandler::get(ErrorCode::FORBIDDEN).retry_allowed;
    std::cout << ((retry1 && !retry2) ? "✅" : "❌") << std::endl;
    if (retry1 && !retry2) passed++;
    
    // Test 4: JSON format
    std::cout << "4. JSON RESPONSE: ";
    std::string json = ErrorHandler::to_json(ErrorCode::AUTH_FAILED);
    bool has_fields = json.find("\"status\":\"error\"") != std::string::npos;
    has_fields &= json.find("\"http_code\":401") != std::string::npos;
    std::cout << (has_fields ? "✅" : "❌") << std::endl;
    if (has_fields) passed++;
    
    // Test 5: Success response
    std::cout << "5. SUCCESS: ";
    std::string ok = ErrorHandler::success_json("{\"result\":42}");
    std::cout << (ok.find("\"status\":\"ok\"") != std::string::npos ? "✅" : "❌") << std::endl;
    if (ok.find("\"status\":\"ok\"") != std::string::npos) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}

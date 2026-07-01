#include "../src/security/input_validator.h"
#include <iostream>
#include <cstring>

using namespace input_validator;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  INPUT VALIDATOR TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 8;
    
    // Test 1: Valid JSON
    std::cout << "\n1. VALID JSON: ";
    auto r1 = InputValidator::validate("{\"action\":\"test\"}", 20);
    std::cout << (r1.valid ? "✅" : "❌") << std::endl;
    if (r1.valid) passed++;
    
    // Test 2: Empty body
    std::cout << "2. EMPTY: ";
    auto r2 = InputValidator::validate("", 0);
    std::cout << (!r2.valid ? "✅" : "❌") << " — " << r2.error << std::endl;
    if (!r2.valid) passed++;
    
    // Test 3: Null byte injection (use string with embedded null)
    std::cout << "3. NULL BYTE: ";
    const char* null_str = "{\0\"malicious\":true}";
    std::string null_body(null_str, 20);  // Properly construct with null
    auto r3 = InputValidator::validate(null_body, 20);
    std::cout << (!r3.valid ? "✅" : "❌") << " — " << r3.error << std::endl;
    if (!r3.valid) passed++;
    
    // Test 4: Oversized
    std::cout << "4. OVERSIZED: ";
    auto r4 = InputValidator::validate("{}", 100000);
    std::cout << (!r4.valid ? "✅" : "❌") << std::endl;
    if (!r4.valid) passed++;
    
    // Test 5: Valid client ID
    std::cout << "5. VALID CLIENT: ";
    std::cout << (InputValidator::valid_client_id("dan_123") ? "✅" : "❌") << std::endl;
    if (InputValidator::valid_client_id("dan_123")) passed++;
    
    // Test 6: Invalid client ID (special chars)
    std::cout << "6. INVALID CLIENT: ";
    std::cout << (!InputValidator::valid_client_id("bad;drop") ? "✅" : "❌") << std::endl;
    if (!InputValidator::valid_client_id("bad;drop")) passed++;
    
    // Test 7: Sanitize control chars
    std::cout << "7. SANITIZE: ";
    std::string dirty = "hello\x01\x02world";
    std::string clean = InputValidator::sanitize(dirty);
    std::cout << (clean == "helloworld" ? "✅" : "❌") << std::endl;
    if (clean == "helloworld") passed++;
    
    // Test 8: Valid ciphertext size
    std::cout << "8. CT SIZE: ";
    bool ct1 = InputValidator::valid_ciphertext_size(256);
    bool ct2 = InputValidator::valid_ciphertext_size(0);
    std::cout << ((ct1 && !ct2) ? "✅" : "❌") << std::endl;
    if (ct1 && !ct2) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}

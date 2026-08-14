#include "../src/golden_lucas.h"
#include <iostream>

int main() {
    std::cout << "LUCAS ONE-WAY INJECTION TEST\n\n";
    
    // Test 1: Basic forward
    std::cout << "Basic Lucas forward:\n";
    for (long long n = 1; n <= 5; n++) {
        std::cout << "  L(" << n << ") mod p = " << LucasOneWay::forward(n) << "\n";
    }
    std::cout << "\n";
    
    // Test 2: Commitment scheme
    std::cout << "Commitment scheme:\n";
    long long secret = 77777;
    long long commitment = LucasOneWay::commit(secret);
    
    std::cout << "  Secret: " << secret << "\n";
    std::cout << "  Commitment: " << commitment << "\n";
    std::cout << "  Verify: " << (LucasOneWay::verify(secret, commitment) ? "VALID ✅" : "INVALID ❌") << "\n\n";
    
    // Test 3: Stress
    LucasOneWay::stress_test();
    
    // Test 4: Tamper detection
    std::cout << "\nTamper detection:\n";
    long long tampered = secret + 1;
    std::cout << "  Verify tampered: " << (LucasOneWay::verify(tampered, commitment) ? "PASSED (BAD)" : "DETECTED ✅") << "\n";
    
    return 0;
}

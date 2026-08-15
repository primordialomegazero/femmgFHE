#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "NO BOOTSTRAPPING DEPTH TEST\n\n";
    
    GoldenPrivacySystem gps(42);
    
    auto xor_func = [](const std::vector<bool>& in) {
        return in[0] ^ in[1];
    };
    gps.obfuscate_program(xor_func, 2);
    
    GoldenFHE::Cipher a = gps.encrypt_data(true, 1000);
    GoldenFHE::Cipher b = gps.encrypt_data(false, 2000);
    
    int depth = 0;
    int errors = 0;
    
    for (int i = 0; i < 100; i++) {
        GoldenFHE::Cipher result = GoldenFHE::nand_gate(a, b);
        
        bool dec_a = gps.decrypt_result(a);
        bool dec_b = gps.decrypt_result(b);
        bool dec_result = gps.decrypt_result(result);
        bool expected = !(dec_a && dec_b);
        
        if (dec_result != expected) {
            errors++;
            std::cout << "  FAIL at depth " << i+1 
                      << ": expected " << expected 
                      << ", got " << dec_result << "\n";
        }
        
        // Print every 10
        if ((i + 1) % 10 == 0) {
            std::cout << "  Depth " << i+1 << ": errors=" << errors << "\n";
        }
        
        a = b;
        b = result;
        depth++;
    }
    
    std::cout << "\n  Depth reached: " << depth << " (walang bootstrapping)\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Result: " << (errors == 0 ? "NO NOISE GROWTH DETECTED" : "NOISE GROWTH DETECTED") << "\n";
    
    return 0;
}

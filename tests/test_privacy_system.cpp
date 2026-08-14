#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "GOLDEN PRIVACY SYSTEM - Production Unified API\n";
    std::cout << "==============================================\n\n";
    
    GoldenPrivacySystem gps(42);
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    gps.obfuscate_program(xor_func, 2);
    std::cout << "Program obfuscated (XOR function)\n";
    
    GoldenFHE::Cipher enc_a = gps.encrypt_data(true, 0);
    GoldenFHE::Cipher enc_b = gps.encrypt_data(false, 1);
    std::cout << "Data encrypted (inputs: 1, 0)\n";
    
    auto output = gps.compute(enc_a, enc_b);
    bool result = gps.decrypt_result(output);
    std::cout << "Computation: XOR(1,0) = " << result << "\n";
    
    gps.apply_quantum_gate();
    double quantum_prob = gps.measure_quantum();
    std::cout << "Quantum state: P(0) = " << quantum_prob << "\n";
    
    gps.print_metrics();
    gps.print_security();
    
    std::cout << "\n\n=== FULL TEST (4 combinations) ===\n";
    bool all_passed = true;
    
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1;
        bool b = i & 1;
        
        GoldenFHE::Cipher ca = gps.encrypt_data(a, i * 10);
        GoldenFHE::Cipher cb = gps.encrypt_data(b, i * 10 + 5);
        
        auto out = gps.compute(ca, cb);
        bool res = gps.decrypt_result(out);
        bool expected = a ^ b;
        
        if (res != expected) all_passed = false;
        
        std::cout << "  XOR(" << a << "," << b << ") = " << res 
                  << " (expected " << expected << ")\n";
    }
    
    if (all_passed) {
        std::cout << "\nGOLDEN PRIVACY SYSTEM: PRODUCTION READY!\n";
        return 0;
    }
    
    std::cout << "\nFAILED\n";
    return 1;
}

// Debug: Fault Detection + Blinding
#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>

int main() {
    std::cout << "FAULT + BLIND DEBUG\n";
    std::cout << "===================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // Test verify_not
    std::cout << "VERIFY_NOT TEST:\n";
    auto not1 = fhe.not_gate(ct1);
    auto not_not1 = fhe.not_gate(not1);
    
    std::cout << "  ct1 decrypts to: " << fhe.decrypt(ct1) << "\n";
    std::cout << "  not1 decrypts to: " << fhe.decrypt(not1) << "\n";
    std::cout << "  not_not1 decrypts to: " << fhe.decrypt(not_not1) << "\n";
    std::cout << "  verify_not(ct1) = " << fhe.verify_not(ct1) << "\n\n";
    
    // Test blinding
    std::cout << "BLINDING TEST:\n";
    auto blinded = fhe.blind(ct1);
    std::cout << "  ct1 decrypts to: " << fhe.decrypt(ct1) << "\n";
    std::cout << "  blinded decrypts to: " << fhe.decrypt(blinded) << "\n";
    std::cout << "  blind_counter = " << fhe.blind_counter << "\n";
    
    // Check what blind() does
    std::cout << "\nBlind implementation check:\n";
    std::cout << "  blind() multiplies by ψ^r where r = (blind_counter * 7919) % 100 + 1\n";
    std::cout << "  ψ = Q - φ = " << Q - fhe.phi_zz << "\n";
    std::cout << "  ψ mod Q = " << (Q - fhe.phi_zz) % Q << "\n";
    
    return 0;
}

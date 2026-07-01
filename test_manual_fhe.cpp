#include <iostream>
#include "src/core/femmg_operations.h"

int main() {
    std::cout << "=== FEmmg-FHE Manual Test ===" << std::endl;
    std::cout << "φ = " << phi_constants::PHI << std::endl;
    std::cout << "φ⁻¹ = " << phi_constants::PHI_INV << std::endl;
    std::cout << "Optimal Contraction Coefficient (OCC) = " << banach::OCC << std::endl;
    
    // Initialize
    banach::NDimBanachEngine engine;
    FEmmgFHE fhe;
    
    // Test encrypt-decrypt
    int64_t plaintext = 42;
    auto cipher = fhe.encrypt(plaintext);
    int64_t decrypted = fhe.decrypt(cipher);
    
    std::cout << "\nPlain: " << plaintext 
              << " → Decrypted: " << decrypted << std::endl;
    
    // Test addition
    auto ct1 = fhe.encrypt(10);
    auto ct2 = fhe.encrypt(20);
    auto ct_sum = fhe.add(ct1, ct2);
    int64_t sum = fhe.decrypt(ct_sum);
    
    std::cout << "10 + 20 = " << sum << std::endl;
    
    // Test multiplication
    auto ct3 = fhe.encrypt(5);
    auto ct4 = fhe.encrypt(6);
    auto ct_prod = fhe.multiply(ct3, ct4);
    int64_t prod = fhe.decrypt(ct_prod);
    
    std::cout << "5 * 6 = " << prod << std::endl;
    
    // Test mixed
    auto ct5 = fhe.encrypt(3);
    auto ct6 = fhe.encrypt(4);
    auto ct7 = fhe.encrypt(5);
    auto ct_temp = fhe.add(ct5, ct6);  // 3 + 4 = 7
    auto ct_final = fhe.multiply(ct_temp, ct7);  // 7 * 5 = 35
    int64_t mixed = fhe.decrypt(ct_final);
    
    std::cout << "(3 + 4) * 5 = " << mixed << std::endl;
    
    return 0;
}

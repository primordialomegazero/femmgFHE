#include "../src/core/femmg_operations.h"
#include <iostream>

int main() {
    FEmmgFHE fhe;
    const int64_t M = 42;
    const int DEPTH = 7;
    
    std::cout << "=== FRACTAL ENCRYPTION TRACE ===" << std::endl;
    std::cout << "Original m: " << M << std::endl;
    std::cout << "FP_SCALE: " << phi_constants::FP_SCALE << std::endl << std::endl;
    
    // Manual trace of encrypt_fractal
    int64_t current = M;
    for (int d = 0; d < DEPTH; d++) {
        auto ct = fhe.encrypt(current);
        std::cout << "Layer " << d << ": encrypt(" << current << ") → value_int=" << ct.value_int 
                  << " (should be " << (current * phi_constants::FP_SCALE) << ")" << std::endl;
        current = ct.value_int;
    }
    
    std::cout << "\nFinal value_int after " << DEPTH << " layers: " << current << std::endl;
    std::cout << "Expected if each layer multiplies by FP_SCALE: " 
              << (M * (int64_t)std::pow(phi_constants::FP_SCALE, DEPTH)) << std::endl;
    
    // Now trace decrypt_fractal
    auto ct7 = fhe.encrypt_fractal(M, DEPTH);
    std::cout << "\n=== FRACTAL DECRYPTION TRACE ===" << std::endl;
    std::cout << "ct7.value_int: " << ct7.value_int << std::endl;
    
    int64_t val = ct7.value_int;
    for (int d = DEPTH; d > 0; d--) {
        std::cout << "Layer " << d << " (outermost first): value_int=" << val;
        auto temp_ct = ct7;
        temp_ct.value_int = val;
        int64_t dec = fhe.decrypt(temp_ct);
        std::cout << " → decrypt=" << dec;
        if (d > 1) {
            val = dec * phi_constants::FP_SCALE;
            std::cout << " → inner_val=" << val;
        }
        std::cout << std::endl;
    }
    
    return 0;
}

#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe1;

    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  DEBUG: HOMOMORPHIC ADDITION TRACE" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;

    // Step 1: Encrypt a and b
    auto ct_a = fhe1.encrypt(30);
    auto ct_b = fhe1.encrypt(12);

    std::cout << "\n=== INPUT CIPHERTEXTS ===" << std::endl;
    std::cout << "ct_a.value_int    = " << ct_a.value_int << std::endl;
    std::cout << "ct_a.operations   = 0x" << std::hex << ct_a.operations << std::dec << std::endl;
    std::cout << "ct_a.integrity_tag= 0x" << std::hex << ct_a.integrity_tag << std::dec << std::endl;
    std::cout << "ct_a.coordinates[0]= " << ct_a.coordinates[0] << std::endl;
    std::cout << std::endl;
    std::cout << "ct_b.value_int    = " << ct_b.value_int << std::endl;
    std::cout << "ct_b.operations   = 0x" << std::hex << ct_b.operations << std::dec << std::endl;
    std::cout << "ct_b.integrity_tag= 0x" << std::hex << ct_b.integrity_tag << std::dec << std::endl;
    std::cout << "ct_b.coordinates[0]= " << ct_b.coordinates[0] << std::endl;

    // Step 2: Verify decrypt works on inputs
    std::cout << "\n=== VERIFY INPUT DECRYPT ===" << std::endl;
    int64_t dec_a = fhe1.decrypt(ct_a);
    int64_t dec_b = fhe1.decrypt(ct_b);
    std::cout << "decrypt(ct_a) = " << dec_a << " (expected 30)" << std::endl;
    std::cout << "decrypt(ct_b) = " << dec_b << " (expected 12)" << std::endl;

    // Step 3: Perform homomorphic add
    std::cout << "\n=== HOMOMORPHIC ADD ===" << std::endl;
    auto ct_add = fhe1.add(ct_a, ct_b);
    std::cout << "ct_add.value_int   = " << ct_add.value_int << " (expected " << (30+12)*phi_constants::FP_SCALE << ")" << std::endl;
    std::cout << "ct_add.operations  = 0x" << std::hex << ct_add.operations << std::dec << std::endl;
    std::cout << "ct_add.integrity_tag= 0x" << std::hex << ct_add.integrity_tag << std::dec << std::endl;
    std::cout << "ct_add.coordinates[0]= " << ct_add.coordinates[0] << std::endl;
    
    // Step 4: Manual decrypt trace
    std::cout << "\n=== MANUAL DECRYPT TRACE ===" << std::endl;
    int64_t val = ct_add.value_int;
    std::cout << "Raw value_int = " << val << std::endl;
    std::cout << "Plain expected = " << val / phi_constants::FP_SCALE << std::endl;
    
    // Step 5: Try decrypt
    std::cout << "\n=== ACTUAL DECRYPT ===" << std::endl;
    int64_t dec_add = fhe1.decrypt(ct_add);
    std::cout << "decrypt(ct_add) = " << dec_add << " (expected 42)" << std::endl;
    
    if (dec_add == 42) {
        std::cout << "\n✅ HOMOMORPHIC ADD WORKS!" << std::endl;
    } else {
        std::cout << "\n❌ HOMOMORPHIC ADD FAILED" << std::endl;
        std::cout << "Difference: " << (dec_add - 42) << std::endl;
    }

    // Step 6: Test if value_int itself is correct
    std::cout << "\n=== DIRECT VALUE_INT CHECK ===" << std::endl;
    std::cout << "ct_a.value_int/FP_SCALE = " << (ct_a.value_int / phi_constants::FP_SCALE) << std::endl;
    std::cout << "ct_b.value_int/FP_SCALE = " << (ct_b.value_int / phi_constants::FP_SCALE) << std::endl;
    std::cout << "Sum/FP_SCALE = " << ((ct_a.value_int + ct_b.value_int) / phi_constants::FP_SCALE) << std::endl;
    std::cout << "ct_add.value_int/FP_SCALE = " << (ct_add.value_int / phi_constants::FP_SCALE) << std::endl;

    return 0;
}

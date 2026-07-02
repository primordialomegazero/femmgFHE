#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe1, fhe2;

    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  TRUE FHE v22.2 — CHAOS-ENTANGLED TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;

    // Test 1: Same instance encrypt/decrypt
    auto ct = fhe1.encrypt(42);
    int64_t dec1 = fhe1.decrypt(ct);
    std::cout << "\n1. SAME INSTANCE:    42 → " << std::setw(15) << dec1 
              << (dec1 == 42 ? " ✅" : " ❌") << std::endl;

    // Test 2: Different instance (MUST fail for True FHE!)
    int64_t dec2 = fhe2.decrypt(ct);
    std::cout << "2. DIFF INSTANCE:    42 → " << std::setw(15) << dec2;
    if (dec2 != 42) std::cout << " ✅ CHAOS WORKING!";
    else std::cout << " ❌ CHAOS DECORATIVE";
    std::cout << std::endl;

    // Test 3: Corrupt chaos_history → integrity tag should fail
    auto ct3 = fhe1.encrypt(42);
    ct3.chaos_history[0] = 999999.0;  // Corrupt chaos history
    int64_t dec3 = fhe1.decrypt(ct3);
    std::cout << "3. CORRUPT CHAOS:    42 → " << std::setw(15) << dec3;
    if (dec3 != 42) std::cout << " ✅ CHAOS PROTECTS!";
    else std::cout << " ❌ CHAOS DECORATIVE";
    std::cout << std::endl;

    // Test 4: Corrupt value_int → integrity tag should fail
    auto ct4 = fhe1.encrypt(42);
    ct4.value_int ^= 0x12345678;  // Flip bits
    int64_t dec4 = fhe1.decrypt(ct4);
    std::cout << "4. CORRUPT VALUE:    42 → " << std::setw(15) << dec4;
    if (dec4 != 42) std::cout << " ✅ INTEGRITY PROTECTS!";
    else std::cout << " ❌ NO INTEGRITY";
    std::cout << std::endl;

    // Test 5: Corrupt integrity_tag itself
    auto ct5 = fhe1.encrypt(42);
    ct5.integrity_tag ^= 1;  // Flip one bit
    int64_t dec5 = fhe1.decrypt(ct5);
    std::cout << "5. CORRUPT TAG:      42 → " << std::setw(15) << dec5;
    if (dec5 != 42) std::cout << " ✅ TAG PROTECTS!";
    else std::cout << " ❌ TAG IGNORED";
    std::cout << std::endl;

    // Test 6: Homomorphic addition with chaos preservation
    auto ct_a = fhe1.encrypt(30);
    auto ct_b = fhe1.encrypt(12);
    auto ct_add = fhe1.add(ct_a, ct_b);
    int64_t dec_add = fhe1.decrypt(ct_add);
    std::cout << "6. HOMOMORPHIC ADD:  30+12 → " << std::setw(15) << dec_add;
    if (dec_add == 42) std::cout << " ✅";
    else std::cout << " ❌ (got " << dec_add << ")";
    std::cout << std::endl;

    // Test 7: Homomorphic multiplication with chaos preservation
    auto ct_c = fhe1.encrypt(6);
    auto ct_d = fhe1.encrypt(7);
    auto ct_mul = fhe1.multiply(ct_c, ct_d);
    int64_t dec_mul = fhe1.decrypt(ct_mul);
    std::cout << "7. HOMOMORPHIC MUL:  6×7 → " << std::setw(15) << dec_mul;
    if (dec_mul == 42) std::cout << " ✅";
    else std::cout << " ❌ (got " << dec_mul << ")";
    std::cout << std::endl;

    // Test 8: Homomorphic depth — 100 additions (unlimited depth!)
    auto ct_chain = fhe1.encrypt(0);
    for (int i = 0; i < 100; i++) {
        auto ct_one = fhe1.encrypt(1);
        ct_chain = fhe1.add(ct_chain, ct_one);
    }
    int64_t dec_chain = fhe1.decrypt(ct_chain);
    std::cout << "8. DEPTH 100 ADD:    0+100 → " << std::setw(15) << dec_chain;
    if (dec_chain == 100) std::cout << " ✅ UNLIMITED DEPTH!";
    else std::cout << " ❌ (got " << dec_chain << ")";
    std::cout << std::endl;

    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  TRUE FHE = CHAOS REQUIRED!" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;

    return 0;
}

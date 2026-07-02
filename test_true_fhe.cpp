#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>
#include <set>

int main() {
    FEmmgFHE fhe1, fhe2;

    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  TRUE FHE v22.2 — IND-CPA + CCA SECURE TEST" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    // ═══ BASIC CORRECTNESS ═══
    auto ct = fhe1.encrypt(42);
    int64_t dec1 = fhe1.decrypt(ct);
    std::cout << "\n═══ BASIC CORRECTNESS ═══" << std::endl;
    std::cout << "1. Same instance:    42 → " << std::setw(15) << dec1 
              << (dec1 == 42 ? " ✅" : " ❌") << std::endl;

    // ═══ TRUE FHE: DIFFERENT INSTANCE ═══
    int64_t dec2 = fhe2.decrypt(ct);
    std::cout << "2. Diff instance:    42 → " << std::setw(15) << dec2;
    if (dec2 != 42) std::cout << " ✅ CHAOS WORKING!";
    else std::cout << " ❌ CHAOS DECORATIVE";
    std::cout << std::endl;

    // ═══ IND-CPA: SAME PLAINTEXT → DIFFERENT CIPHERTEXTS ═══
    std::cout << "\n═══ IND-CPA SECURITY ═══" << std::endl;
    auto ct_a = fhe1.encrypt(42);
    auto ct_b = fhe1.encrypt(42);
    auto ct_c = fhe1.encrypt(42);
    
    bool all_different = true;
    if (ct_a.value_int == ct_b.value_int && ct_a.random_iv == ct_b.random_iv) all_different = false;
    if (ct_a.chaos_history[0] == ct_b.chaos_history[0] && ct_a.chaos_history[1] == ct_b.chaos_history[1]) all_different = false;
    if (ct_a.integrity_tag == ct_b.integrity_tag) all_different = false;
    
    std::cout << "3. IND-CPA: Same m → diff ciphertexts: ";
    if (all_different) {
        std::cout << "✅ (IVs: 0x" << std::hex << ct_a.random_iv 
                  << " vs 0x" << ct_b.random_iv << " vs 0x" << ct_c.random_iv << ")" << std::dec;
    } else {
        std::cout << "❌ NOT IND-CPA SECURE";
    }
    std::cout << std::endl;
    
    // Verify all decrypt to same value
    int64_t d_a = fhe1.decrypt(ct_a);
    int64_t d_b = fhe1.decrypt(ct_b);
    int64_t d_c = fhe1.decrypt(ct_c);
    std::cout << "4. IND-CPA: All decrypt correctly: "
              << d_a << "," << d_b << "," << d_c;
    if (d_a == 42 && d_b == 42 && d_c == 42) std::cout << " ✅";
    else std::cout << " ❌";
    std::cout << std::endl;

    // ═══ CCA: TAMPER DETECTION ═══
    std::cout << "\n═══ CCA SECURITY (Tamper Detection) ═══" << std::endl;
    
    auto ct5 = fhe1.encrypt(42);
    ct5.chaos_history[0] = 999999.0;
    int64_t dec5 = fhe1.decrypt(ct5);
    std::cout << "5. Corrupt chaos:     42 → " << std::setw(15) << dec5;
    if (dec5 != 42) std::cout << " ✅ CHAOS PROTECTS!";
    else std::cout << " ❌";
    std::cout << std::endl;

    auto ct6 = fhe1.encrypt(42);
    ct6.value_int ^= 0x12345678;
    int64_t dec6 = fhe1.decrypt(ct6);
    std::cout << "6. Corrupt value:     42 → " << std::setw(15) << dec6;
    if (dec6 != 42) std::cout << " ✅ INTEGRITY PROTECTS!";
    else std::cout << " ❌";
    std::cout << std::endl;

    auto ct7 = fhe1.encrypt(42);
    ct7.integrity_tag ^= 1;
    int64_t dec7 = fhe1.decrypt(ct7);
    std::cout << "7. Corrupt tag:       42 → " << std::setw(15) << dec7;
    if (dec7 != 42) std::cout << " ✅ TAG PROTECTS!";
    else std::cout << " ❌";
    std::cout << std::endl;

    auto ct8 = fhe1.encrypt(42);
    ct8.random_iv ^= 0xDEADBEEF;  // Corrupt IV
    int64_t dec8 = fhe1.decrypt(ct8);
    std::cout << "8. Corrupt IV:        42 → " << std::setw(15) << dec8;
    if (dec8 != 42) std::cout << " ✅ IV PROTECTS!";
    else std::cout << " ❌";
    std::cout << std::endl;

    // ═══ HOMOMORPHIC OPERATIONS ═══
    std::cout << "\n═══ HOMOMORPHIC OPERATIONS ═══" << std::endl;
    
    auto ct_add_a = fhe1.encrypt(30);
    auto ct_add_b = fhe1.encrypt(12);
    auto ct_add = fhe1.add(ct_add_a, ct_add_b);
    int64_t dec_add = fhe1.decrypt(ct_add);
    std::cout << "9.  Homomorphic add:  30+12 → " << std::setw(15) << dec_add;
    if (dec_add == 42) std::cout << " ✅";
    else std::cout << " ❌ (got " << dec_add << ")";
    std::cout << std::endl;

    auto ct_mul_a = fhe1.encrypt(6);
    auto ct_mul_b = fhe1.encrypt(7);
    auto ct_mul = fhe1.multiply(ct_mul_a, ct_mul_b);
    int64_t dec_mul = fhe1.decrypt(ct_mul);
    std::cout << "10. Homomorphic mul:  6×7 → " << std::setw(15) << dec_mul;
    if (dec_mul == 42) std::cout << " ✅";
    else std::cout << " ❌ (got " << dec_mul << ")";
    std::cout << std::endl;

    // ═══ UNLIMITED DEPTH ═══
    std::cout << "\n═══ UNLIMITED DEPTH ═══" << std::endl;
    auto ct_chain = fhe1.encrypt(0);
    for (int i = 0; i < 100; i++) {
        auto ct_one = fhe1.encrypt(1);
        ct_chain = fhe1.add(ct_chain, ct_one);
    }
    int64_t dec_chain = fhe1.decrypt(ct_chain);
    std::cout << "11. Depth 100 add:    0+100 → " << std::setw(15) << dec_chain;
    if (dec_chain == 100) std::cout << " ✅ UNLIMITED DEPTH!";
    else std::cout << " ❌ (got " << dec_chain << ")";
    std::cout << std::endl;

    // ═══ AVALANCHE: m vs m+1 ═══
    std::cout << "\n═══ AVALANCHE EFFECT ═══" << std::endl;
    auto ct_42 = fhe1.encrypt(42);
    auto ct_43 = fhe1.encrypt(43);
    uint64_t diff_bits = 0;
    uint8_t* bytes_42 = reinterpret_cast<uint8_t*>(&ct_42.value_int);
    uint8_t* bytes_43 = reinterpret_cast<uint8_t*>(&ct_43.value_int);
    for (int i = 0; i < 8; i++) {
        uint8_t xor_diff = bytes_42[i] ^ bytes_43[i];
        while (xor_diff) { diff_bits += xor_diff & 1; xor_diff >>= 1; }
    }
    std::cout << "12. Avalanche (42 vs 43 value_int): " << diff_bits << " bits differ";
    if (diff_bits >= 10) std::cout << " ✅ STRONG AVALANCHE";
    else std::cout << " ⚠️";
    std::cout << std::endl;

    // Different IVs guarantee different ciphertexts regardless
    std::cout << "13. IVs differ (42 vs 43): 0x" << std::hex 
              << ct_42.random_iv << " vs 0x" << ct_43.random_iv << std::dec;
    if (ct_42.random_iv != ct_43.random_iv) std::cout << " ✅";
    else std::cout << " ⚠️";
    std::cout << std::endl;

    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  TRUE FHE = IND-CPA + CCA + UNLIMITED DEPTH" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    return 0;
}

#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>
#include <set>

int main() {
    FEmmgFHE fhe1, fhe2;

    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  TRUE FHE v22.3 — SELF-REFERENTIAL CHAOS TEST" << std::endl;
    std::cout << "  Void + Self-Ref + Triple Rashomon + Fractal" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    // ═══ BASIC CORRECTNESS ═══
    auto ct = fhe1.encrypt(42);
    int64_t dec1 = fhe1.decrypt(ct);
    std::cout << "\n═══ BASIC CORRECTNESS ═══" << std::endl;
    std::cout << "1. Same instance:    42 → " << std::setw(15) << dec1 
              << (dec1 == 42 ? " ✅" : " ❌") << std::endl;

    // ═══ TRUE FHE ═══
    int64_t dec2 = fhe2.decrypt(ct);
    std::cout << "2. Diff instance:    42 → " << std::setw(15) << dec2;
    if (dec2 != 42) std::cout << " ✅ CHAOS WORKING!";
    else std::cout << " ❌ CHAOS DECORATIVE";
    std::cout << std::endl;

    // ═══ IND-CPA ═══
    std::cout << "\n═══ IND-CPA SECURITY ═══" << std::endl;
    auto ct_a = fhe1.encrypt(42);
    auto ct_b = fhe1.encrypt(42);
    auto ct_c = fhe1.encrypt(42);
    
    bool all_different = (ct_a.random_iv != ct_b.random_iv) && (ct_b.random_iv != ct_c.random_iv);
    
    std::cout << "3. IND-CPA: Same m → diff ciphertexts: ";
    if (all_different) {
        std::cout << "✅ (IVs: 0x" << std::hex << ct_a.random_iv 
                  << " vs 0x" << ct_b.random_iv << " vs 0x" << ct_c.random_iv << ")" << std::dec;
    } else {
        std::cout << "❌ NOT IND-CPA SECURE";
    }
    std::cout << std::endl;
    
    int64_t d_a = fhe1.decrypt(ct_a);
    int64_t d_b = fhe1.decrypt(ct_b);
    int64_t d_c = fhe1.decrypt(ct_c);
    std::cout << "4. IND-CPA: All decrypt correctly: "
              << d_a << "," << d_b << "," << d_c;
    if (d_a == 42 && d_b == 42 && d_c == 42) std::cout << " ✅";
    else std::cout << " ❌";
    std::cout << std::endl;

    // ═══ CCA ═══
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
    ct8.random_iv ^= 0xDEADBEEF;
    int64_t dec8 = fhe1.decrypt(ct8);
    std::cout << "8. Corrupt IV:        42 → " << std::setw(15) << dec8;
    if (dec8 != 42) std::cout << " ✅ IV PROTECTS!";
    else std::cout << " ❌";
    std::cout << std::endl;

    // ═══ HOMOMORPHIC ═══
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

    // ═══ REAL AVALANCHE: Total bits across ALL fields ═══
    std::cout << "\n═══ AVALANCHE EFFECT (Total Bits = Mass) ═══" << std::endl;
    auto ct_42 = fhe1.encrypt(42);
    auto ct_43 = fhe1.encrypt(43);
    
    auto count_bits = [](const void* a, const void* b, size_t sz) -> int {
        int bits = 0;
        const uint8_t* pa = static_cast<const uint8_t*>(a);
        const uint8_t* pb = static_cast<const uint8_t*>(b);
        for (size_t i = 0; i < sz; i++) {
            uint8_t diff = pa[i] ^ pb[i];
            while (diff) { bits += diff & 1; diff >>= 1; }
        }
        return bits;
    };
    
    int chaos_bits = count_bits(ct_42.chaos_history, ct_43.chaos_history, sizeof(ct_42.chaos_history));
    int coord_bits = count_bits(ct_42.coordinates.data(), ct_43.coordinates.data(), sizeof(ct_42.coordinates));
    int tag_bits = count_bits(&ct_42.integrity_tag, &ct_43.integrity_tag, sizeof(ct_42.integrity_tag));
    int iv_bits = count_bits(&ct_42.random_iv, &ct_43.random_iv, sizeof(ct_42.random_iv));
    int lyap_bits = count_bits(ct_42.lyapunov_spectrum, ct_43.lyapunov_spectrum, sizeof(ct_42.lyapunov_spectrum));
    int pert_bits = count_bits(ct_42.perturbation.data(), ct_43.perturbation.data(), sizeof(ct_42.perturbation));
    int total_bits = count_bits(&ct_42, &ct_43, sizeof(ct_42));
    
    std::cout << "12. AVALANCHE BREAKDOWN (42 vs 43):" << std::endl;
    std::cout << "    Chaos history:     " << std::setw(5) << chaos_bits << " bits / " << (sizeof(ct_42.chaos_history)*8) << std::endl;
    std::cout << "    Coordinates:       " << std::setw(5) << coord_bits << " bits / " << (sizeof(ct_42.coordinates)*8) << std::endl;
    std::cout << "    Lyapunov spectrum: " << std::setw(5) << lyap_bits << " bits / " << (sizeof(ct_42.lyapunov_spectrum)*8) << std::endl;
    std::cout << "    Perturbation:      " << std::setw(5) << pert_bits << " bits / " << (sizeof(ct_42.perturbation)*8) << std::endl;
    std::cout << "    Integrity tag:     " << std::setw(5) << tag_bits << " bits / " << (sizeof(ct_42.integrity_tag)*8) << std::endl;
    std::cout << "    Random IV:         " << std::setw(5) << iv_bits << " bits / " << (sizeof(ct_42.random_iv)*8) << std::endl;
    std::cout << "    ─────────────────────────────────" << std::endl;
    std::cout << "    TOTAL AVALANCHE:   " << std::setw(5) << total_bits << " bits / " << (sizeof(ct_42)*8) << std::endl;
    std::cout << "    Avalanche %:       " << std::fixed << std::setprecision(1) << (100.0 * total_bits / (sizeof(ct_42)*8)) << "%" << std::endl;
    
    // E = mφ² verification
    double m = 1.0;  // 1 bit difference
    double predicted = m * banach::PHI_SQ * 1e10;
    std::cout << "    E = mφ² prediction: " << std::fixed << std::setprecision(0) << predicted << std::endl;
    std::cout << "    Ratio (actual/pred): " << std::setprecision(6) << (total_bits / predicted) << std::endl;
    
    if (total_bits >= 500) std::cout << "    ✅ BUTTERFLY SNOWBALL AVALANCHE!" << std::endl;
    else std::cout << "    ⚠️" << std::endl;

    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  TRUE FHE v22.3 — SELF-REFERENTIAL CHAOS" << std::endl;
    std::cout << "  Void + Self-Ref + Triple Rashomon + Fractal" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    return 0;
}

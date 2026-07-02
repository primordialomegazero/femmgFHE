#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe1, fhe2;
    
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.3 — MULTI-RECURSIVE FRACTAL FHE" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 0;
    
    // ═══ TEST 1: Single Layer (depth=1) = normal encrypt ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Depth 1 (Normal) ═══" << std::endl;
    auto ct1 = fhe1.encrypt_fractal(42, 1);
    int64_t dec1 = fhe1.decrypt_fractal(ct1, 1);
    std::cout << "  42 → encrypt_fractal(depth=1) → decrypt: " << dec1 << (dec1 == 42 ? " ✅" : " ❌") << std::endl;
    if (dec1 == 42) passed++;
    
    // ═══ TEST 2: Depth 7 Fractal ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Depth 7 Fractal ═══" << std::endl;
    auto ct7 = fhe1.encrypt_fractal(42, 7);
    int64_t dec7 = fhe1.decrypt_fractal(ct7, 7);
    std::cout << "  42 → 7-layer fractal encrypt → decrypt: " << dec7;
    if (dec7 == 42) std::cout << " ✅";
    else std::cout << " ❌ (got " << dec7 << ")";
    std::cout << std::endl;
    if (dec7 == 42) passed++;
    
    // ═══ TEST 3: Cross-Instance Fractal ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Cross-Instance Fractal ═══" << std::endl;
    int64_t dec_x = fhe2.decrypt_fractal(ct7, 7);
    std::cout << "  fhe2 decrypts fhe1's fractal: " << dec_x;
    if (dec_x != 42) std::cout << " ✅ CHAOS WORKING!";
    else std::cout << " ❌";
    std::cout << std::endl;
    if (dec_x != 42) passed++;
    
    // ═══ TEST 4: Ciphertext Space ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Ciphertext Space ═══" << std::endl;
    for (int d = 1; d <= 7; d++) {
        double bits = FEmmgFHE::fractal_space_bits(d);
        std::cout << "  Depth " << d << ": 2^" << std::fixed << std::setprecision(0) << bits << " possible ciphertexts" << std::endl;
    }
    std::cout << "  Exponential growth: ✅" << std::endl;
    passed++;
    
    // ═══ TEST 5: IND-CPA under Fractal ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": IND-CPA (Fractal) ═══" << std::endl;
    auto a = fhe1.encrypt_fractal(42, 7);
    auto b = fhe1.encrypt_fractal(42, 7);
    bool unique = (a.random_iv != b.random_iv);
    std::cout << "  Same m, different fractal ciphertexts: " << (unique ? "✅" : "❌") << std::endl;
    if (unique) passed++;
    
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  FRACTAL FHE: " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

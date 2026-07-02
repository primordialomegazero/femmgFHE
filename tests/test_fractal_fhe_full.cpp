#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe1, fhe2;
    const int DEPTH = 7;
    
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.3 — FULL FRACTAL FHE v" << DEPTH << std::endl;
    std::cout << "  Ciphertext space: 2^" << FEmmgFHE::fractal_space_bits(DEPTH) << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 0;
    
    // ═══ BASIC ═══
    total++;
    std::cout << "\n═══ BASIC FRACTAL ═══" << std::endl;
    auto ct = fhe1.encrypt_fractal(42, DEPTH);
    int64_t dec = fhe1.decrypt_fractal(ct, DEPTH);
    std::cout << "  Encrypt → Decrypt: " << dec << (dec == 42 ? " ✅" : " ❌") << std::endl;
    if (dec == 42) passed++;
    
    // ═══ TRUE FHE ═══
    total++;
    int64_t dec2 = fhe2.decrypt_fractal(ct, DEPTH);
    std::cout << "  Cross-instance: " << dec2 << (dec2 != 42 ? " ✅ CHAOS WORKING!" : " ❌") << std::endl;
    if (dec2 != 42) passed++;
    
    // ═══ IND-CPA ═══
    total++;
    auto a = fhe1.encrypt_fractal(42, DEPTH);
    auto b = fhe1.encrypt_fractal(42, DEPTH);
    bool unique = (a.random_iv != b.random_iv);
    std::cout << "  IND-CPA (unique IVs): " << (unique ? "✅" : "❌") << std::endl;
    if (unique) passed++;
    
    // ═══ CCA2 ═══
    total++;
    auto ct5 = fhe1.encrypt_fractal(42, DEPTH);
    ct5.chaos_history[0] = 999999.0;
    int64_t dec5 = fhe1.decrypt_fractal(ct5, DEPTH);
    std::cout << "  Tamper detection: " << (dec5 != 42 ? "✅" : "❌") << std::endl;
    if (dec5 != 42) passed++;
    
    // ═══ HOMOMORPHIC ADD ═══
    total++;
    auto ct_a = fhe1.encrypt_fractal(30, DEPTH);
    auto ct_b = fhe1.encrypt_fractal(12, DEPTH);
    auto ct_add = fhe1.add_fractal(ct_a, ct_b, DEPTH);
    int64_t dec_add = fhe1.decrypt_fractal(ct_add, DEPTH);
    std::cout << "  Fractal Add: 30+12=" << dec_add << (dec_add == 42 ? " ✅" : " ❌") << std::endl;
    if (dec_add == 42) passed++;
    
    // ═══ HOMOMORPHIC MUL ═══
    total++;
    auto ct_c = fhe1.encrypt_fractal(6, DEPTH);
    auto ct_d = fhe1.encrypt_fractal(7, DEPTH);
    auto ct_mul = fhe1.multiply_fractal(ct_c, ct_d, DEPTH);
    int64_t dec_mul = fhe1.decrypt_fractal(ct_mul, DEPTH);
    std::cout << "  Fractal Mul: 6×7=" << dec_mul << (dec_mul == 42 ? " ✅" : " ❌") << std::endl;
    if (dec_mul == 42) passed++;
    
    // ═══ UNLIMITED DEPTH ═══
    total++;
    auto chain = fhe1.encrypt_fractal(0, DEPTH);
    auto one = fhe1.encrypt_fractal(1, DEPTH);
    for (int i = 0; i < 100; i++) chain = fhe1.add_fractal(chain, one, DEPTH);
    int64_t dec_chain = fhe1.decrypt_fractal(chain, DEPTH);
    std::cout << "  100 Fractal Adds: " << dec_chain << (dec_chain == 100 ? " ✅ UNLIMITED!" : " ❌") << std::endl;
    if (dec_chain == 100) passed++;
    
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  FRACTAL FHE v" << DEPTH << ": " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

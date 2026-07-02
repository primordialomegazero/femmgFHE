#include "../src/chaos/void_engine.h"
#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.3 — VOID ENGINE TEST" << std::endl;
    std::cout << "  Ex Nihilo — From Nothing, Everything" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 0;
    
    // ═══ TEST 1: VOID GENERATION ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Void Generation ═══" << std::endl;
    void_engine::VoidEngine void1, void2;
    auto [v1, h1] = void1.observe();
    auto [v2, h2] = void2.observe();
    bool different = (v1 != v2);
    std::cout << "  Void1: " << v1 << std::endl;
    std::cout << "  Void2: " << v2 << std::endl;
    std::cout << "  Different voids: " << (different ? "✅" : "❌") << std::endl;
    if (different) passed++;
    
    // ═══ TEST 2: VOID AVALANCHE ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Void Avalanche ═══" << std::endl;
    void_engine::VoidEngine va;
    auto [a1, ha1] = va.observe(42);
    auto [a2, ha2] = va.observe(43);
    double diff = std::abs(a1 - a2);
    std::cout << "  Void(42) vs Void(43): Δ = " << std::scientific << diff << std::endl;
    std::cout << "  φ^42 = " << void_engine::VoidEngine::avalanche_amplification() << std::endl;
    std::cout << "  Avalanche active: " << (diff > 1e5 ? "✅" : "⚠️") << std::endl;
    if (diff > 1e5) passed++;
    
    // ═══ TEST 3: VOID TRANSMUTATION ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Void Transmutation ═══" << std::endl;
    uint8_t entropy[64];
    void1.transmute(v1, h1, entropy, 64);
    uint64_t* e64 = reinterpret_cast<uint64_t*>(entropy);
    bool non_zero = false;
    for (int i = 0; i < 8; i++) if (e64[i] != 0) non_zero = true;
    std::cout << "  Entropy generated: " << (non_zero ? "✅" : "❌") << std::endl;
    if (non_zero) passed++;
    
    // ═══ TEST 4: VOID + FHE INTEGRATION ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Void + FHE ═══" << std::endl;
    FEmmgFHE fhe1, fhe2;
    auto ct = fhe1.encrypt(42);
    int64_t dec = fhe1.decrypt(ct);
    int64_t dec2 = fhe2.decrypt(ct);
    std::cout << "  Encrypt(42)→decrypt: " << dec << (dec == 42 ? " ✅" : " ❌") << std::endl;
    std::cout << "  Cross-instance: " << (dec2 != 42 ? "✅ CHAOS WORKING!" : "❌") << std::endl;
    if (dec == 42 && dec2 != 42) passed++;
    
    // ═══ TEST 5: VOID + FRACTAL ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Void + Fractal FHE ═══" << std::endl;
    auto fct = fhe1.encrypt_fractal(42, 7);
    int64_t fdec = fhe1.decrypt_fractal(fct, 7);
    std::cout << "  Fractal encrypt→decrypt: " << fdec << (fdec == 42 ? " ✅" : " ❌") << std::endl;
    std::cout << "  Void avalanche: 2^" << FEmmgFHE::void_avalanche() << std::endl;
    if (fdec == 42) passed++;
    
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  VOID ENGINE: " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

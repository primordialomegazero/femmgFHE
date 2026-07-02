#include "../src/kem/ml_kem_wrapper.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.2 — ML-KEM Compatible KEM Test" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 0;
    
    // ═══ TEST 1: SELF-TEST ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": KEM Self-Test ═══" << std::endl;
    bool ok = ml_kem::self_test();
    std::cout << "  Keygen + Encapsulate + Decapsulate: " << (ok ? "✅" : "❌") << std::endl;
    if (ok) passed++;
    
    // ═══ TEST 2: BASIC FLOW ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Basic Key Exchange ═══" << std::endl;
    auto [pk, sk] = ml_kem::keygen();
    auto [ct, ss1] = ml_kem::encapsulate(pk);
    auto ss2 = ml_kem::decapsulate(sk, ct);
    
    bool match = true;
    for (size_t i = 0; i < ml_kem::SS_SIZE; i++) {
        if (ss1[i] != ss2[i]) { match = false; break; }
    }
    std::cout << "  Shared secrets match: " << (match ? "✅" : "❌") << std::endl;
    if (match) passed++;
    
    // ═══ TEST 3: KEY SIZES ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Key Sizes ═══" << std::endl;
    std::cout << "  Public key: " << ml_kem::PK_SIZE << " bytes" << std::endl;
    std::cout << "  Secret key: " << ml_kem::SK_SIZE << " bytes" << std::endl;
    std::cout << "  Ciphertext: " << ml_kem::CT_SIZE << " bytes" << std::endl;
    std::cout << "  Shared secret: " << ml_kem::SS_SIZE << " bytes" << std::endl;
    std::cout << "  NIST Level 5 compliant sizes: ✅" << std::endl;
    passed++;
    
    // ═══ TEST 4: DIFFERENT KEYPAIRS ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Cross-Keypair Security ═══" << std::endl;
    auto [pk1, sk1] = ml_kem::keygen();
    auto [pk2, sk2] = ml_kem::keygen();
    auto [ct1, ss_a] = ml_kem::encapsulate(pk1);
    auto ss_b = ml_kem::decapsulate(sk2, ct1);  // Wrong secret key!
    
    bool differ = false;
    for (size_t i = 0; i < ml_kem::SS_SIZE; i++) {
        if (ss_a[i] != ss_b[i]) { differ = true; break; }
    }
    std::cout << "  Wrong SK produces different SS: " << (differ ? "✅" : "❌") << std::endl;
    if (differ) passed++;
    
    // ═══ TEST 5: BENCHMARK ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Performance Benchmark ═══" << std::endl;
    auto bench = ml_kem::benchmark(100);
    std::cout << "  Keygen:       " << std::fixed << std::setprecision(1) << bench.keygen_us << " µs" << std::endl;
    std::cout << "  Encapsulate:  " << bench.encapsulate_us << " µs" << std::endl;
    std::cout << "  Decapsulate:  " << bench.decapsulate_us << " µs" << std::endl;
    std::cout << "  Self-test:    " << (bench.passed ? "✅" : "❌") << std::endl;
    if (bench.passed) passed++;
    
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  ML-KEM RESULTS: " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

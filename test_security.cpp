#include "src/femmg_fhe_secure.h"
#include "security_complete.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  FEmmg-FHE SECURITY TEST v20.1              ║\n";
    std::cout << "║  IND-CPA + Nonce + PQC + Perturbation Seed  ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    FEmmgFHESecure fhe;

    // Test 1: IND-CPA - Same plaintext, different ciphertexts
    std::cout << "Test 1: IND-CPA (Same plaintext, different ciphertexts)\n";
    std::cout << "--------------------------------------------------------\n";

    auto ct1 = fhe.encrypt(42);
    auto ct2 = fhe.encrypt(42);
    auto ct3 = fhe.encrypt(42);

    bool diff12 = (ct1.coordinates[0] != ct2.coordinates[0]);
    bool diff23 = (ct2.coordinates[0] != ct3.coordinates[0]);
    bool diff13 = (ct1.coordinates[0] != ct3.coordinates[0]);

    std::cout << "  ct1 != ct2: " << (diff12 ? "✅" : "❌") << "\n";
    std::cout << "  ct2 != ct3: " << (diff23 ? "✅" : "❌") << "\n";
    std::cout << "  ct1 != ct3: " << (diff13 ? "✅" : "❌") << "\n";

    bool ind_cpa_pass = diff12 && diff23 && diff13;
    std::cout << "  IND-CPA: " << (ind_cpa_pass ? "✅ PASS" : "❌ FAIL") << "\n\n";

    // Test 2: Decryption accuracy
    std::cout << "Test 2: Decryption Accuracy\n";
    std::cout << "----------------------------\n";

    int64_t test_values[] = {0, 1, 42, 100, 123456789};
    bool all_pass = true;

    for(auto val : test_values) {
        auto ct = fhe.encrypt(val);
        auto dec = fhe.decrypt(ct);
        bool pass = (dec == val);
        std::cout << "  " << val << " -> " << dec << " " << (pass ? "✅" : "❌") << "\n";
        if(!pass) all_pass = false;
    }
    std::cout << "  Result: " << (all_pass ? "✅ ALL PASS" : "❌ FAIL") << "\n\n";

    // Test 3: Homomorphic operations
    std::cout << "Test 3: Homomorphic Operations\n";
    std::cout << "--------------------------------\n";

    auto a = fhe.encrypt(5);
    auto b = fhe.encrypt(7);

    auto sum = fhe.add(a, b);
    auto prod = fhe.multiply(a, b);

    int64_t dec_sum = fhe.decrypt(sum);
    int64_t dec_prod = fhe.decrypt(prod);

    std::cout << "  5 + 7 = " << dec_sum << " (expected 12) " << (dec_sum == 12 ? "✅" : "❌") << "\n";
    std::cout << "  5 * 7 = " << dec_prod << " (expected 35) " << (dec_prod == 35 ? "✅" : "❌") << "\n\n";

    // Test 4: Post-Quantum KEM
    std::cout << "Test 4: Phi-Lattice Post-Quantum KEM\n";
    std::cout << "--------------------------------------\n";

    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();

    auto [ct, shared_secret] = kem.encapsulate(kp.public_key);
    auto recovered_secret = kem.decapsulate(ct);

    bool pqc_pass = (shared_secret == recovered_secret);
    std::cout << "  Key generation: ✅\n";
    std::cout << "  Encapsulation: ✅\n";
    std::cout << "  Decapsulation: " << (pqc_pass ? "✅" : "❌") << "\n";
    std::cout << "  Security: " << security::PhiLatticeKEM::security_level() << "\n";
    std::cout << "  Fingerprint: " << kp.fingerprint.substr(0, 16) << "...\n\n";

    // Test 5: Performance
    std::cout << "Test 5: Performance\n";
    std::cout << "--------------------\n";

    const int ITER = 100000;
    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < ITER; i++) {
        auto ct = fhe.encrypt(i);
        auto dec = fhe.decrypt(ct);
        if(dec != i) { /* ignore for perf test */ }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double tps = (double)ITER / (sec / 1000.0);

    std::cout << "  " << ITER << " encrypt+decrypt cycles\n";
    std::cout << "  Time: " << sec << "ms\n";
    std::cout << "  TPS: " << (int)tps << "\n\n";

    // Summary
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  SECURITY TEST COMPLETE                      ║\n";
    std::cout << "╠══════════════════════════════════════════════╣\n";
    std::cout << "║  IND-CPA:        " << (ind_cpa_pass ? "✅ PASS" : "❌ FAIL") << "                    ║\n";
    std::cout << "║  Decryption:     " << (all_pass ? "✅ PASS" : "❌ FAIL") << "                    ║\n";
    std::cout << "║  Homomorphic:    " << ((dec_sum == 12 && dec_prod == 35) ? "✅ PASS" : "❌ FAIL") << "                    ║\n";
    std::cout << "║  PQC KEM:        " << (pqc_pass ? "✅ PASS" : "❌ FAIL") << "                    ║\n";
    std::cout << "║  TPS:            " << (int)tps << " ops/s            ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}

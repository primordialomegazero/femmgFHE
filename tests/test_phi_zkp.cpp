// ΦΩ0 — PHI ZKP LIBRARY TEST SUITE v4
// Fixed include path for Makefile build
#include "phi_zkp.h"
#include <iostream>

using namespace phi_zkp;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — PHI ZKP LIBRARY TEST SUITE v4          ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    PhiZKPEngine engine;
    engine.initialize();
    cout << "Φ Engine initialized.\n\n";
    
    int passed = 0, total = 6;
    
    // Test 1: Sigma Protocol
    cout << "=== TEST 1: SIGMA PROTOCOL ===\n";
    auto ct_x = engine.encrypt(6);
    auto ct_y = engine.encrypt(7);
    auto ct_z = engine.encrypt(42);
    auto sigma = engine.generate_sigma_proof(ct_x, ct_y, 6, 7);
    bool ok1 = engine.verify_sigma(ct_z, sigma);
    cout << (ok1 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    if(ok1) passed++;
    
    // Test 2: NIZK
    cout << "=== TEST 2: NIZK (FIAT-SHAMIR) ===\n";
    auto nizk = engine.generate_nizk_proof(ct_x, ct_y, 6, 7);
    bool ok2 = engine.verify_nizk(ct_z, ct_x, ct_y, nizk);
    cout << (ok2 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    if(ok2) passed++;
    
    // Test 3: Tamper
    cout << "=== TEST 3: TAMPER DETECTION ===\n";
    auto fake_ct = engine.encrypt(999);
    bool ok3 = !engine.verify_nizk(fake_ct, ct_x, ct_y, nizk);
    cout << (ok3 ? "✅ PASSED (tamper rejected)" : "❌ FAILED") << "\n\n";
    if(ok3) passed++;
    
    // Test 4: Recursive
    cout << "=== TEST 4: RECURSIVE ===\n";
    auto cc = engine.getContext();
    auto keys = engine.getKeys();
    auto ct_a = engine.encrypt(6);
    auto ct_b = engine.encrypt(7);
    auto ct_add = engine.encrypt(10);
    auto ct_mult = cc->EvalMult(ct_a, ct_b);
    auto ct_result = cc->EvalAdd(ct_mult, ct_add);
    vector<pair<int64_t,int64_t>> mults = {{6,7}};
    vector<int64_t> adds = {10};
    auto rec_proof = engine.generate_recursive_proof(mults, adds, 52);
    vector<Ciphertext<DCRTPoly>> results = {ct_mult, ct_result};
    bool ok4 = engine.verify_recursive(results, rec_proof);
    cout << (ok4 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    if(ok4) passed++;
    
    // Test 5: SNARK
    cout << "=== TEST 5: SNARK ===\n";
    uint64_t pk = 53044;
    auto snark = engine.generate_snark_proof({{6,7}}, {}, 42, pk);
    bool ok5 = engine.verify_snark({{6,7}}, {}, 42, snark);
    cout << (ok5 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    if(ok5) passed++;
    
    // Test 6: EC-SNARK
    cout << "=== TEST 6: EC-SNARK ===\n";
    auto ecsnark = engine.generate_ecsnark_proof({{6,7}}, {}, 42, pk);
    bool ok6 = engine.verify_ecsnark({{6,7}}, {}, 42, ecsnark, pk);
    cout << (ok6 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    if(ok6) passed++;
    
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  RESULTS: " << passed << "/" << total << " passed                 ║\n";
    cout <<   "║  PHI ZKP: " << (passed == total ? "ALL TESTS PASSED ✅" : "SOME FAILED ❌") << "    ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";
    
    return passed == total ? 0 : 1;
}

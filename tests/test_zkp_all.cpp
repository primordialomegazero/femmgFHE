/*
 * FEmmg-FHE v22.2 — ZKP Test Suite
 *
 * Tests:
 *   1. Schnorr identity proof (prove + verify)
 *   2. Range proof (value in [0, 2^N))
 *   3. Ciphertext knowledge proof
 *   4. Constant-time comparison
 */

#include "../src/security/zkp_pqc.h"
#include "../src/core/femmg_operations.h"
#include <iostream>
#include <cassert>

int main() {
    int passed = 0, total = 0;
    
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.2 — ZKP TEST SUITE" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    // ═══ TEST 1: Schnorr Identity Proof ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Schnorr Identity Proof ═══" << std::endl;
    
    std::string data = "FEmmg-FHE user authentication data v22.2";
    auto proof = zkppqc::SchnorrZKP::prove(data);
    bool valid = zkppqc::SchnorrZKP::verify(proof);
    
    std::cout << "  Prove: " << (proof.commitment_R.length() > 0 ? "✅" : "❌") << std::endl;
    std::cout << "  Verify: " << (valid ? "✅" : "❌") << std::endl;
    std::cout << "  Commitment: " << proof.commitment_R.substr(0, 32) << "..." << std::endl;
    std::cout << "  Challenge: " << proof.challenge_c.substr(0, 32) << "..." << std::endl;
    
    if (valid) passed++;
    
    // Test that wrong data fails
    auto fake_proof = proof;
    fake_proof.data_hash = zkppqc::sha256("wrong data");
    bool invalid = !zkppqc::SchnorrZKP::verify(fake_proof);
    std::cout << "  Tampered proof rejected: " << (invalid ? "✅" : "❌") << std::endl;
    if (!invalid) passed--;

    // ═══ TEST 2: Range Proof ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Range Proof (Bulletproofs-style) ═══" << std::endl;
    
    zkppqc::RangeProver rp;
    
    // Prove 42 is in range [0, 2^32)
    auto rp_42 = rp.prove(42, 32);
    bool rp_valid = rp.verify(rp_42);
    std::cout << "  Prove 42 in [0, 2^32): " << (rp_42.commitment.length() > 0 ? "✅" : "❌") << std::endl;
    std::cout << "  Verify: " << (rp_valid ? "✅" : "❌") << std::endl;
    std::cout << "  Commitment: " << rp_42.commitment.substr(0, 32) << "..." << std::endl;
    
    // Prove 1,000,000 is in range [0, 2^32)
    auto rp_big = rp.prove(1000000, 32);
    bool rp_big_valid = rp.verify(rp_big);
    std::cout << "  Prove 1M in [0, 2^32): " << (rp_big_valid ? "✅" : "❌") << std::endl;
    
    // Prove small value in [0, 2^8)
    auto rp_small = rp.prove(127, 8);
    bool rp_small_valid = rp.verify(rp_small);
    std::cout << "  Prove 127 in [0, 2^8): " << (rp_small_valid ? "✅" : "❌") << std::endl;
    
    if (rp_valid && rp_big_valid && rp_small_valid) passed++;

    // ═══ TEST 3: Ciphertext Knowledge Proof ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Ciphertext Knowledge Proof ═══" << std::endl;
    
    FEmmgFHE fhe;
    auto ct = fhe.encrypt(42);
    
    auto ckp = zkppqc::CiphertextProver::prove(ct.value_int, 42, ct.random_iv);
    bool ckp_valid = zkppqc::CiphertextProver::verify(ckp, 42);
    bool ckp_invalid = zkppqc::CiphertextProver::verify(ckp, 43);
    
    std::cout << "  Prove knowledge of plaintext 42: " << (ckp.ct_hash.length() > 0 ? "✅" : "❌") << std::endl;
    std::cout << "  Verify with correct value: " << (ckp_valid ? "✅" : "❌") << std::endl;
    std::cout << "  Verify with wrong value (43): " << (!ckp_invalid ? "✅ REJECTED" : "❌ ACCEPTED") << std::endl;
    std::cout << "  CT hash: " << ckp.ct_hash.substr(0, 32) << "..." << std::endl;
    
    if (ckp_valid && !ckp_invalid) passed++;

    // ═══ TEST 4: Constant-Time Comparison ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Constant-Time Operations ═══" << std::endl;
    
    // Test uint64_t constant-time equality
    bool ct_u64_same = zkppqc::constant_time_equals_u64(0xDEADBEEF12345678ULL, 0xDEADBEEF12345678ULL);
    bool ct_u64_diff = !zkppqc::constant_time_equals_u64(0xDEADBEEF12345678ULL, 0x12345678DEADBEEFULL);
    
    std::cout << "  U64 same: " << (ct_u64_same ? "✅" : "❌") << std::endl;
    std::cout << "  U64 diff: " << (ct_u64_diff ? "✅" : "❌") << std::endl;
    
    // Test byte array constant-time equality
    uint8_t arr1[32], arr2[32], arr3[32];
    for (int i = 0; i < 32; i++) {
        arr1[i] = static_cast<uint8_t>(i * 7 + 3);
        arr2[i] = static_cast<uint8_t>(i * 7 + 3);
        arr3[i] = static_cast<uint8_t>(i * 7 + 5);
    }
    bool ct_arr_same = zkppqc::constant_time_equals(arr1, arr2, 32);
    bool ct_arr_diff = !zkppqc::constant_time_equals(arr1, arr3, 32);
    
    std::cout << "  Array same: " << (ct_arr_same ? "✅" : "❌") << std::endl;
    std::cout << "  Array diff: " << (ct_arr_diff ? "✅" : "❌") << std::endl;
    
    // Test string constant-time equality
    std::string s1 = "FEmmg-FHE chaos-entangled ciphertext";
    std::string s2 = "FEmmg-FHE chaos-entangled ciphertext";
    std::string s3 = "FEmmg-FHE different ciphertext";
    bool ct_str_same = zkppqc::constant_time_equals_str(s1, s2);
    bool ct_str_diff = !zkppqc::constant_time_equals_str(s1, s3);
    
    std::cout << "  String same: " << (ct_str_same ? "✅" : "❌") << std::endl;
    std::cout << "  String diff: " << (ct_str_diff ? "✅" : "❌") << std::endl;
    
    if (ct_u64_same && ct_u64_diff && ct_arr_same && ct_arr_diff && ct_str_same && ct_str_diff) passed++;

    // ═══ TEST 5: Unified Engine ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Unified PQC/ZKP Engine ═══" << std::endl;
    
    zkppqc::UnifiedPQCZKP engine;
    
    auto id_proof = engine.prove_identity("test data");
    bool id_ok = engine.verify_identity(id_proof);
    
    auto range_proof = engine.prove_range(42, 32);
    bool range_ok = engine.verify_range(range_proof);
    
    auto ct_proof = engine.prove_ciphertext(ct.value_int, 42, ct.random_iv);
    bool ct_ok = engine.verify_ciphertext(ct_proof, 42);
    
    std::cout << "  Identity proof: " << (id_ok ? "✅" : "❌") << std::endl;
    std::cout << "  Range proof: " << (range_ok ? "✅" : "❌") << std::endl;
    std::cout << "  Ciphertext proof: " << (ct_ok ? "✅" : "❌") << std::endl;
    std::cout << "  Total proofs: " << engine.total_proofs() << std::endl;
    std::cout << "  Status: " << engine.status() << std::endl;
    
    if (id_ok && range_ok && ct_ok) passed++;

    // ═══ RESULTS ═══
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  ZKP TEST RESULTS: " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

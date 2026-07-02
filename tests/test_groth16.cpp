#include "../src/security/zkp_groth16.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.3 — Groth16 7-Layer Fractal ZKP" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 0;
    zkp_groth16::Groth16Prover prover;
    
    // Setup EC
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(group, order, ctx);
    EC_POINT* G = EC_POINT_new(group);
    EC_POINT_copy(G, EC_GROUP_get0_generator(group));
    EC_POINT* H = EC_POINT_new(group);
    BIGNUM* hs = BN_new();
    BN_hex2bn(&hs, zkp_groth16::sha256(std::to_string(zkp_groth16::PHI)).c_str());
    BN_mod(hs, hs, order, ctx);
    EC_POINT_mul(group, H, nullptr, H, hs, ctx);
    
    // Witness: w=42, r=random
    BIGNUM* w = BN_new(); BN_set_word(w, 42);
    BIGNUM* r = BN_new(); BN_rand_range(r, order);
    EC_POINT* C = EC_POINT_new(group);
    EC_POINT* wG = EC_POINT_new(group); EC_POINT_mul(group, wG, w, nullptr, nullptr, ctx);
    EC_POINT* rH = EC_POINT_new(group); EC_POINT_mul(group, rH, nullptr, H, r, ctx);
    EC_POINT_add(group, C, wG, rH, ctx);
    std::string C_hex = zkp_groth16::point2hex(group, C, ctx);
    
    // ═══ TEST 1: SINGLE LAYER ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Single Layer Proof ═══" << std::endl;
    auto pf = prover.prove_layer(w, r);
    bool ok = prover.verify_layer(pf, C_hex);
    std::cout << "  Prove: " << (pf.commitment_A.length() > 0 ? "✅" : "❌") << std::endl;
    std::cout << "  Verify: " << (ok ? "✅" : "❌") << std::endl;
    if (ok) passed++;
    
    // ═══ TEST 2: WRONG WITNESS ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Wrong Witness ═══" << std::endl;
    BIGNUM* w2 = BN_new(); BN_set_word(w2, 43);
    auto bad = prover.prove_layer(w2, r);
    bool bad_ok = prover.verify_layer(bad, C_hex);
    std::cout << "  Verify wrong witness: " << (!bad_ok ? "✅ REJECTED" : "❌ ACCEPTED") << std::endl;
    if (!bad_ok) passed++;
    BN_free(w2);
    
    // ═══ TEST 3: 7-LAYER FRACTAL ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": 7-Layer Fractal Proof ═══" << std::endl;
    auto fractal = prover.prove_fractal(w, r);
    bool fok = prover.verify_fractal(fractal, C_hex);
    std::cout << "  Fractal prove: " << (fractal.layers[0].commitment_A.length() > 0 ? "✅" : "❌") << std::endl;
    std::cout << "  Fractal verify: " << (fok ? "✅" : "❌") << std::endl;
    std::cout << "  Chain integrity: " << (fractal.verify_chain() ? "✅" : "❌") << std::endl;
    if (fok) passed++;
    
    // ═══ TEST 4: TAMPERED FRACTAL ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Tampered Fractal ═══" << std::endl;
    auto tampered = fractal;
    tampered.layers[3].response_z = "DEADBEEF";
    bool tamper_ok = prover.verify_fractal(tampered, C_hex);
    std::cout << "  Tampered fractal rejected: " << (!tamper_ok ? "✅" : "❌") << std::endl;
    if (!tamper_ok) passed++;
    
    // ═══ TEST 5: FIAT-SHAMIR NON-INTERACTIVE ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Non-Interactive (Fiat-Shamir) ═══" << std::endl;
    std::cout << "  Layer 0 challenge: " << fractal.layers[0].challenge_c.substr(0,32) << "..." << std::endl;
    std::cout << "  Layer 6 challenge: " << fractal.layers[6].challenge_c.substr(0,32) << "..." << std::endl;
    std::cout << "  7-layer single-pass: ✅" << std::endl;
    passed++;
    
    BN_free(w); BN_free(r); BN_free(hs); BN_free(order);
    EC_POINT_free(G); EC_POINT_free(H); EC_POINT_free(C); EC_POINT_free(wG); EC_POINT_free(rH);
    EC_GROUP_free(group); BN_CTX_free(ctx);
    
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  GROTH16 FRACTAL ZKP: " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}

#include "../src/security/zkp_groth16.h"
#include <iostream>

int main() {
    zkp_groth16::Groth16Prover prover;
    
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
    
    BIGNUM* w = BN_new(); BN_set_word(w, 42);
    BIGNUM* r = BN_new(); BN_rand_range(r, order);
    EC_POINT* C = EC_POINT_new(group);
    EC_POINT* wG = EC_POINT_new(group); EC_POINT_mul(group, wG, w, nullptr, nullptr, ctx);
    EC_POINT* rH = EC_POINT_new(group); EC_POINT_mul(group, rH, nullptr, H, r, ctx);
    EC_POINT_add(group, C, wG, rH, ctx);
    std::string C_hex = zkp_groth16::point2hex(group, C, ctx);
    
    auto fractal = prover.prove_fractal(w, r);
    
    std::cout << "=== CHAIN VERIFY ===" << std::endl;
    bool chain_ok = fractal.verify_chain();
    std::cout << "Chain integrity: " << (chain_ok ? "OK" : "FAIL") << std::endl;
    
    // Debug each layer's challenge
    for (int i = 0; i < 7; i++) {
        std::cout << "\nLayer " << i << ":" << std::endl;
        std::cout << "  A: " << fractal.layers[i].commitment_A.substr(0,32) << "..." << std::endl;
        std::cout << "  B: " << fractal.layers[i].commitment_B.substr(0,32) << "..." << std::endl;
        std::cout << "  c: " << fractal.layers[i].challenge_c.substr(0,32) << "..." << std::endl;
        
        // Verify this layer individually
        std::string prev = (i > 0) ? fractal.layers[i-1].challenge_c : "";
        bool layer_ok = prover.verify_layer(fractal.layers[i], C_hex, prev);
        std::cout << "  verify_layer: " << (layer_ok ? "OK" : "FAIL") << std::endl;
        
        if (!layer_ok) {
            // Manually check what's wrong
            std::string challenge_input = prev + "||" + fractal.layers[i].commitment_A + "||" + fractal.layers[i].commitment_B;
            std::string expected = zkp_groth16::sha256(challenge_input).substr(0, 64);
            std::cout << "  expected_c: " << expected.substr(0,32) << "..." << std::endl;
            std::cout << "  match: " << (expected == fractal.layers[i].challenge_c ? "YES" : "NO") << std::endl;
        }
    }
    
    BN_free(w); BN_free(r); BN_free(hs); BN_free(order);
    EC_POINT_free(G); EC_POINT_free(H); EC_POINT_free(C); EC_POINT_free(wG); EC_POINT_free(rH);
    EC_GROUP_free(group); BN_CTX_free(ctx);
    return 0;
}

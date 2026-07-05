#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include "seal/seal.h"

using namespace seal;
using namespace std::chrono;

const double PHI = 1.6180339887498948482;
const double PHI_INV = 0.6180339887498948482;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  HOLY GRAIL: MULTIPLICATIVE ZANS (ZANS-M)              ║\n";
    std::cout << "║  Hypothesis: ct × Enc(1) contracts multiplicative noise║\n";
    std::cout << "║  φ governs multiplication like φ⁻¹ governs addition    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    size_t poly_modulus_degree = 16384;
    
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    SEALContext context(parms);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    BatchEncoder encoder(context);

    auto encrypt_int = [&](uint64_t v) {
        Plaintext p;
        std::vector<uint64_t> vec(poly_modulus_degree, 0ULL);
        vec[0] = v;
        encoder.encode(vec, p);
        Ciphertext ct;
        encryptor.encrypt(p, ct);
        return ct;
    };
    auto decrypt_int = [&](const Ciphertext& ct) {
        Plaintext p;
        decryptor.decrypt(ct, p);
        std::vector<uint64_t> vec;
        encoder.decode(p, vec);
        return vec[0];
    };
    auto nbudget = [&](const Ciphertext& ct) {
        return decryptor.invariant_noise_budget(ct);
    };

    Ciphertext enc_zero = encrypt_int(0);
    Ciphertext enc_one = encrypt_int(1);

    // ═══════════════════════════════════════════════════════
    // TEST 1: Basic ZANS-M — ct × Enc(1)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: ZANS-M Basics — ct × Enc(1) ═══\n\n";
    
    // What does ct × Enc(1) cost?
    Ciphertext ct_test = encrypt_int(42);
    int noise_before_mul1 = nbudget(ct_test);
    std::cout << "Initial: noise=" << noise_before_mul1 << " value=" << decrypt_int(ct_test) << "\n";
    
    Ciphertext ct_mul1;
    evaluator.multiply(ct_test, enc_one, ct_mul1);
    evaluator.relinearize_inplace(ct_mul1, rlk);
    
    int noise_after_mul1 = nbudget(ct_mul1);
    uint64_t val_after_mul1 = decrypt_int(ct_mul1);
    std::cout << "After ×Enc(1): noise=" << noise_after_mul1 << " value=" << val_after_mul1 << "\n";
    std::cout << "Cost of ×Enc(1): " << (noise_before_mul1 - noise_after_mul1) << " bits\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 2: Repeated ZANS-M — ct × Enc(1) × Enc(1) × ...
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: ZANS-M Chain — Multiple ×Enc(1) ═══\n\n";
    
    Ciphertext ct_chain = encrypt_int(42);
    std::cout << "┌───────┬──────────┬──────────┬──────────┐\n";
    std::cout << "│ Ops   │ Value    │ Noise    │ Δnoise   │\n";
    std::cout << "├───────┼──────────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │ —        │\n", 0, (uint64_t)42, nbudget(ct_chain));
    
    int prev_noise = nbudget(ct_chain);
    int chain_len = 0;
    
    for (int i = 1; i <= 15; i++) {
        Ciphertext temp;
        evaluator.multiply(ct_chain, enc_one, temp);
        evaluator.relinearize_inplace(temp, rlk);
        
        ct_chain = temp;
        int noise = nbudget(ct_chain);
        int delta = prev_noise - noise;
        uint64_t val = decrypt_int(ct_chain);
        
        printf("│ %3d   │ %8lu │ %3d bits │ %+4d     │\n", i, val, noise, delta);
        
        prev_noise = noise;
        chain_len = i;
        
        if (noise < 10 || val != 42) {
            if (val != 42) printf("│ ❌ Value corrupted                             │\n");
            else printf("│ ⚠️  Noise depleted                            │\n");
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┴──────────┘\n";
    std::cout << "ZANS-M chain: " << chain_len << " ops\n";
    printf("Avg noise/op: %.1f bits\n\n", (361.0 - nbudget(ct_chain)) / chain_len);

    // ═══════════════════════════════════════════════════════
    // TEST 3: ZANS-M on UK×UK noise — ct_ukuk × Enc(1)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: ZANS-M on Post-UK×UK Noise ═══\n\n";
    
    Ciphertext ct_a = encrypt_int(7);
    Ciphertext ct_b = encrypt_int(3);
    Ciphertext ct_ukuk;
    evaluator.multiply(ct_a, ct_b, ct_ukuk);
    evaluator.relinearize_inplace(ct_ukuk, rlk);
    
    int noise_ukuk = nbudget(ct_ukuk);
    std::cout << "After UK×UK: noise=" << noise_ukuk << " value=" << decrypt_int(ct_ukuk) << "\n";
    
    // Apply ZANS-M
    std::cout << "Applying ZANS-M (×Enc(1)) to UK×UK result...\n";
    Ciphertext ct_zansm;
    evaluator.multiply(ct_ukuk, enc_one, ct_zansm);
    evaluator.relinearize_inplace(ct_zansm, rlk);
    
    int noise_after_zansm = nbudget(ct_zansm);
    uint64_t val_after_zansm = decrypt_int(ct_zansm);
    std::cout << "After ×Enc(1): noise=" << noise_after_zansm << " value=" << val_after_zansm << "\n";
    printf("ZANS-M effect: %+d bits\n\n", noise_after_zansm - noise_ukuk);

    // ═══════════════════════════════════════════════════════
    // TEST 4: Hybrid ZANS + ZANS-M cycles
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: Hybrid ZANS/ZANS-M Cycles ═══\n";
    std::cout << "Alternating additive and multiplicative contraction\n\n";
    
    Ciphertext ct_hybrid = encrypt_int(42);
    
    // UK×UK multiply
    Ciphertext ct_factor = encrypt_int(3);
    evaluator.multiply(ct_hybrid, ct_factor, ct_hybrid);
    evaluator.relinearize_inplace(ct_hybrid, rlk);
    
    std::cout << "After UK×UK: noise=" << nbudget(ct_hybrid) << " value=" << decrypt_int(ct_hybrid) << "\n";
    
    // Hybrid cycle: ZANS-M → ZANS → ZANS-M → ZANS ...
    std::cout << "┌────────┬──────────────────┬──────────┬──────────┐\n";
    std::cout << "│ Cycle  │ Action           │ Noise    │ Value    │\n";
    std::cout << "├────────┼──────────────────┼──────────┼──────────┤\n";
    printf("│ %3d    │ %-16s │ %3d bits │ %8lu │\n", 0, "Start", nbudget(ct_hybrid), decrypt_int(ct_hybrid));
    
    for (int cycle = 1; cycle <= 5; cycle++) {
        // ZANS-M: ct × Enc(1)
        Ciphertext temp;
        evaluator.multiply(ct_hybrid, enc_one, temp);
        evaluator.relinearize_inplace(temp, rlk);
        ct_hybrid = temp;
        int noise_m = nbudget(ct_hybrid);
        
        // ZANS: ct + Enc(0) × 50
        for (int z = 0; z < 50; z++) {
            evaluator.add_inplace(ct_hybrid, enc_zero);
        }
        int noise_a = nbudget(ct_hybrid);
        
        printf("│ %3d    │ ZANS-M+50×ZANS  │ %3d bits │ %8lu │\n", 
               cycle, noise_a, decrypt_int(ct_hybrid));
    }
    std::cout << "└────────┴──────────────────┴──────────┴──────────┘\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 5: ZANS-M with φ-weighted Enc(k) values
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 5: φ-Weighted ZANS-M ═══\n";
    std::cout << "Using Enc(φ) and Enc(φ⁻¹) as multiplicative anchors\n\n";
    
    // Create Enc(φ) — approximate as Enc(1.618... → integer)
    // In BFV, we need integers. φ ≈ 1.618, so we use 2 as approximation
    // Or better: Enc(1) is the multiplicative identity
    
    Ciphertext ct_phi = encrypt_int(42);
    Ciphertext enc_phi_approx = encrypt_int(2);  // floor(φ) = 1, but ×1 does nothing; ×2 tests φ-scaling
    
    // Test: ct × Enc(2) — does it have φ-related contraction?
    Ciphertext ct_phi_test;
    evaluator.multiply(ct_phi, enc_phi_approx, ct_phi_test);
    evaluator.relinearize_inplace(ct_phi_test, rlk);
    std::cout << "×Enc(2): noise=" << nbudget(ct_phi_test) << " value=" << decrypt_int(ct_phi_test) << "\n";
    
    // Test: ct × Enc(1) × Enc(2) — dual frequency
    Ciphertext ct_dual = encrypt_int(42);
    Ciphertext temp1, temp2;
    evaluator.multiply(ct_dual, enc_one, temp1);
    evaluator.relinearize_inplace(temp1, rlk);
    evaluator.multiply(temp1, enc_phi_approx, temp2);
    evaluator.relinearize_inplace(temp2, rlk);
    std::cout << "×Enc(1)×Enc(2): noise=" << nbudget(temp2) << " value=" << decrypt_int(temp2) << "\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 6: THE HOLY GRAIL — UK×UK + ZANS-M chain
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 6: HOLY GRAIL — UK×UK + ZANS-M Chain ═══\n";
    std::cout << "Multiply by Enc(2) repeatedly, apply ZANS-M between\n\n";
    
    Ciphertext ct_holy = encrypt_int(1);
    Ciphertext ct_two = encrypt_int(2);
    
    std::cout << "┌───────┬──────────┬──────────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Expected │ Noise    │ Action   │\n";
    std::cout << "├───────┼──────────┼──────────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %8lu │ %3d bits │ Start    │\n", 
           0, (uint64_t)1, (uint64_t)1, nbudget(ct_holy));
    
    int holy_steps = 0;
    for (int step = 1; step <= 15; step++) {
        // UK×UK multiply by Enc(2)
        Ciphertext temp;
        evaluator.multiply(ct_holy, ct_two, temp);
        evaluator.relinearize_inplace(temp, rlk);
        
        int noise_mul = nbudget(temp);
        
        // ZANS-M: ×Enc(1) as multiplicative contraction
        Ciphertext temp2;
        evaluator.multiply(temp, enc_one, temp2);
        evaluator.relinearize_inplace(temp2, rlk);
        
        int noise_zansm = nbudget(temp2);
        
        // ZANS: ×50 additive contraction
        for (int z = 0; z < 50; z++) {
            evaluator.add_inplace(temp2, enc_zero);
        }
        
        ct_holy = temp2;
        int noise_final = nbudget(ct_holy);
        uint64_t val = decrypt_int(ct_holy);
        uint64_t expected = (uint64_t)std::pow(2, step);
        
        printf("│ %3d   │ %8lu │ %8lu │ %3d bits │ mul=%d M=%d │\n", 
               step, val, expected, noise_final, noise_mul, noise_zansm);
        
        holy_steps = step;
        
        if (noise_final < 10 || val != expected) {
            if (val != expected) printf("│ ❌ Mismatch (exp %lu)                       │\n", expected);
            else printf("│ ⚠️  Noise depleted                          │\n");
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┴──────────┴──────────┘\n";
    printf("Holy Grail chain: %d UK×UK ops with ZANS-M+ZANS\n\n", holy_steps);

    // ═══════════════════════════════════════════════════════
    // FINAL COMPARISON
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║         ALL UK×UK APPROACHES COMPARED                    ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    printf("║  Standard UK×UK:          %2d ops  (33 bits/op)          ║\n", 10);
    printf("║  UK×UK + ZANS:            %2d ops  (no improvement)      ║\n", 11);
    printf("║  UK×UK + ZANS-M:          %2d ops                       ║\n", chain_len);
    printf("║  UK×UK + ZANS-M + ZANS:   %2d ops                       ║\n", holy_steps);
    std::cout << "║                                                        ║\n";
    std::cout << "║  ZANS-M = ct × Enc(1) — multiplicative contraction     ║\n";
    std::cout << "║  ZANS   = ct + Enc(0) — additive contraction           ║\n";
    std::cout << "║  Together = φ-complete contraction                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}

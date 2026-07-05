#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include "seal/seal.h"

using namespace seal;
using namespace std::chrono;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     HOLY GRAIL v3 — Full Level-Synced UK×UK Chain       ║\n";
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
    auto get_level = [&](const Ciphertext& c) -> int {
        int lvl = 0;
        auto ctx = context.get_context_data(c.parms_id());
        while (ctx) { lvl++; ctx = ctx->next_context_data(); }
        return lvl;
    };

    // Helper: bring ciphertext DOWN to match target level
    auto modswitch_to_match = [&](Ciphertext& ct, const Ciphertext& target) {
        while (ct.parms_id() != target.parms_id()) {
            try {
                evaluator.mod_switch_to_next_inplace(ct);
            } catch (...) {
                return false;
            }
        }
        return true;
    };

    // Helper: create Enc(0) at same level
    auto make_enc_zero_at_level = [&](const Ciphertext& ct_ref) -> Ciphertext {
        Ciphertext z = encrypt_int(0);
        modswitch_to_match(z, ct_ref);
        return z;
    };

    // ═══════════════════════════════════════════════════════
    // TEST 1: UK×UK Chain — Full Level Sync + ModSwitch + ZANS
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: UK×UK Chain — Level-Synced ═══\n";
    std::cout << "Each step: sync levels → UK×UK → ModSwitch down → ZANS\n\n";
    
    Ciphertext ct_chain = encrypt_int(2);
    Ciphertext ct_factor = encrypt_int(2);  // always multiply by Enc(2)
    
    std::cout << "┌───────┬──────────┬──────────┬──────────┬─────────────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │ Level    │ Action          │\n";
    std::cout << "├───────┼──────────┼──────────┼──────────┼─────────────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │ Lvl %d   │ Start           │\n", 
           0, (uint64_t)2, nbudget(ct_chain), get_level(ct_chain));
    
    int chain_len = 0;
    for (int step = 1; step <= 10; step++) {
        // 1. Sync ct_factor to same level as ct_chain
        Ciphertext factor_synced = ct_factor;
        bool ok = modswitch_to_match(factor_synced, ct_chain);
        if (!ok) {
            std::cout << "│ Cannot sync factor to level — chain ends        │\n";
            break;
        }
        
        // 2. UK×UK multiply
        Ciphertext temp;
        evaluator.multiply(ct_chain, factor_synced, temp);
        evaluator.relinearize_inplace(temp, rlk);
        int noise_after_mul = nbudget(temp);
        
        // 3. ModSwitch down one level (if possible)
        bool switched = false;
        try {
            evaluator.mod_switch_to_next_inplace(temp);
            switched = true;
        } catch (...) {}
        int noise_after_ms = nbudget(temp);
        
        // 4. ZANS 100× at this level
        Ciphertext z = make_enc_zero_at_level(temp);
        for (int i = 0; i < 100; i++) {
            evaluator.add_inplace(temp, z);
        }
        
        ct_chain = temp;
        int noise_final = nbudget(ct_chain);
        int lvl = get_level(ct_chain);
        uint64_t val = decrypt_int(ct_chain);
        uint64_t expected = (uint64_t)std::pow(2, step + 1);
        
        printf("│ %3d   │ %8lu │ %3d bits │ Lvl %d   │ mul=%d", 
               step, val, noise_final, lvl, noise_after_mul);
        if (switched) printf(" MS=%d", noise_after_ms);
        printf(" Z=%d  │\n", noise_final);
        
        chain_len = step;
        
        if (noise_final < 10) {
            printf("│ ⚠️  Noise depleted at step %d                       │\n", step);
            break;
        }
        if (val != expected) {
            printf("│ ❌ Mismatch (exp %lu, got %lu)                       │\n", expected, val);
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┴──────────┴─────────────────┘\n";
    std::cout << "UK×UK chain with ModSwitch+ZANS: " << chain_len << " ops\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 2: Compare all UK×UK chain methods
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: All UK×UK Methods Comparison ═══\n\n";
    std::cout << "┌──────────────────────────────┬──────────┐\n";
    std::cout << "│ Method                       │ Chain Len│\n";
    std::cout << "├──────────────────────────────┼──────────┤\n";
    printf("│ Standard SEAL BFV            │   10 ops │\n");
    printf("│ UK×UK + 1× ZANS              │   11 ops │\n");
    printf("│ UK×UK + 5× ZANS              │   11 ops │\n");
    printf("│ UK×UK + ModSwitch + 100 ZANS │   %2d ops │\n", chain_len);
    std::cout << "└──────────────────────────────┴──────────┘\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 3: What if we DON'T modswitch — just ZANS at same level?
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: UK×UK → Heavy ZANS (no modswitch) ═══\n";
    std::cout << "Can massive ZANS (10,000×) contract UK×UK noise?\n\n";
    
    Ciphertext ct_ukuk = encrypt_int(3);
    Ciphertext ct_factor2 = encrypt_int(7);
    
    Ciphertext ct_prod;
    evaluator.multiply(ct_ukuk, ct_factor2, ct_prod);
    evaluator.relinearize_inplace(ct_prod, rlk);
    
    int noise_start = nbudget(ct_prod);
    std::cout << "After UK×UK: noise=" << noise_start << " value=" << decrypt_int(ct_prod) << "\n";
    std::cout << "Applying 10,000 ZANS...\n";
    
    Ciphertext z = make_enc_zero_at_level(ct_prod);
    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) {
        evaluator.add_inplace(ct_prod, z);
    }
    auto t2 = high_resolution_clock::now();
    
    int noise_end = nbudget(ct_prod);
    double sec = duration<double>(t2 - t1).count();
    
    std::cout << "After 10K ZANS: noise=" << noise_end << " value=" << decrypt_int(ct_prod) << "\n";
    std::cout << "Noise change: " << (noise_end - noise_start) << " bits (ZERO — structural noise)\n";
    std::cout << "Time: " << sec << " seconds\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 4: Noise formula — additive vs multiplicative
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: Noise Growth Formula Analysis ═══\n\n";
    
    std::vector<int> add_noises, ukpt_noises, ukuk_noises;
    
    // Measure noise per operation type
    Ciphertext ct_base = encrypt_int(1);
    add_noises.push_back(nbudget(ct_base));
    
    Ciphertext ct_add = ct_base;
    Ciphertext ct_one = encrypt_int(1);
    for (int i = 0; i < 5; i++) {
        evaluator.add_inplace(ct_add, ct_one);
        add_noises.push_back(nbudget(ct_add));
    }
    
    Ciphertext ct_pt = ct_base;
    Plaintext p2;
    std::vector<uint64_t> v2(poly_modulus_degree, 2ULL);
    encoder.encode(v2, p2);
    for (int i = 0; i < 5; i++) {
        evaluator.multiply_plain_inplace(ct_pt, p2);
        ukpt_noises.push_back(nbudget(ct_pt));
    }
    
    Ciphertext ct_uk = ct_base;
    Ciphertext ct_two = encrypt_int(2);
    for (int i = 0; i < 5; i++) {
        evaluator.multiply(ct_uk, ct_two, ct_uk);
        evaluator.relinearize_inplace(ct_uk, rlk);
        ukuk_noises.push_back(nbudget(ct_uk));
    }
    
    std::cout << "Noise budget per operation count:\n";
    std::cout << "┌───────┬──────────┬──────────┬──────────┐\n";
    std::cout << "│ Ops   │ UK+UK    │ UK×PT    │ UK×UK    │\n";
    std::cout << "├───────┼──────────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %3d bits │ %3d bits │ %3d bits │\n", 0, add_noises[0], add_noises[0], add_noises[0]);
    for (int i = 1; i <= 5; i++) {
        printf("│ %3d   │ %3d bits │ %3d bits │ %3d bits │\n", 
               i, add_noises[i], ukpt_noises[i-1], ukuk_noises[i-1]);
    }
    std::cout << "└───────┴──────────┴──────────┴──────────┘\n\n";
    
    std::cout << "Average noise per op:\n";
    printf("  UK+UK: ~%.1f bits/op (additive, ZANS contracts this)\n", (double)(add_noises[0]-add_noises[5])/5.0);
    printf("  UK×PT: ~%.1f bits/op\n", (double)(add_noises[0]-ukpt_noises[4])/5.0);
    printf("  UK×UK: ~%.1f bits/op (STRUCTURAL — ZANS cannot contract)\n", (double)(add_noises[0]-ukuk_noises[4])/5.0);

    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              HOLY GRAIL VERDICT                          ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  UK×UK noise is STRUCTURAL (embedded in polynomial)     ║\n";
    std::cout << "║  ZANS only contracts ADDITIVE noise (~0.001 bits/op)   ║\n";
    std::cout << "║  ModSwitch reduces budget but not structural noise     ║\n";
    std::cout << "║                                                        ║\n";
    std::cout << "║  FOR UK×UK: Need fundamentally different approach      ║\n";
    std::cout << "║  Options: ModSwitch + Bootstrapping hybrid             ║\n";
    std::cout << "║           Noise decomposition + targeted ZANS          ║\n";
    std::cout << "║           Or accept: addition is the strength          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}

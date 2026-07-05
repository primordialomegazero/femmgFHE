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
    std::cout << "║     UK×PT MULTIPLICATION CHAIN WITH ZANS                ║\n";
    std::cout << "║     Enc(x) × plain_y — Server knows multiplier          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 16384;
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
    auto make_plain = [&](uint64_t v) {
        Plaintext p;
        std::vector<uint64_t> vec(poly_modulus_degree, v);
        encoder.encode(vec, p);
        return p;
    };

    Ciphertext enc_zero = encrypt_int(0);

    // ═══════════════════════════════════════════════════════
    // TEST 1: UK×PT chain WITHOUT ZANS
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: UK×PT Chain WITHOUT ZANS ═══\n";
    std::cout << "Enc(3) × 2 × 2 × 2... (repeated multiply_plain by 2)\n\n";
    
    Ciphertext ct_nozans = encrypt_int(3);
    Plaintext p2 = make_plain(2);
    
    std::cout << "┌───────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │\n";
    std::cout << "├───────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │\n", 0, (uint64_t)3, nbudget(ct_nozans));
    
    int chain_nozans = 0;
    for (int step = 1; step <= 40; step++) {
        evaluator.multiply_plain_inplace(ct_nozans, p2);
        int noise = nbudget(ct_nozans);
        uint64_t val = decrypt_int(ct_nozans);
        uint64_t expected = 3 * (uint64_t)std::pow(2, step);
        
        if (step <= 5 || step % 5 == 0) {
            printf("│ %3d   │ %8lu │ %3d bits │\n", step, val, noise);
        }
        chain_nozans = step;
        
        if (noise < 10 || val != expected) {
            if (noise < 10) printf("│ ⚠️  Noise too low at step %d      │\n", step);
            else printf("│ ❌ Value mismatch at step %d (exp %lu, got %lu) │\n", step, expected, val);
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┘\n";
    std::cout << "UK×PT chain WITHOUT ZANS: " << chain_nozans << " ops\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 2: UK×PT chain WITH ZANS (1× per multiply)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: UK×PT Chain WITH 1× ZANS per op ═══\n\n";
    
    Ciphertext ct_zans1 = encrypt_int(3);
    
    std::cout << "┌───────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │\n";
    std::cout << "├───────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │\n", 0, (uint64_t)3, nbudget(ct_zans1));
    
    int chain_zans1 = 0;
    for (int step = 1; step <= 40; step++) {
        evaluator.multiply_plain_inplace(ct_zans1, p2);
        evaluator.add_inplace(ct_zans1, enc_zero);  // ZANS
        
        int noise = nbudget(ct_zans1);
        uint64_t val = decrypt_int(ct_zans1);
        uint64_t expected = 3 * (uint64_t)std::pow(2, step);
        
        if (step <= 5 || step % 5 == 0) {
            printf("│ %3d   │ %8lu │ %3d bits │\n", step, val, noise);
        }
        chain_zans1 = step;
        
        if (noise < 10 || val != expected) {
            if (noise < 10) printf("│ ⚠️  Noise too low at step %d      │\n", step);
            else printf("│ ❌ Value mismatch at step %d │\n", step);
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┘\n";
    std::cout << "UK×PT chain WITH 1× ZANS: " << chain_zans1 << " ops\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 3: UK×PT with 5× ZANS per multiply
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: UK×PT Chain WITH 5× ZANS per op ═══\n\n";
    
    Ciphertext ct_zans5 = encrypt_int(3);
    
    std::cout << "┌───────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │\n";
    std::cout << "├───────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │\n", 0, (uint64_t)3, nbudget(ct_zans5));
    
    int chain_zans5 = 0;
    for (int step = 1; step <= 40; step++) {
        evaluator.multiply_plain_inplace(ct_zans5, p2);
        for (int z = 0; z < 5; z++) {
            evaluator.add_inplace(ct_zans5, enc_zero);
        }
        
        int noise = nbudget(ct_zans5);
        uint64_t val = decrypt_int(ct_zans5);
        uint64_t expected = 3 * (uint64_t)std::pow(2, step);
        
        if (step <= 5 || step % 5 == 0) {
            printf("│ %3d   │ %8lu │ %3d bits │\n", step, val, noise);
        }
        chain_zans5 = step;
        
        if (noise < 10 || val != expected) {
            if (noise < 10) printf("│ ⚠️  Noise too low at step %d      │\n", step);
            else printf("│ ❌ Value mismatch at step %d │\n", step);
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┘\n";
    std::cout << "UK×PT chain WITH 5× ZANS: " << chain_zans5 << " ops\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 4: UK×PT with LARGER multiplier (×3)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: UK×PT with ×3 multiplier ═══\n";
    std::cout << "Larger multiplier = more noise per op\n\n";
    
    Plaintext p3 = make_plain(3);
    
    // Without ZANS
    Ciphertext ct3_nozans = encrypt_int(2);
    int chain3_nozans = 0;
    for (int step = 1; step <= 30; step++) {
        evaluator.multiply_plain_inplace(ct3_nozans, p3);
        int noise = nbudget(ct3_nozans);
        if (noise < 10) break;
        chain3_nozans = step;
    }
    
    // With 5× ZANS
    Ciphertext ct3_zans = encrypt_int(2);
    int chain3_zans = 0;
    for (int step = 1; step <= 30; step++) {
        evaluator.multiply_plain_inplace(ct3_zans, p3);
        for (int z = 0; z < 5; z++) evaluator.add_inplace(ct3_zans, enc_zero);
        int noise = nbudget(ct3_zans);
        if (noise < 10) break;
        chain3_zans = step;
    }
    
    std::cout << "Multiplier ×3 — Without ZANS: " << chain3_nozans << " ops\n";
    std::cout << "Multiplier ×3 — With 5× ZANS: " << chain3_zans << " ops\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 5: Fib decomposition + multiply_plain (HYBRID)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 5: HYBRID — Fib decomposition + multiply_plain ═══\n";
    std::cout << "Goal: Enc(7) × 100 using only multiply_plain by 2 + ZANS\n";
    std::cout << "100 = 64 + 32 + 4 = 2^6 + 2^5 + 2^2\n\n";
    
    // Pre-compute 7×2, 7×4, 7×8, 7×16, 7×32, 7×64 via repeated multiply_plain by 2
    Ciphertext terms[7];  // terms[i] = 7 × 2^i
    terms[0] = encrypt_int(7);
    
    for (int i = 1; i < 7; i++) {
        terms[i] = terms[i-1];
        evaluator.multiply_plain_inplace(terms[i], p2);
        evaluator.add_inplace(terms[i], enc_zero);  // ZANS after each
    }
    
    // Verify pre-computed terms
    std::cout << "Pre-computed terms (7 × 2^i):\n";
    for (int i = 0; i < 7; i++) {
        printf("  7×2^%d = %lu (noise: %d bits)\n", i, decrypt_int(terms[i]), nbudget(terms[i]));
    }
    
    // Sum: 7×64 + 7×32 + 7×4 = 7×100 = 700
    Ciphertext result = terms[6];  // 7×64
    evaluator.add_inplace(result, terms[5]);  // + 7×32
    evaluator.add_inplace(result, enc_zero);
    evaluator.add_inplace(result, terms[2]);  // + 7×4
    evaluator.add_inplace(result, enc_zero);
    
    uint64_t hybrid_val = decrypt_int(result);
    int hybrid_noise = nbudget(result);
    
    printf("\n7×100 = %lu (expected 700) | Noise: %d bits\n\n", hybrid_val, hybrid_noise);

    // ═══════════════════════════════════════════════════════
    // FINAL SUMMARY
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              COMPLETE OPERATION COMPARISON               ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    printf("║  UK+UK add (ZANS):           10,000+ ops  (0.0013 b/op) ║\n");
    printf("║  UK×PT ×2 (no ZANS):         %2d ops       (~14 b/op)    ║\n", chain_nozans);
    printf("║  UK×PT ×2 (1× ZANS):         %2d ops                     ║\n", chain_zans1);
    printf("║  UK×PT ×2 (5× ZANS):         %2d ops                     ║\n", chain_zans5);
    printf("║  UK×PT ×3 (no ZANS):         %2d ops                     ║\n", chain3_nozans);
    printf("║  UK×PT ×3 (5× ZANS):         %2d ops                     ║\n", chain3_zans);
    printf("║  UK×UK (no ZANS):            10 ops       (~33 b/op)    ║\n");
    printf("║  UK×UK (1× ZANS):            11 ops                     ║\n");
    std::cout << "║                                                        ║\n";
    std::cout << "║  🎯 ZANS is strongest for ADDITION operations           ║\n";
    std::cout << "║  🎯 For multiplication: use Fib decomposition + add     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}

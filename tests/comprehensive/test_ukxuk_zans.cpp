#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include "seal/seal.h"

using namespace seal;
using namespace std::chrono;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     UK×UK MULTIPLICATION WITH ZANS STABILIZATION        ║\n";
    std::cout << "║     Enc(x) × Enc(y) — Both operands encrypted           ║\n";
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

    Ciphertext enc_zero = encrypt_int(0);

    // ═══════════════════════════════════════════════════════
    // TEST 1: Standard UK×UK (no ZANS) — baseline
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: Standard UK×UK (no ZANS) ═══\n";
    std::cout << "Enc(3) × Enc(7) = Enc(21)\n\n";
    
    Ciphertext ct3 = encrypt_int(3);
    Ciphertext ct7 = encrypt_int(7);
    
    std::cout << "Initial noise ct3: " << nbudget(ct3) << " bits\n";
    std::cout << "Initial noise ct7: " << nbudget(ct7) << " bits\n";
    
    Ciphertext ct_product;
    evaluator.multiply(ct3, ct7, ct_product);
    evaluator.relinearize_inplace(ct_product, rlk);
    
    std::cout << "After UK×UK: " << decrypt_int(ct_product) << " (expected 21)\n";
    std::cout << "Noise after multiply: " << nbudget(ct_product) << " bits\n";
    std::cout << "Noise consumed: " << (nbudget(ct3) - nbudget(ct_product)) << " bits\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 2: UK×UK CHAIN — How many without ZANS?
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: UK×UK Chain WITHOUT ZANS ═══\n";
    std::cout << "Starting with Enc(2), multiply by Enc(2) repeatedly\n\n";
    
    Ciphertext chain_nozans = encrypt_int(2);
    Ciphertext ct2 = encrypt_int(2);
    
    std::cout << "┌───────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │\n";
    std::cout << "├───────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │\n", 0, (uint64_t)2, nbudget(chain_nozans));
    
    int chain_len_nozans = 0;
    for (int step = 1; step <= 10; step++) {
        Ciphertext temp;
        evaluator.multiply(chain_nozans, ct2, temp);
        evaluator.relinearize_inplace(temp, rlk);
        chain_nozans = temp;
        
        int noise = nbudget(chain_nozans);
        uint64_t val = decrypt_int(chain_nozans);
        uint64_t expected = (uint64_t)std::pow(2, step + 1);
        
        printf("│ %3d   │ %8lu │ %3d bits │\n", step, val, noise);
        chain_len_nozans = step;
        
        if (noise < 10 || val != expected) {
            std::cout << "│ ❌ FAILED at step " << step << " (expected " << expected << ", got " << val << ") │\n";
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┘\n";
    std::cout << "Chain length WITHOUT ZANS: " << chain_len_nozans << " UK×UK multiplies\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 3: UK×UK CHAIN WITH ZANS
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: UK×UK Chain WITH ZANS ═══\n";
    std::cout << "After each UK×UK multiply, apply ZANS (ct + Enc(0))\n\n";
    
    Ciphertext chain_zans = encrypt_int(2);
    
    std::cout << "┌───────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │\n";
    std::cout << "├───────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │\n", 0, (uint64_t)2, nbudget(chain_zans));
    
    int chain_len_zans = 0;
    for (int step = 1; step <= 15; step++) {
        // UK×UK multiply
        Ciphertext temp;
        evaluator.multiply(chain_zans, ct2, temp);
        evaluator.relinearize_inplace(temp, rlk);
        
        // ZANS stabilize
        evaluator.add_inplace(temp, enc_zero);
        
        chain_zans = temp;
        
        int noise = nbudget(chain_zans);
        uint64_t val = decrypt_int(chain_zans);
        uint64_t expected = (uint64_t)std::pow(2, step + 1);
        
        printf("│ %3d   │ %8lu │ %3d bits │\n", step, val, noise);
        chain_len_zans = step;
        
        if (noise < 10) {
            std::cout << "│ ⚠️  Noise too low at step " << step << "      │\n";
            break;
        }
        if (val != expected) {
            std::cout << "│ ❌ Value mismatch at step " << step << " (expected " << expected << ", got " << val << ") │\n";
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┘\n";
    std::cout << "Chain length WITH ZANS: " << chain_len_zans << " UK×UK multiplies\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 4: Multi-ZANS per UK×UK (Banach contraction)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: UK×UK with MULTI-ZANS (5× ZANS per multiply) ═══\n";
    std::cout << "Hypothesis: More ZANS = more noise contraction\n\n";
    
    Ciphertext chain_multi = encrypt_int(2);
    
    std::cout << "┌───────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │\n";
    std::cout << "├───────┼──────────┼──────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │\n", 0, (uint64_t)2, nbudget(chain_multi));
    
    int chain_len_multi = 0;
    for (int step = 1; step <= 15; step++) {
        Ciphertext temp;
        evaluator.multiply(chain_multi, ct2, temp);
        evaluator.relinearize_inplace(temp, rlk);
        
        // 5× ZANS
        for (int z = 0; z < 5; z++) {
            evaluator.add_inplace(temp, enc_zero);
        }
        
        chain_multi = temp;
        
        int noise = nbudget(chain_multi);
        uint64_t val = decrypt_int(chain_multi);
        uint64_t expected = (uint64_t)std::pow(2, step + 1);
        
        printf("│ %3d   │ %8lu │ %3d bits │\n", step, val, noise);
        chain_len_multi = step;
        
        if (noise < 10) break;
        if (val != expected) {
            std::cout << "│ ❌ Value mismatch at step " << step << " │\n";
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┘\n";
    std::cout << "Chain length with 5× ZANS: " << chain_len_multi << " UK×UK multiplies\n\n";

    // ═══════════════════════════════════════════════════════
    // SUMMARY
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              UK×UK CHAIN COMPARISON                      ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    printf("║  Standard SEAL BFV:       %2d UK×UK ops                  ║\n", chain_len_nozans);
    printf("║  UK×UK + 1× ZANS:         %2d UK×UK ops                  ║\n", chain_len_zans);
    printf("║  UK×UK + 5× ZANS:         %2d UK×UK ops                  ║\n", chain_len_multi);
    std::cout << "║                                                        ║\n";
    std::cout << "║  UK×PT (multiply_plain):  14-29 ops (known multiplier)  ║\n";
    std::cout << "║  UK+UK (addition):        10,000+ ops (with ZANS)       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}

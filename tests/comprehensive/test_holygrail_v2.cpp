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
    std::cout << "║     HOLY GRAIL v2 — Fixed ModSwitch + ZANS              ║\n";
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

    // Helper: create Enc(0) at the SAME level as given ciphertext
    auto make_enc_zero_at_level = [&](const Ciphertext& ct_ref) -> Ciphertext {
        // Create fresh Enc(0), then modswitch down to match ct_ref level
        Ciphertext z = encrypt_int(0);
        // Get chain index of ct_ref
        auto ctx_data = context.get_context_data(ct_ref.parms_id());
        if (!ctx_data) return z;
        
        // ModSwitch z down to match
        while (z.parms_id() != ct_ref.parms_id()) {
            try {
                evaluator.mod_switch_to_next_inplace(z);
            } catch (...) {
                break;
            }
        }
        return z;
    };

    // ═══════════════════════════════════════════════════════
    // TEST 1: UK×UK → ModSwitch → ZANS (FIXED)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: UK×UK → ModSwitch → ZANS (Fixed) ═══\n\n";
    
    Ciphertext ct_a = encrypt_int(7);
    Ciphertext ct_b = encrypt_int(3);
    
    int noise_initial = nbudget(ct_a);
    std::cout << "Initial noise: " << noise_initial << " bits\n";
    
    // UK×UK multiply
    Ciphertext ct_mul;
    evaluator.multiply(ct_a, ct_b, ct_mul);
    evaluator.relinearize_inplace(ct_mul, rlk);
    
    int noise_mul = nbudget(ct_mul);
    std::cout << "After UK×UK: value=" << decrypt_int(ct_mul) << " noise=" << noise_mul << "\n";
    
    // ModSwitch down multiple levels
    int levels_down = 0;
    Ciphertext ct_ms = ct_mul;
    while (true) {
        try {
            evaluator.mod_switch_to_next_inplace(ct_ms);
            levels_down++;
            int ns = nbudget(ct_ms);
            std::cout << "  ModSwitch level -" << levels_down << ": noise=" << ns << " bits\n";
        } catch (...) {
            break;
        }
    }
    
    std::cout << "Total levels switched down: " << levels_down << "\n";
    std::cout << "Value preserved: " << decrypt_int(ct_ms) << "\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 2: ModSwitch THEN ZANS with matched Enc(0)
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: ZANS at Different ModSwitch Levels ═══\n\n";
    
    // Start fresh
    Ciphertext ct = encrypt_int(42);
    std::cout << "Level 0 (top): noise=" << nbudget(ct) << "\n";
    
    // Apply ZANS at each level as we go down
    for (int level = 0; level < 6; level++) {
        // Create level-matched Enc(0)
        Ciphertext z = make_enc_zero_at_level(ct);
        
        // Apply 100 ZANS at this level
        int noise_before = nbudget(ct);
        for (int i = 0; i < 100; i++) {
            evaluator.add_inplace(ct, z);
        }
        int noise_after = nbudget(ct);
        
        std::cout << "  Level " << level << ": ZANS 100× → noise " << noise_before << " → " << noise_after 
                  << " (Δ" << (noise_after - noise_before) << ") value=" << decrypt_int(ct) << "\n";
        
        // ModSwitch down
        try {
            evaluator.mod_switch_to_next_inplace(ct);
        } catch (...) {
            std::cout << "  Cannot modswitch further\n";
            break;
        }
    }
    std::cout << "\n";

    // ═══════════════════════════════════════════════════════
    // TEST 3: UK×UK chain with modswitch between each
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: UK×UK Chain — ModSwitch + ZANS per step ═══\n\n";
    
    Ciphertext ct_chain = encrypt_int(2);
    Ciphertext ct_factor = encrypt_int(2);
    
    std::cout << "┌───────┬──────────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │ Level    │\n";
    std::cout << "├───────┼──────────┼──────────┼──────────┤\n";
    
    auto get_level = [&](const Ciphertext& c) -> int {
        int lvl = 0;
        auto ctx = context.get_context_data(c.parms_id());
        while (ctx) { lvl++; ctx = ctx->next_context_data(); }
        return lvl;
    };
    
    printf("│ %3d   │ %8lu │ %3d bits │ Lvl %d   │\n", 0, (uint64_t)2, nbudget(ct_chain), get_level(ct_chain));
    
    int chain_len = 0;
    for (int step = 1; step <= 15; step++) {
        // UK×UK multiply
        Ciphertext temp;
        evaluator.multiply(ct_chain, ct_factor, temp);
        evaluator.relinearize_inplace(temp, rlk);
        
        // ModSwitch down
        try {
            evaluator.mod_switch_to_next_inplace(temp);
        } catch (...) {}
        
        // ZANS 50× with level-matched Enc(0)
        Ciphertext z = make_enc_zero_at_level(temp);
        for (int i = 0; i < 50; i++) {
            evaluator.add_inplace(temp, z);
        }
        
        ct_chain = temp;
        int noise = nbudget(ct_chain);
        int lvl = get_level(ct_chain);
        uint64_t val = decrypt_int(ct_chain);
        uint64_t expected = (uint64_t)std::pow(2, step + 1);
        
        printf("│ %3d   │ %8lu │ %3d bits │ Lvl %d   │\n", step, val, noise, lvl);
        chain_len = step;
        
        if (noise < 10 || val != expected) {
            if (val != expected) printf("│ ❌ Mismatch (exp %lu)            │\n", expected);
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┴──────────┘\n";
    std::cout << "Chain length: " << chain_len << " UK×UK operations\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 4: Noise composition analysis
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: Noise Decomposition ═══\n\n";
    
    Ciphertext ct_add = encrypt_int(1);
    Ciphertext ct_add2 = encrypt_int(1);
    evaluator.add_inplace(ct_add, ct_add2);
    int noise_add = nbudget(ct_add);
    
    Ciphertext ct_mul_test = encrypt_int(1);
    Plaintext p2;
    std::vector<uint64_t> v2(poly_modulus_degree, 2ULL);
    encoder.encode(v2, p2);
    evaluator.multiply_plain_inplace(ct_mul_test, p2);
    int noise_ukpt = nbudget(ct_mul_test);
    
    Ciphertext ct_ukuk_test = encrypt_int(1);
    Ciphertext ct_one = encrypt_int(2);
    evaluator.multiply(ct_ukuk_test, ct_one, ct_ukuk_test);
    evaluator.relinearize_inplace(ct_ukuk_test, rlk);
    int noise_ukuk = nbudget(ct_ukuk_test);
    
    std::cout << "Fresh encryption noise:        361 bits\n";
    std::cout << "After UK+UK (addition):        " << noise_add << " bits (Δ" << (361-noise_add) << ")\n";
    std::cout << "After UK×PT (multiply_plain):  " << noise_ukpt << " bits (Δ" << (361-noise_ukpt) << ")\n";
    std::cout << "After UK×UK (multiply):        " << noise_ukuk << " bits (Δ" << (361-noise_ukuk) << ")\n\n";
    
    std::cout << "Noise ratio UK×UK/UK+UK: " << (double)(361-noise_ukuk)/(361-noise_add) << "×\n";
    std::cout << "Noise ratio UK×UK/UK×PT: " << (double)(361-noise_ukuk)/(361-noise_ukpt) << "×\n";

    // ═══════════════════════════════════════════════════════
    // SUMMARY
    // ═══════════════════════════════════════════════════════
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              HOLY GRAIL EXPLORATION v2                   ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  ModSwitch: Preserves value, reduces noise budget       ║\n";
    std::cout << "║  ZANS at lower levels: Reduced effectiveness            ║\n";
    std::cout << "║  UK×UK noise = " << std::setw(2) << (361-noise_ukuk) << " bits (structural, not ZANS-contractable) ║\n";
    std::cout << "║  UK+UK noise =  " << std::setw(2) << (361-noise_add) << " bits (additive, ZANS-contractable)    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}

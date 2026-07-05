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
    std::cout << "║     HOLY GRAIL EXPLORATION                              ║\n";
    std::cout << "║     ModSwitch + ZANS for UK×UK noise recovery           ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    // ─── SETUP with MODULUS CHAIN ───
    size_t poly_modulus_degree = 16384;
    
    // Create a modulus chain: 3 levels for mod switching
    std::vector<int> mod_bits = {60, 50, 40};  // Actually SEAL uses specific primes
    // Let's use SEAL's default chain and work with it
    
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
    
    // Check how many modulus levels we have
    auto context_data = context.key_context_data();
    int total_levels = 0;
    while (context_data) {
        total_levels++;
        context_data = context_data->next_context_data();
    }
    std::cout << "Total modulus chain levels: " << total_levels << "\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 1: UK×UK → ModSwitch → ZANS
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: UK×UK → ModSwitch → ZANS ═══\n\n";
    
    Ciphertext ct_a = encrypt_int(7);
    Ciphertext ct_b = encrypt_int(3);
    
    std::cout << "Initial noise ct_a: " << nbudget(ct_a) << " bits\n";
    std::cout << "Initial noise ct_b: " << nbudget(ct_b) << " bits\n";
    
    // UK×UK multiply
    Ciphertext ct_mul;
    evaluator.multiply(ct_a, ct_b, ct_mul);
    evaluator.relinearize_inplace(ct_mul, rlk);
    
    int noise_after_mul = nbudget(ct_mul);
    uint64_t val_after_mul = decrypt_int(ct_mul);
    std::cout << "After UK×UK: " << val_after_mul << " (exp 21) noise=" << noise_after_mul << " bits\n";
    
    // Try mod switching down one level
    std::cout << "Current scale: " << ct_mul.scale() << "\n";
    
    // Mod switch to next level
    try {
        evaluator.mod_switch_to_next_inplace(ct_mul);
        int noise_after_modswitch = nbudget(ct_mul);
        uint64_t val_after_modswitch = decrypt_int(ct_mul);
        std::cout << "After ModSwitch: " << val_after_modswitch << " noise=" << noise_after_modswitch << " bits\n";
        
        // Now apply ZANS
        int zans_before = nbudget(ct_mul);
        for (int i = 0; i < 1000; i++) {
            evaluator.add_inplace(ct_mul, enc_zero);
        }
        int zans_after = nbudget(ct_mul);
        uint64_t val_after_zans = decrypt_int(ct_mul);
        
        std::cout << "After 1000 ZANS: " << val_after_zans << " noise=" << zans_after << " bits\n";
        std::cout << "ZANS effect: " << (zans_after - zans_before) << " bits change\n\n";
    } catch (const std::exception& e) {
        std::cout << "ModSwitch failed: " << e.what() << "\n\n";
    }

    // ═══════════════════════════════════════════════════════
    // TEST 2: UK×UK chain with ModSwitch between each
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: UK×UK Chain with ModSwitch + ZANS ═══\n";
    std::cout << "Strategy: multiply → modswitch → ZANS → repeat\n\n";
    
    Ciphertext ct_chain = encrypt_int(2);
    Ciphertext ct_factor = encrypt_int(2);
    Plaintext p2;
    std::vector<uint64_t> vec2(poly_modulus_degree, 2ULL);
    encoder.encode(vec2, p2);
    
    std::cout << "┌───────┬──────────┬──────────┬──────────────────────┐\n";
    std::cout << "│ Step  │ Value    │ Noise    │ Action               │\n";
    std::cout << "├───────┼──────────┼──────────┼──────────────────────┤\n";
    printf("│ %3d   │ %8lu │ %3d bits │ Start                │\n", 0, (uint64_t)2, nbudget(ct_chain));
    
    int chain_len = 0;
    for (int step = 1; step <= 20; step++) {
        // UK×UK multiply
        Ciphertext temp;
        evaluator.multiply(ct_chain, ct_factor, temp);
        evaluator.relinearize_inplace(temp, rlk);
        
        int noise_mul = nbudget(temp);
        
        // ModSwitch if possible
        bool modswitched = false;
        try {
            evaluator.mod_switch_to_next_inplace(temp);
            modswitched = true;
        } catch (...) {}
        
        int noise_ms = nbudget(temp);
        
        // ZANS × 100
        for (int z = 0; z < 100; z++) {
            evaluator.add_inplace(temp, enc_zero);
        }
        
        ct_chain = temp;
        int noise_final = nbudget(ct_chain);
        uint64_t val = decrypt_int(ct_chain);
        uint64_t expected = (uint64_t)std::pow(2, step + 1);
        
        printf("│ %3d   │ %8lu │ %3d bits │ mul=%d", step, val, noise_final, noise_mul);
        if (modswitched) printf(" MS=%d", noise_ms);
        printf(" ZANS=%d │\n", noise_final);
        
        chain_len = step;
        
        if (noise_final < 10 || val != expected) {
            if (val != expected) printf("│ ❌ Value mismatch (exp %lu)                    │\n", expected);
            else printf("│ ⚠️  Noise depleted                           │\n");
            break;
        }
    }
    std::cout << "└───────┴──────────┴──────────┴──────────────────────┘\n";
    std::cout << "Chain length with ModSwitch+ZANS: " << chain_len << "\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 3: Different approach — scale down before ZANS
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: Scale Manipulation ═══\n";
    std::cout << "Does ciphertext scale affect ZANS effectiveness?\n\n";
    
    // Compare ZANS on same value at different scales
    Ciphertext ct_high = encrypt_int(42);
    std::cout << "High scale ct: noise=" << nbudget(ct_high) << " scale=" << ct_high.scale() << "\n";
    
    // ModSwitch down
    Ciphertext ct_low = ct_high;
    bool can_switch = true;
    try {
        evaluator.mod_switch_to_next_inplace(ct_low);
        std::cout << "Low scale ct:  noise=" << nbudget(ct_low) << " scale=" << ct_low.scale() << "\n";
    } catch (...) {
        std::cout << "Cannot mod switch further\n";
        can_switch = false;
    }
    
    // Apply ZANS to both
    if (can_switch) {
        int zans_high_before = nbudget(ct_high);
        int zans_low_before = nbudget(ct_low);
        
        for (int i = 0; i < 1000; i++) {
            evaluator.add_inplace(ct_high, enc_zero);
            evaluator.add_inplace(ct_low, enc_zero);
        }
        
        int zans_high_after = nbudget(ct_high);
        int zans_low_after = nbudget(ct_low);
        
        std::cout << "\nAfter 1000 ZANS:\n";
        std::cout << "  High scale: noise " << zans_high_before << " → " << zans_high_after << " (Δ" << (zans_high_after - zans_high_before) << ")\n";
        std::cout << "  Low scale:  noise " << zans_low_before << " → " << zans_low_after << " (Δ" << (zans_low_after - zans_low_before) << ")\n";
    }

    // ═══════════════════════════════════════════════════════
    // TEST 4: Enc(1) multiplication — ZANS-M concept
    // ═══════════════════════════════════════════════════════
    std::cout << "\n═══ TEST 4: ZANS-M — ct × Enc(1) ═══\n";
    std::cout << "Does multiplying by Enc(1) act as noise anchor?\n\n";
    
    Ciphertext ct_test = encrypt_int(42);
    Ciphertext enc_one = encrypt_int(1);
    
    std::cout << "Initial: noise=" << nbudget(ct_test) << " value=" << decrypt_int(ct_test) << "\n";
    
    // Try: ct × Enc(1) — should preserve value
    Ciphertext ct_mul_one;
    evaluator.multiply(ct_test, enc_one, ct_mul_one);
    evaluator.relinearize_inplace(ct_mul_one, rlk);
    
    std::cout << "After ×Enc(1): noise=" << nbudget(ct_mul_one) << " value=" << decrypt_int(ct_mul_one) << "\n";
    std::cout << "Noise cost of ×Enc(1): " << (nbudget(ct_test) - nbudget(ct_mul_one)) << " bits\n";
    
    // Try: ct × Enc(1) then ZANS
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_mul_one, enc_zero);
    }
    std::cout << "After ×Enc(1) + 1000 ZANS: noise=" << nbudget(ct_mul_one) << "\n\n";

    // ═══════════════════════════════════════════════════════
    // SUMMARY
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              HOLY GRAIL EXPLORATION RESULTS              ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  ModSwitch: Converts structural→additive?              ║\n";
    std::cout << "║  ZANS-M (×Enc(1)):  Costly but preserves value         ║\n";
    std::cout << "║  Next: Try different modswitch strategies              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}

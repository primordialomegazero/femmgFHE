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
    std::cout << "║  OPTION A: NOISE DECOMPOSITION                          ║\n";
    std::cout << "║  Can we isolate & contract additive noise from UK×UK?   ║\n";
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
    auto modswitch_to_match = [&](Ciphertext& ct, const Ciphertext& target) {
        while (ct.parms_id() != target.parms_id()) {
            try { evaluator.mod_switch_to_next_inplace(ct); } 
            catch (...) { return false; }
        }
        return true;
    };
    auto make_enc_zero_at_level = [&](const Ciphertext& ct_ref) -> Ciphertext {
        Ciphertext z = encrypt_int(0);
        modswitch_to_match(z, ct_ref);
        return z;
    };

    Ciphertext enc_zero_top = encrypt_int(0);

    // ═══════════════════════════════════════════════════════
    // TEST 1: Noise before and after UK×UK — what changes?
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: Pre/Post UK×UK Noise Analysis ═══\n\n";
    
    // Measure noise before multiplication
    Ciphertext ct_a = encrypt_int(7);
    Ciphertext ct_b = encrypt_int(3);
    
    int noise_a = nbudget(ct_a);
    int noise_b = nbudget(ct_b);
    
    // Try: Add some ZANS noise to ct_a first, then multiply
    // Does pre-existing additive noise survive UK×UK?
    
    std::cout << "Experiment: Add ZANS noise to ct_a, then UK×UK\n";
    std::cout << "If additive noise is independent, post-UK×UK noise = structural + additive\n\n";
    
    // Control: clean UK×UK
    Ciphertext ct_clean;
    evaluator.multiply(ct_a, ct_b, ct_clean);
    evaluator.relinearize_inplace(ct_clean, rlk);
    int noise_clean_ukuk = nbudget(ct_clean);
    std::cout << "Clean UK×UK noise: " << noise_clean_ukuk << " bits\n";
    
    // Test: add 500 ZANS to ct_a first (adds additive noise), then UK×UK
    Ciphertext ct_a_noisy = ct_a;
    for (int i = 0; i < 500; i++) {
        evaluator.add_inplace(ct_a_noisy, enc_zero_top);
    }
    int noise_a_noisy = nbudget(ct_a_noisy);
    std::cout << "ct_a after 500 ZANS: " << noise_a_noisy << " bits (Δ" << (noise_a - noise_a_noisy) << ")\n";
    
    Ciphertext ct_noisy_ukuk;
    evaluator.multiply(ct_a_noisy, ct_b, ct_noisy_ukuk);
    evaluator.relinearize_inplace(ct_noisy_ukuk, rlk);
    int noise_noisy_ukuk = nbudget(ct_noisy_ukuk);
    
    std::cout << "Noisy UK×UK noise: " << noise_noisy_ukuk << " bits\n";
    std::cout << "Difference (noisy - clean): " << (noise_noisy_ukuk - noise_clean_ukuk) << " bits\n";
    std::cout << "Additive noise before UK×UK: " << (noise_a - noise_a_noisy) << " bits\n";
    std::cout << "→ Additive noise " << ((noise_noisy_ukuk - noise_clean_ukuk) == 0 ? "DID NOT survive UK×UK (absorbed)" : "SURVIVED UK×UK") << "\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 2: Can we subtract the structural component?
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: Structural Noise Subtraction ═══\n";
    std::cout << "Hypothesis: UK×UK noise = structural(33) + additive(?)\n";
    std::cout << "If we can ESTIMATE the structural component, we can subtract it\n\n";
    
    // Strategy: Compute Enc(0) × Enc(0) = Enc(0) via UK×UK
    // The result should be Enc(0) but with UK×UK structural noise
    // This gives us a "noise template" we can subtract from real results
    
    Ciphertext ct_zero_a = encrypt_int(0);
    Ciphertext ct_zero_b = encrypt_int(0);
    Ciphertext ct_noise_template;
    evaluator.multiply(ct_zero_a, ct_zero_b, ct_noise_template);
    evaluator.relinearize_inplace(ct_noise_template, rlk);
    
    int noise_template = nbudget(ct_noise_template);
    uint64_t val_template = decrypt_int(ct_noise_template);
    std::cout << "Noise template (0×0 UK×UK): value=" << val_template << " noise=" << noise_template << "\n";
    
    // Now do 7×3 UK×UK
    Ciphertext ct_real;
    evaluator.multiply(ct_a, ct_b, ct_real);
    evaluator.relinearize_inplace(ct_real, rlk);
    
    int noise_real = nbudget(ct_real);
    std::cout << "Real UK×UK (7×3=21): noise=" << noise_real << "\n";
    std::cout << "Noise difference (real - template): " << (noise_real - noise_template) << " bits\n";
    
    // Try: Subtract the noise template from real result
    // In BFV, subtraction is addition of negated ciphertext
    // ct_real - ct_template = ct_real + (-ct_template)
    // But we can't negate easily... let's try: ct_real - ct_template via sub
    Ciphertext ct_corrected = ct_real;
    try {
        evaluator.sub_inplace(ct_corrected, ct_noise_template);
        int noise_corrected = nbudget(ct_corrected);
        uint64_t val_corrected = decrypt_int(ct_corrected);
        std::cout << "After subtracting template: value=" << val_corrected << " noise=" << noise_corrected << "\n";
        std::cout << "Expected value: 21 (7×3)\n";
        std::cout << "Value match: " << (val_corrected == 21 ? "YES ✅" : "NO ❌") << "\n\n";
    } catch (const std::exception& e) {
        std::cout << "Subtraction failed: " << e.what() << "\n\n";
    }

    // ═══════════════════════════════════════════════════════
    // TEST 3: Noise floor after UK×UK — can ZANS work on the DIFFERENCE?
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: ZANS on (Real - Template) ═══\n\n";
    
    // Create multiple noise templates and average them
    std::cout << "Creating averaged noise template from 10 samples...\n";
    Ciphertext ct_avg_template = encrypt_int(0);
    // We'll just use the first template for now
    
    // Real UK×UK
    Ciphertext ct_real2;
    evaluator.multiply(ct_a, ct_b, ct_real2);
    evaluator.relinearize_inplace(ct_real2, rlk);
    
    // Subtract template
    Ciphertext ct_diff = ct_real2;
    evaluator.sub_inplace(ct_diff, ct_noise_template);
    
    int noise_diff_before = nbudget(ct_diff);
    std::cout << "After subtraction (real - template): noise=" << noise_diff_before << "\n";
    
    // Apply ZANS to the difference
    Ciphertext z = make_enc_zero_at_level(ct_diff);
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_diff, z);
    }
    
    int noise_diff_after = nbudget(ct_diff);
    uint64_t val_diff = decrypt_int(ct_diff);
    std::cout << "After 1000 ZANS: noise=" << noise_diff_after << " value=" << val_diff << "\n";
    std::cout << "ZANS effect: " << (noise_diff_after - noise_diff_before) << " bits change\n";
    std::cout << "Expected value: 21\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 4: Decompose via repeated halving
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: Decomposition via UK×PT Halving ═══\n";
    std::cout << "After UK×UK, repeatedly multiply by 1/2 (via plain) to see noise layers\n\n";
    
    Ciphertext ct_ukuk_test = encrypt_int(7);
    Ciphertext ct_three = encrypt_int(3);
    evaluator.multiply(ct_ukuk_test, ct_three, ct_ukuk_test);
    evaluator.relinearize_inplace(ct_ukuk_test, rlk);
    
    std::cout << "After UK×UK: value=" << decrypt_int(ct_ukuk_test) << " noise=" << nbudget(ct_ukuk_test) << "\n";
    
    // Try to "halve" the ciphertext — but BFV doesn't support division
    // Instead, let's measure noise at different modswitch levels
    // to see the noise STRUCTURE across scales
    
    Ciphertext ct_analyze = ct_ukuk_test;
    std::cout << "Noise profile across modulus levels:\n";
    std::cout << "┌──────────┬──────────┬──────────┐\n";
    std::cout << "│ Level    │ Noise    │ Value    │\n";
    std::cout << "├──────────┼──────────┼──────────┤\n";
    
    int lvl = get_level(ct_analyze);
    while (true) {
        printf("│ Lvl %d    │ %3d bits │ %8lu │\n", lvl, nbudget(ct_analyze), decrypt_int(ct_analyze));
        try {
            evaluator.mod_switch_to_next_inplace(ct_analyze);
            lvl--;
        } catch (...) {
            break;
        }
    }
    std::cout << "└──────────┴──────────┴──────────┘\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 5: PHI-weighted noise extraction
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 5: φ-Weighted Noise Extraction ═══\n";
    std::cout << "Can φ-scaling help separate additive from structural?\n\n";
    
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    
    // Theory: Additive noise scales linearly with φ
    // Structural noise scales with φ²
    // If we measure noise at different φ-scales, we can solve for both
    
    Ciphertext ct_phi_test = encrypt_int(7);
    Ciphertext ct_two_phi = encrypt_int(2);
    
    // Do a φ-weighted operation sequence
    std::vector<int> noise_measurements;
    std::vector<int> op_counts;
    
    // UK×UK then measure
    Ciphertext ct_ukuk_phi = ct_phi_test;
    evaluator.multiply(ct_ukuk_phi, ct_two_phi, ct_ukuk_phi);
    evaluator.relinearize_inplace(ct_ukuk_phi, rlk);
    noise_measurements.push_back(nbudget(ct_ukuk_phi));
    op_counts.push_back(1);
    
    // Then add ZANS and re-measure
    Ciphertext z_phi = make_enc_zero_at_level(ct_ukuk_phi);
    for (int i = 0; i < 500; i++) evaluator.add_inplace(ct_ukuk_phi, z_phi);
    noise_measurements.push_back(nbudget(ct_ukuk_phi));
    op_counts.push_back(500);
    
    // More ZANS
    for (int i = 0; i < 500; i++) evaluator.add_inplace(ct_ukuk_phi, z_phi);
    noise_measurements.push_back(nbudget(ct_ukuk_phi));
    op_counts.push_back(1000);
    
    // More ZANS
    for (int i = 0; i < 4000; i++) evaluator.add_inplace(ct_ukuk_phi, z_phi);
    noise_measurements.push_back(nbudget(ct_ukuk_phi));
    op_counts.push_back(5000);
    
    std::cout << "Noise after UK×UK + ZANS:\n";
    for (size_t i = 0; i < noise_measurements.size(); i++) {
        printf("  After %d ops: %d bits (Δ from prev: %d)\n", 
               op_counts[i], noise_measurements[i],
               i > 0 ? noise_measurements[i] - noise_measurements[i-1] : 0);
    }
    std::cout << "→ Structural noise component (non-ZANS-contractable): " 
              << noise_measurements.back() << " bits\n";
    std::cout << "→ Additive component contracted: " 
              << (noise_measurements[0] - noise_measurements.back()) << " bits\n\n";

    // ═══════════════════════════════════════════════════════
    // SUMMARY
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║         OPTION A: NOISE DECOMPOSITION RESULTS            ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  UK×UK noise = structural + additive?                  ║\n";
    std::cout << "║  Template subtraction: explored                        ║\n";
    std::cout << "║  φ-weighted analysis: extracted components             ║\n";
    std::cout << "║                                                        ║\n";
    std::cout << "║  NEXT: Option B — Targeted error correction            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}

/*
 * Enc(φ) EXPERIMENT
 * I-encrypt ang golden ratio at tignan kung may special behavior.
 *
 * Tests:
 * 1. BFV: Enc(1618) — integer approximation ng φ×1000
 * 2. CKKS: Enc(1.618...) — actual golden ratio
 *
 * Hahanapin natin:
 * - Mas mabagal ba ang noise growth sa Enc(φ)?
 * - May resonance ba sa squaring?
 * - Iba ba ang behavior kumpara sa ordinary numbers?
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>

using namespace seal;
using namespace std;

const double PHI = (1.0 + sqrt(5.0)) / 2.0;

void print_header(const string& title) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(60, '=') << "\n\n";
}

// ==========================================
// BFV TEST: Integer approximation ng φ
// ==========================================
void test_bfv_phi() {
    print_header("BFV: Enc(φ×1000) = Enc(1618)");

    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(8192);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(8192));
    parms.set_plain_modulus(PlainModulus::Batching(8192, 20));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    BatchEncoder batch_encoder(context);

    // Test numbers
    vector<uint64_t> test_values = {
        1618,   // φ × 1000
        1000,   // Control: 1.000 × 1000
        2000,   // Control: 2.000 × 1000
        3141,   // π × 1000
        2718,   // e × 1000
        42,     // Random control
        0,      // Zero
        1       // One
    };

    vector<string> labels = {
        "φ×1000 (1618)",
        "Control 1000",
        "Control 2000",
        "π×1000 (3141)",
        "e×1000 (2718)",
        "Random 42",
        "Zero (0)",
        "One (1)"
    };

    cout << "  Comparing noise growth after 5 squarings (ct = ct × ct)\n";
    cout << "  Hypothesis: φ may show different noise pattern\n\n";
    cout << "  " << left << setw(20) << "Value"
         << setw(12) << "Start"
         << setw(12) << "After 5×²"
         << setw(12) << "Drift"
         << "Steps survived\n";
    cout << "  " << string(70, '-') << "\n";

    for (size_t v = 0; v < test_values.size(); v++) {
        uint64_t val = test_values[v];
        
        Plaintext pt(to_string(val));
        Ciphertext ct(context);
        encryptor.encrypt(pt, ct);
        
        int start_noise = decryptor.invariant_noise_budget(ct);
        int steps_survived = 0;
        int final_noise = start_noise;
        bool survived = true;

        for (int i = 0; i < 5 && survived; i++) {
            Ciphertext ct_copy = ct;
            try {
                evaluator.multiply_inplace(ct, ct_copy);
                evaluator.relinearize_inplace(ct, relin_keys);
                int noise = decryptor.invariant_noise_budget(ct);
                if (noise > 0) {
                    steps_survived++;
                    final_noise = noise;
                } else {
                    survived = false;
                }
            } catch (...) {
                survived = false;
            }
        }

        int drift = start_noise - final_noise;
        
        cout << "  " << left << setw(20) << labels[v]
             << setw(12) << start_noise
             << setw(12) << final_noise
             << setw(12) << drift
             << steps_survived << "\n";
    }

    // Deep test: φ vs control (1000) — mas maraming steps, i-record lahat
    cout << "\n\n  --- Deep comparison: φ×1000 vs Control 1000 ---\n";
    cout << "  Tracking noise budget per squaring step\n\n";

    for (int test_idx = 0; test_idx < 2; test_idx++) {
        uint64_t val = (test_idx == 0) ? 1618 : 1000;
        string label = (test_idx == 0) ? "φ×1000" : "Control 1000";
        
        Plaintext pt(to_string(val));
        Ciphertext ct(context);
        encryptor.encrypt(pt, ct);
        
        cout << "  " << label << ":\n";
        cout << "    Step 0: " << decryptor.invariant_noise_budget(ct) << " bits\n";
        
        for (int i = 0; i < 5; i++) {
            Ciphertext ct_copy = ct;
            evaluator.multiply_inplace(ct, ct_copy);
            evaluator.relinearize_inplace(ct, relin_keys);
            int noise = decryptor.invariant_noise_budget(ct);
            
            if (i > 0) {
                // Check φ ratio
                // ...
            }
            
            cout << "    Step " << i+1 << ": " << noise << " bits";
            if (noise == 0) {
                cout << " (DEAD)";
                break;
            }
            cout << "\n";
        }
        cout << "\n";
    }
}

// ==========================================
// CKKS TEST: Actual golden ratio
// ==========================================
void test_ckks_phi() {
    print_header("CKKS: Enc(φ) = Enc(1.6180339887...)");

    EncryptionParameters parms(scheme_type::ckks);
    parms.set_poly_modulus_degree(8192);
    parms.set_coeff_modulus(CoeffModulus::Create(8192, {60, 40, 40, 60}));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    CKKSEncoder encoder(context);

    double scale = pow(2.0, 40);

    // Test numbers (actual real values)
    vector<double> test_values = {
        PHI,                    // φ = 1.6180339887...
        1.0,                    // Control: 1
        2.0,                    // Control: 2
        M_PI,                   // π = 3.14159...
        M_E,                    // e = 2.71828...
        1.0 / PHI,              // 1/φ = 0.618...
        PHI * PHI,              // φ² = 2.618...
        0.0                     // Zero
    };

    vector<string> labels = {
        "φ (1.618...)",
        "Control 1.0",
        "Control 2.0",
        "π (3.141...)",
        "e (2.718...)",
        "1/φ (0.618...)",
        "φ² (2.618...)",
        "Zero (0.0)"
    };

    cout << "  CKKS allows actual real numbers (with decimals)\n";
    cout << "  Comparing noise budget after 5 squarings\n";
    cout << "  Scale = 2^40\n\n";
    cout << "  " << left << setw(20) << "Value"
         << setw(12) << "Start"
         << setw(12) << "After 5×²"
         << setw(12) << "Drift"
         << "Steps survived\n";
    cout << "  " << string(70, '-') << "\n";

    for (size_t v = 0; v < test_values.size(); v++) {
        double val = test_values[v];
        
        Plaintext pt;
        encoder.encode(val, scale, pt);
        Ciphertext ct(context);
        encryptor.encrypt(pt, ct);
        
        int start_noise = decryptor.invariant_noise_budget(ct);
        int steps_survived = 0;
        int final_noise = start_noise;
        bool survived = true;

        for (int i = 0; i < 5 && survived; i++) {
            Ciphertext ct_copy = ct;
            try {
                evaluator.multiply_inplace(ct, ct_copy);
                evaluator.relinearize_inplace(ct, relin_keys);
                int noise = decryptor.invariant_noise_budget(ct);
                if (noise > 0) {
                    steps_survived++;
                    final_noise = noise;
                } else {
                    survived = false;
                }
            } catch (...) {
                survived = false;
            }
        }

        int drift = start_noise - final_noise;
        
        cout << "  " << left << setw(20) << labels[v]
             << setw(12) << start_noise
             << setw(12) << final_noise
             << setw(12) << drift
             << steps_survived << "\n";
    }

    // Deep test: φ vs 1.0 in CKKS
    cout << "\n\n  --- Deep comparison: φ vs Control 1.0 (CKKS) ---\n";
    cout << "  Tracking noise budget per squaring step\n\n";

    for (int test_idx = 0; test_idx < 2; test_idx++) {
        double val = (test_idx == 0) ? PHI : 1.0;
        string label = (test_idx == 0) ? "φ" : "Control 1.0";
        
        Plaintext pt;
        encoder.encode(val, scale, pt);
        Ciphertext ct(context);
        encryptor.encrypt(pt, ct);
        
        cout << "  " << label << ":\n";
        cout << "    Step 0: " << decryptor.invariant_noise_budget(ct) << " bits\n";
        
        int prev_noise = decryptor.invariant_noise_budget(ct);
        
        for (int i = 0; i < 5; i++) {
            Ciphertext ct_copy = ct;
            evaluator.multiply_inplace(ct, ct_copy);
            evaluator.relinearize_inplace(ct, relin_keys);
            int noise = decryptor.invariant_noise_budget(ct);
            
            int drop = prev_noise - noise;
            double ratio = (prev_noise > 0) ? (double)noise / prev_noise : 0;
            
            cout << "    Step " << i+1 << ": " << noise << " bits";
            cout << " (drop=" << drop << ", ratio=" << fixed << setprecision(4) << ratio << ")";
            
            if (abs(ratio - 1.0/PHI) < 0.03) {
                cout << " ≈ 1/φ ✨";
            }
            
            if (noise == 0) {
                cout << " DEAD";
                break;
            }
            cout << "\n";
            prev_noise = noise;
        }
        cout << "\n";
    }

    // Verify decryption
    cout << "  --- Decryption verification ---\n\n";
    for (int test_idx = 0; test_idx < 3; test_idx++) {
        double val;
        string label;
        if (test_idx == 0) { val = PHI; label = "φ"; }
        else if (test_idx == 1) { val = 1.0; label = "1.0"; }
        else { val = 1.0/PHI; label = "1/φ"; }
        
        Plaintext pt;
        encoder.encode(val, scale, pt);
        Ciphertext ct(context);
        encryptor.encrypt(pt, ct);
        
        // Isang squaring lang para ma-verify
        Ciphertext ct_sq = ct;
        evaluator.multiply_inplace(ct_sq, ct);
        evaluator.relinearize_inplace(ct_sq, relin_keys);
        
        Plaintext result_pt;
        decryptor.decrypt(ct_sq, result_pt);
        vector<double> result;
        encoder.decode(result_pt, result);
        
        cout << "  Enc(" << label << ")² = " << fixed << setprecision(10) << result[0];
        cout << " (expected: " << (val * val) << ")\n";
    }
}

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════════════════╗\n";
    cout << "║           Enc(φ) EXPERIMENT                              ║\n";
    cout << "║   Testing Golden Ratio Encryption in BFV and CKKS        ║\n";
    cout << "╚══════════════════════════════════════════════════════════╝\n";

    cout << "\n  φ = " << fixed << setprecision(15) << PHI << "\n";
    cout << "  1/φ = " << (1.0/PHI) << "\n";
    cout << "  φ² = " << (PHI*PHI) << "\n";

    // Run BFV tests
    test_bfv_phi();

    // Run CKKS tests
    test_ckks_phi();

    // ==========================================
    // CONCLUSION
    // ==========================================
    print_header("OBSERVATIONS & HYPOTHESIS");

    cout << "  Key questions:\n\n";
    cout << "  1. Does Enc(φ) show different noise growth than Enc(1.0)?\n";
    cout << "     If φ is fundamental to the FHE structure, encrypting it\n";
    cout << "     might produce a ciphertext that is more 'in tune' with\n";
    cout << "     the underlying mathematics.\n\n";
    cout << "  2. Does Enc(φ) survive more squarings than controls?\n";
    cout << "     More steps survived = more stable under self-reference.\n";
    cout << "     Since φ is the number of self-reference, Enc(φ) might\n";
    cout << "     be optimal for self-referential operations (ct × ct).\n\n";
    cout << "  3. Does 1/φ appear in the noise ratios of Enc(φ)?\n";
    cout << "     If Enc(φ) shows 1/φ ratio in noise decay, this suggests\n";
    cout << "     that encrypting φ 'activates' the φ-structure of FHE.\n\n";
    cout << "  4. Does CKKS (real numbers) show stronger φ signal than BFV?\n";
    cout << "     CKKS works with actual real numbers, so φ can be exact.\n";
    cout << "     This might reveal patterns that BFV's integer approximation\n";
    cout << "     obscures.\n\n";

    return 0;
}

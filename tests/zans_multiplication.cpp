/*
 * ZANS vs ORDINARY MULTIPLICATION
 * Dito natin makikita kung ang ZANS ba ay may advantage
 * sa multiplication, at kung may φ/Fibonacci pattern
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>

using namespace seal;
using namespace std;

void print_header(const string& title) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(60, '=') << "\n\n";
}

int main() {
    print_header("ZANS vs ORDINARY - MULTIPLICATION TEST");

    // Setup SEAL
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

    Plaintext pt_one("1");
    Plaintext pt_zero("0");
    Plaintext pt_result;

    // Fresh noise
    Ciphertext fresh_ct(context);
    encryptor.encrypt(pt_one, fresh_ct);
    int fresh_noise = decryptor.invariant_noise_budget(fresh_ct);
    cout << "  Fresh ciphertext noise budget: " << fresh_noise << " bits\n\n";

    // ==========================================
    // TEST 1: MULTIPLY BY Enc(0) - ZANS style
    // ==========================================
    cout << "--- TEST 1: Multiply by Enc(0) (ZANS multiplication) ---\n";
    cout << "    ct = ct * Enc(0)\n\n";

    Ciphertext ct_mul_zero(context);
    encryptor.encrypt(pt_one, ct_mul_zero);
    int mul_zero_start = decryptor.invariant_noise_budget(ct_mul_zero);

    Ciphertext ct_zero(context);
    encryptor.encrypt(pt_zero, ct_zero);

    auto start = chrono::high_resolution_clock::now();

    // Multiply by Enc(0) 10 beses lang — multiplication is expensive
    vector<int> mul_zero_noise;
    mul_zero_noise.push_back(mul_zero_start);

    for (int i = 0; i < 10; i++) {
        Ciphertext zero_copy = ct_zero;
        evaluator.multiply_inplace(ct_mul_zero, zero_copy);
        evaluator.relinearize_inplace(ct_mul_zero, relin_keys);
        mul_zero_noise.push_back(decryptor.invariant_noise_budget(ct_mul_zero));
    }

    auto end = chrono::high_resolution_clock::now();
    double mul_zero_time = chrono::duration<double>(end - start).count();

    decryptor.decrypt(ct_mul_zero, pt_result);
    cout << "  Start noise: " << mul_zero_start << " bits\n";
    cout << "  End noise:   " << mul_zero_noise.back() << " bits\n";
    cout << "  Drift:       " << (mul_zero_start - mul_zero_noise.back()) << " bits\n";
    cout << "  Value:       " << pt_result.to_string() << " (expected 0 — kasi ×0)\n";
    cout << "  Time:        " << mul_zero_time << " sec\n\n";

    // Print per-step noise
    cout << "  Step-by-step noise:\n";
    for (size_t i = 0; i < mul_zero_noise.size(); i++) {
        cout << "    Step " << i << ": " << mul_zero_noise[i] << " bits";
        if (i > 0) {
            int drop = mul_zero_noise[i-1] - mul_zero_noise[i];
            cout << " (-" << drop << ")";
        }
        cout << "\n";
    }
    cout << "\n";

    // ==========================================
    // TEST 2: MULTIPLY BY Enc(1) - Ordinary
    // ==========================================
    cout << "--- TEST 2: Multiply by Enc(1) (ordinary multiplication) ---\n";
    cout << "    ct = ct * Enc(1)\n\n";

    Ciphertext ct_mul_one(context);
    encryptor.encrypt(pt_one, ct_mul_one);
    int mul_one_start = decryptor.invariant_noise_budget(ct_mul_one);

    Ciphertext ct_one(context);
    encryptor.encrypt(pt_one, ct_one);

    start = chrono::high_resolution_clock::now();

    vector<int> mul_one_noise;
    mul_one_noise.push_back(mul_one_start);

    for (int i = 0; i < 10; i++) {
        Ciphertext one_copy = ct_one;
        evaluator.multiply_inplace(ct_mul_one, one_copy);
        evaluator.relinearize_inplace(ct_mul_one, relin_keys);
        mul_one_noise.push_back(decryptor.invariant_noise_budget(ct_mul_one));
    }

    end = chrono::high_resolution_clock::now();
    double mul_one_time = chrono::duration<double>(end - start).count();

    decryptor.decrypt(ct_mul_one, pt_result);
    cout << "  Start noise: " << mul_one_start << " bits\n";
    cout << "  End noise:   " << mul_one_noise.back() << " bits\n";
    cout << "  Drift:       " << (mul_one_start - mul_one_noise.back()) << " bits\n";
    cout << "  Value:       " << pt_result.to_string() << " (expected 1 — kasi ×1)\n";
    cout << "  Time:        " << mul_one_time << " sec\n\n";

    cout << "  Step-by-step noise:\n";
    for (size_t i = 0; i < mul_one_noise.size(); i++) {
        cout << "    Step " << i << ": " << mul_one_noise[i] << " bits";
        if (i > 0) {
            int drop = mul_one_noise[i-1] - mul_one_noise[i];
            cout << " (-" << drop << ")";
        }
        cout << "\n";
    }
    cout << "\n";

    // ==========================================
    // TEST 3: ct * ct — SQUARING
    // ==========================================
    cout << "--- TEST 3: Squaring (ct = ct * ct) ---\n";
    cout << "    Ito ang pinakamahalagang test\n\n";

    Ciphertext ct_sq(context);
    encryptor.encrypt(pt_one, ct_sq);
    int sq_start = decryptor.invariant_noise_budget(ct_sq);

    start = chrono::high_resolution_clock::now();

    vector<int> sq_noise;
    sq_noise.push_back(sq_start);

    for (int i = 0; i < 10; i++) {
        Ciphertext ct_copy = ct_sq;
        evaluator.multiply_inplace(ct_sq, ct_copy);
        evaluator.relinearize_inplace(ct_sq, relin_keys);
        sq_noise.push_back(decryptor.invariant_noise_budget(ct_sq));
    }

    end = chrono::high_resolution_clock::now();
    double sq_time = chrono::duration<double>(end - start).count();

    decryptor.decrypt(ct_sq, pt_result);
    cout << "  Start noise: " << sq_start << " bits\n";
    cout << "  End noise:   " << sq_noise.back() << " bits\n";
    cout << "  Drift:       " << (sq_start - sq_noise.back()) << " bits\n";
    cout << "  Value:       " << pt_result.to_string() << "\n";
    cout << "  Time:        " << sq_time << " sec\n\n";

    cout << "  Step-by-step noise:\n";
    for (size_t i = 0; i < sq_noise.size(); i++) {
        cout << "    Step " << i << ": " << sq_noise[i] << " bits";
        if (i > 0) {
            int drop = sq_noise[i-1] - sq_noise[i];
            cout << " (-" << drop << ")";
        }
        cout << "\n";
    }
    cout << "\n";

    // ==========================================
    // GOLDEN RATIO / FIBONACCI ANALYSIS
    // ==========================================
    print_header("GOLDEN RATIO / FIBONACCI ANALYSIS");

    double phi = (1.0 + sqrt(5.0)) / 2.0;
    cout << "  φ (Golden Ratio) = " << phi << "\n\n";

    // Fibonacci sequence
    vector<int> fib;
    fib.push_back(0);
    fib.push_back(1);
    for (int i = 2; i < 15; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }
    cout << "  Fibonacci numbers: ";
    for (size_t i = 0; i < fib.size(); i++) {
        cout << fib[i] << " ";
    }
    cout << "\n\n";

    // Suriin ang noise drops sa squaring
    cout << "  --- Noise drops per squaring step ---\n";
    if (sq_noise.size() > 1) {
        for (size_t i = 1; i < sq_noise.size(); i++) {
            int drop = sq_noise[i-1] - sq_noise[i];
            cout << "    Step " << i << ": noise dropped by " << drop << " bits";
            
            // Check kung malapit sa Fibonacci
            for (size_t j = 0; j < fib.size(); j++) {
                if (abs(drop - fib[j]) <= 2) {
                    cout << "  (malapit sa Fib[" << j << "] = " << fib[j] << ")";
                }
            }
            
            // Check ratio sa naunang drop
            if (i > 1) {
                int prev_drop = sq_noise[i-2] - sq_noise[i-1];
                if (prev_drop > 0) {
                    double ratio = (double)drop / prev_drop;
                    cout << "  ratio = " << fixed << setprecision(3) << ratio;
                    if (abs(ratio - phi) < 0.1) {
                        cout << " ≈ φ! ✨";
                    }
                }
            }
            cout << "\n";
        }
    }

    // Suriin ang ratios between successive noise levels
    cout << "\n  --- Noise budget ratios (bₙ/bₙ₋₁) ---\n";
    for (size_t i = 1; i < sq_noise.size(); i++) {
        if (sq_noise[i-1] > 0) {
            double ratio = (double)sq_noise[i] / sq_noise[i-1];
            cout << "    Step " << i << ": " << fixed << setprecision(4) << ratio;
            if (abs(ratio - 1.0/phi) < 0.05) {
                cout << " ≈ 1/φ! ✨";
            }
            if (abs(ratio - 0.618) < 0.05) {
                cout << " ≈ φ-1! ✨";
            }
            cout << "\n";
        }
    }

    // ==========================================
    // CONCLUSION
    // ==========================================
    print_header("CONCLUSION");

    cout << "  Multiplication noise drift:\n";
    cout << "    Multiply by Enc(0): " << (mul_zero_start - mul_zero_noise.back()) << " bits\n";
    cout << "    Multiply by Enc(1): " << (mul_one_start - mul_one_noise.back()) << " bits\n";
    cout << "    Squaring (ct×ct):   " << (sq_start - sq_noise.back()) << " bits\n\n";

    cout << "  Tandaan:\n";
    cout << "  - Ang φ ay lumilitaw sa maraming natural phenomena\n";
    cout << "  - Kung ang noise growth ay sumusunod sa φ-scaling,\n";
    cout << "    ibig sabihin may underlying mathematical structure\n";
    cout << "    na pwedeng ma-exploit para sa noise control.\n";
    cout << "  - Ang Fibonacci numbers ay related sa φ: Fₙ/Fₙ₋₁ → φ\n";
    cout << "  - Kung ang noise drops ay Fibonacci numbers, posibleng\n";
    cout << "    may optimal na sequence ng operations na pwedeng gawin.\n\n";

    return 0;
}

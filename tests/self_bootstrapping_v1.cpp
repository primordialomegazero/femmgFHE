/*
 * SELF-BOOTSTRAPPING NOISE - V1
 * 
 * Test: ZANS sequence at φ-point (Fibonacci noise level)
 * 
 * Steps:
 * 1. Start with fresh ciphertext
 * 2. Square until noise ≈ 55 bits (Fib[10])
 * 3. Apply ZANS sequence (many Enc(0) additions)
 * 4. Measure if noise budget increases
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace seal;
using namespace std;

const double PHI = (1.0 + sqrt(5.0)) / 2.0;

void print_header(const string& title) {
    cout << "\n" << string(65, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(65, '=') << "\n\n";
}

int main() {
    print_header("SELF-BOOTSTRAPPING NOISE - V1");
    cout << "  φ = " << fixed << setprecision(6) << PHI << "\n";
    cout << "  Fibonacci numbers: 0,1,1,2,3,5,8,13,21,34,55,89,144,233,377\n\n";
    
    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
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
    
    Plaintext pt_val("42");
    Plaintext pt_zero("0");
    
    // ==========================================
    // PHASE 1: Square until we hit ~55 bits
    // ==========================================
    print_header("PHASE 1: Squaring to φ-point (~55 bits)");
    
    Ciphertext ct(context);
    encryptor.encrypt(pt_val, ct);
    
    int start_noise = decryptor.invariant_noise_budget(ct);
    cout << "  Initial noise: " << start_noise << " bits\n\n";
    
    vector<int> noise_trajectory;
    noise_trajectory.push_back(start_noise);
    
    int current_noise = start_noise;
    int square_count = 0;
    
    // Square until noise ≈ 55 bits (Fib[10]) or close
    while (current_noise > 60 && square_count < 15) {
        Ciphertext ct_copy = ct;
        evaluator.multiply_inplace(ct, ct_copy);
        evaluator.relinearize_inplace(ct, relin_keys);
        square_count++;
        current_noise = decryptor.invariant_noise_budget(ct);
        noise_trajectory.push_back(current_noise);
        
        cout << "  Square " << square_count << ": " << current_noise << " bits";
        
        // Check if near Fibonacci number
        for (int fib : {55, 89, 144, 233, 377}) {
            if (abs(current_noise - fib) <= 3) {
                cout << " ≈ Fib[" << (fib == 55 ? 10 : fib == 89 ? 11 : fib == 144 ? 12 : fib == 233 ? 13 : 14) << "]=" << fib << " ✨";
            }
        }
        cout << "\n";
    }
    
    cout << "\n  Arrived at noise: " << current_noise << " bits after " << square_count << " squarings\n";
    int noise_at_phi_point = current_noise;
    
    // ==========================================
    // PHASE 2: Apply ZANS sequence
    // ==========================================
    print_header("PHASE 2: ZANS Sequence at φ-Point");
    
    // Test different ZANS sequence lengths
    vector<int> zans_lengths = {1, 2, 3, 5, 8, 13, 21, 34, 55, 100, 500, 1000};
    
    cout << "  Testing ZANS sequences at noise=" << noise_at_phi_point << " bits\n";
    cout << "  " << left << setw(10) << "ZANS Ops"
         << setw(15) << "Noise Before"
         << setw(15) << "Noise After"
         << setw(15) << "Change"
         << "Result\n";
    cout << "  " << string(70, '-') << "\n";
    
    for (int zans_ops : zans_lengths) {
        // Create fresh ciphertext and square to same point
        Ciphertext ct_test(context);
        encryptor.encrypt(pt_val, ct_test);
        
        // Square to same point
        for (int i = 0; i < square_count; i++) {
            Ciphertext ct_copy = ct_test;
            evaluator.multiply_inplace(ct_test, ct_copy);
            evaluator.relinearize_inplace(ct_test, relin_keys);
        }
        
        int noise_before = decryptor.invariant_noise_budget(ct_test);
        
        // Apply ZANS sequence
        Ciphertext ct_zero(context);
        for (int i = 0; i < zans_ops; i++) {
            encryptor.encrypt(pt_zero, ct_zero);
            evaluator.add_inplace(ct_test, ct_zero);
        }
        
        int noise_after = decryptor.invariant_noise_budget(ct_test);
        int change = noise_after - noise_before;
        
        // Verify value still correct
        Plaintext pt_result;
        decryptor.decrypt(ct_test, pt_result);
        bool correct = (pt_result.to_string() == "42");
        
        cout << "  " << left << setw(10) << zans_ops
             << setw(15) << noise_before
             << setw(15) << noise_after
             << setw(15) << (change >= 0 ? "+" + to_string(change) : to_string(change))
             << (correct ? "✅" : "❌");
        
        if (change > 0) {
            cout << " ⬆ NOISE BUDGET INCREASED!";
        }
        cout << "\n";
    }
    
    // ==========================================
    // PHASE 3: Deep ZANS at different φ-points
    // ==========================================
    print_header("PHASE 3: ZANS at Different φ-Points");
    
    // Target different Fibonacci noise levels
    vector<int> fib_targets = {89, 55, 34, 21, 13};
    int zans_depth = 34; // Fib[9]
    
    cout << "  Applying " << zans_depth << " ZANS ops at different φ-points\n";
    cout << "  " << left << setw(15) << "Target Noise"
         << setw(15) << "Actual Before"
         << setw(15) << "After ZANS"
         << setw(15) << "Change\n";
    cout << "  " << string(60, '-') << "\n";
    
    for (int target : fib_targets) {
        Ciphertext ct_target(context);
        encryptor.encrypt(pt_val, ct_target);
        
        // Square until we reach near target
        int noise_before = decryptor.invariant_noise_budget(ct_target);
        int sq = 0;
        while (noise_before > target + 10 && sq < 15) {
            Ciphertext ct_copy = ct_target;
            evaluator.multiply_inplace(ct_target, ct_copy);
            evaluator.relinearize_inplace(ct_target, relin_keys);
            noise_before = decryptor.invariant_noise_budget(ct_target);
            sq++;
        }
        
        // Apply ZANS sequence
        Ciphertext ct_zero(context);
        for (int i = 0; i < zans_depth; i++) {
            encryptor.encrypt(pt_zero, ct_zero);
            evaluator.add_inplace(ct_target, ct_zero);
        }
        
        int noise_after = decryptor.invariant_noise_budget(ct_target);
        int change = noise_after - noise_before;
        
        cout << "  " << left << setw(15) << target
             << setw(15) << noise_before
             << setw(15) << noise_after
             << setw(15) << (change >= 0 ? "+" + to_string(change) : to_string(change));
        
        if (change > 0) cout << " ⬆";
        cout << "\n";
    }
    
    // ==========================================
    // PHASE 4: Fibonacci ZANS Sequence
    // ==========================================
    print_header("PHASE 4: Fibonacci-Weighted ZANS");
    
    // Apply ZANS with Fibonacci-spaced iterations
    // Pattern: 1, 1, 2, 3, 5, 8, 13, 21, 34, 55
    vector<int> fib_sequence = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    
    cout << "  Applying Fibonacci-weighted ZANS sequence\n";
    cout << "  Pattern: ";
    for (int f : fib_sequence) cout << f << " ";
    cout << "\n\n";
    
    Ciphertext ct_fib(context);
    encryptor.encrypt(pt_val, ct_fib);
    
    // Square to ~89 bits
    int noise_fib = decryptor.invariant_noise_budget(ct_fib);
    while (noise_fib > 100) {
        Ciphertext ct_copy = ct_fib;
        evaluator.multiply_inplace(ct_fib, ct_copy);
        evaluator.relinearize_inplace(ct_fib, relin_keys);
        noise_fib = decryptor.invariant_noise_budget(ct_fib);
    }
    
    cout << "  Starting noise: " << noise_fib << " bits\n\n";
    cout << "  " << left << setw(10) << "Fib Step"
         << setw(10) << "ZANS Ops"
         << setw(15) << "Noise Before"
         << setw(15) << "Noise After"
         << setw(15) << "Change\n";
    cout << "  " << string(65, '-') << "\n";
    
    for (size_t i = 0; i < fib_sequence.size(); i++) {
        int zans_ops = fib_sequence[i];
        int noise_before_fib = decryptor.invariant_noise_budget(ct_fib);
        
        // Apply ZANS
        Ciphertext ct_zero(context);
        for (int j = 0; j < zans_ops; j++) {
            encryptor.encrypt(pt_zero, ct_zero);
            evaluator.add_inplace(ct_fib, ct_zero);
        }
        
        int noise_after_fib = decryptor.invariant_noise_budget(ct_fib);
        int change = noise_after_fib - noise_before_fib;
        
        cout << "  " << left << setw(10) << (i+1)
             << setw(10) << zans_ops
             << setw(15) << noise_before_fib
             << setw(15) << noise_after_fib
             << setw(15) << (change >= 0 ? "+" + to_string(change) : to_string(change));
        
        if (change > 0) cout << " ⬆ INCREASE!";
        cout << "\n";
    }
    
    // ==========================================
    // SUMMARY
    // ==========================================
    print_header("SELF-BOOTSTRAPPING V1 - SUMMARY");
    cout << "  Key question: Does ZANS sequence at φ-points\n";
    cout << "  increase noise budget?\n\n";
    cout << "  If any test shows +change, we have evidence\n";
    cout << "  that self-bootstrapping is possible.\n\n";
    
    return 0;
}

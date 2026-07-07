/*
 * ZANS MULTIPLICATION V2
 * 
 * Self-referential multiplication analysis.
 * 
 * Tests:
 * 1. ct × ct (squaring) - pure self-reference, most expensive
 * 2. ct × Enc(1) (ZANS-M) - multiplicative identity, hypothesis: cheaper
 * 3. ct × Enc(φ) - golden ratio multiplier
 * 4. ct × Enc(1/φ) - inverse golden ratio
 * 5. ct × Enc(0) - zero multiplication
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
    cout << "\n" << string(65, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(65, '=') << "\n\n";
}

int main() {
    print_header("ZANS MULTIPLICATION V2 - Self-Reference Analysis");
    
    cout << "  φ = " << fixed << setprecision(10) << PHI << "\n";
    cout << "  1/φ = " << (1.0/PHI) << "\n\n";
    
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
    
    // Fresh noise budget reference
    Plaintext pt_test("1");
    Ciphertext ct_fresh(context);
    encryptor.encrypt(pt_test, ct_fresh);
    int fresh_noise = decryptor.invariant_noise_budget(ct_fresh);
    cout << "  Fresh ciphertext noise budget: " << fresh_noise << " bits\n\n";
    
    // ==========================================
    // TEST 1: ct × ct (SQUARING) - Pure self-reference
    // ==========================================
    print_header("TEST 1: ct × ct (SQUARING) - Pure Self-Reference");
    cout << "  Ito ang pinaka-expensive: ciphertext × sarili\n\n";
    
    Ciphertext ct_sq(context);
    encryptor.encrypt(pt_test, ct_sq);
    int sq_start = decryptor.invariant_noise_budget(ct_sq);
    
    cout << "  Start: " << sq_start << " bits\n";
    
    int prev = sq_start;
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct_sq;
        evaluator.multiply_inplace(ct_sq, ct_copy);
        evaluator.relinearize_inplace(ct_sq, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct_sq);
        int drop = prev - noise;
        double ratio = (prev > 0) ? (double)noise / prev : 0;
        
        cout << "  Step " << i << ": " << noise << " bits (drop=" << drop 
             << ", ratio=" << fixed << setprecision(4) << ratio << ")";
        
        if (abs(ratio - 1.0/PHI) < 0.03) cout << " ≈ 1/φ ✨";
        if (noise == 0) { cout << " DEAD"; break; }
        cout << "\n";
        prev = noise;
    }
    
    int sq_end = decryptor.invariant_noise_budget(ct_sq);
    int sq_steps = 0;
    // Count steps survived
    {
        Ciphertext ct_tmp(context);
        encryptor.encrypt(pt_test, ct_tmp);
        for (int i = 0; i < 6; i++) {
            Ciphertext ct_copy = ct_tmp;
            evaluator.multiply_inplace(ct_tmp, ct_copy);
            evaluator.relinearize_inplace(ct_tmp, relin_keys);
            if (decryptor.invariant_noise_budget(ct_tmp) > 0) sq_steps++;
            else break;
        }
    }
    
    // ==========================================
    // TEST 2: ct × Enc(1) - ZANS-M (Multiplicative Identity)
    // ==========================================
    print_header("TEST 2: ct × Enc(1) - ZANS-M (Multiplicative Identity)");
    cout << "  Hypothesis: Multiplying by fresh Enc(1) is cheaper\n";
    cout << "  than squaring, because Enc(1) has fresh low noise.\n\n";
    
    Ciphertext ct_zm(context);
    encryptor.encrypt(pt_test, ct_zm);
    int zm_start = decryptor.invariant_noise_budget(ct_zm);
    
    Plaintext pt_one("1");
    Ciphertext ct_one(context);
    
    cout << "  Start: " << zm_start << " bits\n";
    
    prev = zm_start;
    for (int i = 1; i <= 6; i++) {
        encryptor.encrypt(pt_one, ct_one);  // Fresh Enc(1) every time
        evaluator.multiply_inplace(ct_zm, ct_one);
        evaluator.relinearize_inplace(ct_zm, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct_zm);
        int drop = prev - noise;
        double ratio = (prev > 0) ? (double)noise / prev : 0;
        
        cout << "  Step " << i << ": " << noise << " bits (drop=" << drop 
             << ", ratio=" << fixed << setprecision(4) << ratio << ")";
        
        if (abs(ratio - 1.0/PHI) < 0.03) cout << " ≈ 1/φ ✨";
        if (noise == 0) { cout << " DEAD"; break; }
        cout << "\n";
        prev = noise;
    }
    
    int zm_end = decryptor.invariant_noise_budget(ct_zm);
    
    // ==========================================
    // TEST 3: ct × Enc(φ) - Golden Ratio Multiplier
    // ==========================================
    print_header("TEST 3: ct × Enc(φ) - Golden Ratio Multiplier");
    cout << "  Testing if φ multiplier has different noise cost\n\n";
    
    Ciphertext ct_phi(context);
    encryptor.encrypt(pt_test, ct_phi);
    int phi_start = decryptor.invariant_noise_budget(ct_phi);
    
    // φ ≈ 1618/1000, but we need integer: use 1618
    Plaintext pt_phi("1618");
    Ciphertext ct_phi_mult(context);
    
    cout << "  Start: " << phi_start << " bits\n";
    
    prev = phi_start;
    for (int i = 1; i <= 6; i++) {
        encryptor.encrypt(pt_phi, ct_phi_mult);
        evaluator.multiply_inplace(ct_phi, ct_phi_mult);
        evaluator.relinearize_inplace(ct_phi, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct_phi);
        int drop = prev - noise;
        double ratio = (prev > 0) ? (double)noise / prev : 0;
        
        cout << "  Step " << i << ": " << noise << " bits (drop=" << drop 
             << ", ratio=" << fixed << setprecision(4) << ratio << ")";
        
        if (abs(ratio - 1.0/PHI) < 0.03) cout << " ≈ 1/φ ✨";
        if (noise == 0) { cout << " DEAD"; break; }
        cout << "\n";
        prev = noise;
    }
    
    int phi_end = decryptor.invariant_noise_budget(ct_phi);
    
    // ==========================================
    // TEST 4: ct × Enc(1/φ) - Inverse Golden Ratio
    // ==========================================
    print_header("TEST 4: ct × Enc(1/φ) - Inverse Golden Ratio");
    cout << "  Testing if 1/φ multiplier has different noise cost\n\n";
    
    Ciphertext ct_invphi(context);
    encryptor.encrypt(pt_test, ct_invphi);
    int invphi_start = decryptor.invariant_noise_budget(ct_invphi);
    
    // 1/φ ≈ 618/1000, use 618
    Plaintext pt_invphi("618");
    Ciphertext ct_invphi_mult(context);
    
    cout << "  Start: " << invphi_start << " bits\n";
    
    prev = invphi_start;
    for (int i = 1; i <= 6; i++) {
        encryptor.encrypt(pt_invphi, ct_invphi_mult);
        evaluator.multiply_inplace(ct_invphi, ct_invphi_mult);
        evaluator.relinearize_inplace(ct_invphi, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct_invphi);
        int drop = prev - noise;
        double ratio = (prev > 0) ? (double)noise / prev : 0;
        
        cout << "  Step " << i << ": " << noise << " bits (drop=" << drop 
             << ", ratio=" << fixed << setprecision(4) << ratio << ")";
        
        if (abs(ratio - 1.0/PHI) < 0.03) cout << " ≈ 1/φ ✨";
        if (noise == 0) { cout << " DEAD"; break; }
        cout << "\n";
        prev = noise;
    }
    
    // ==========================================
    // TEST 5: ct × Enc(0) - Zero Multiplication
    // ==========================================
    print_header("TEST 5: ct × Enc(0) - Zero Multiplication");
    cout << "  Multiplying by zero: value should be 0\n\n";
    
    Ciphertext ct_zero_test(context);
    encryptor.encrypt(pt_test, ct_zero_test);
    int zero_start = decryptor.invariant_noise_budget(ct_zero_test);
    
    Plaintext pt_zero("0");
    Ciphertext ct_zero_mult(context);
    encryptor.encrypt(pt_zero, ct_zero_mult);
    
    evaluator.multiply_inplace(ct_zero_test, ct_zero_mult);
    evaluator.relinearize_inplace(ct_zero_test, relin_keys);
    
    int zero_end = decryptor.invariant_noise_budget(ct_zero_test);
    
    Plaintext pt_result;
    decryptor.decrypt(ct_zero_test, pt_result);
    
    cout << "  Start: " << zero_start << " bits\n";
    cout << "  After ×0: " << zero_end << " bits\n";
    cout << "  Drop: " << (zero_start - zero_end) << " bits\n";
    cout << "  Result: " << pt_result.to_string() << "\n";
    
    // ==========================================
    // HEAD-TO-HEAD SUMMARY
    // ==========================================
    print_header("HEAD-TO-HEAD COMPARISON (5 multiplications each)");
    
    cout << "  " << left << setw(25) << "Method"
         << setw(12) << "Start"
         << setw(12) << "End"
         << setw(12) << "Drift"
         << "1/φ hits\n";
    cout << "  " << string(70, '-') << "\n";
    
    cout << "  " << left << setw(25) << "Squaring (ct × ct)"
         << setw(12) << sq_start
         << setw(12) << sq_end
         << setw(12) << (sq_start - sq_end) << "\n";
    
    cout << "  " << left << setw(25) << "ZANS-M (ct × Enc(1))"
         << setw(12) << zm_start
         << setw(12) << zm_end
         << setw(12) << (zm_start - zm_end) << "\n";
    
    cout << "  " << left << setw(25) << "× Enc(φ)"
         << setw(12) << phi_start
         << setw(12) << phi_end
         << setw(12) << (phi_start - phi_end) << "\n";
    
    cout << "  " << left << setw(25) << "× Enc(1/φ)"
         << setw(12) << invphi_start
         << setw(12) << decryptor.invariant_noise_budget(ct_invphi)
         << setw(12) << (invphi_start - decryptor.invariant_noise_budget(ct_invphi)) << "\n";
    
    cout << "  " << left << setw(25) << "× Enc(0)"
         << setw(12) << zero_start
         << setw(12) << zero_end
         << setw(12) << (zero_start - zero_end) << "\n";
    
    // ==========================================
    // ANALYSIS
    // ==========================================
    print_header("SELF-REFERENCE ANALYSIS");
    
    cout << "  Self-reference in mathematics:\n";
    cout << "  - f(x) = x + 0  → additive identity (ZANS)\n";
    cout << "  - f(x) = x × 1  → multiplicative identity (ZANS-M)\n";
    cout << "  - f(x) = x × x  → pure self-reference (squaring)\n";
    cout << "  - φ = 1 + 1/φ   → golden ratio self-reference\n\n";
    
    cout << "  Hypothesis:\n";
    cout << "  - ZANS-M (×Enc(1)) should be cheaper than squaring\n";
    cout << "  - The φ ratio (1/φ ≈ 0.618) may appear in optimal noise decay\n";
    cout << "  - Self-reference noise growth follows Fibonacci because\n";
    cout << "    the recursion ct = ct × ct mirrors φ's self-definition\n\n";
    
    return 0;
}

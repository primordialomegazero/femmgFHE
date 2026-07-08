#include "seal/seal.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace seal;
using namespace std;

const double PHI = 1.6180339887498948482;
const double PHI_INV = 0.6180339887498948482;

// Decomposed multiply: ct × ct2 with ZANS before and after
void decomposed_multiply(Ciphertext &ct1, const Ciphertext &ct2,
                          Evaluator &evaluator, Encryptor &encryptor,
                          RelinKeys &relin_keys, const SEALContext &context,
                          Decryptor &decryptor, int zans_strength = 5) {
    // ZANS before
    Plaintext pt_zero("0");
    for (int i = 0; i < zans_strength; i++) {
        Ciphertext ct_zero(context, ct1.parms_id());
        encryptor.encrypt(pt_zero, ct_zero);
        evaluator.add_inplace(ct1, ct_zero);
    }
    
    // Multiply
    evaluator.multiply_inplace(ct1, ct2);
    evaluator.relinearize_inplace(ct1, relin_keys);
    
    // ZANS after
    for (int i = 0; i < zans_strength * 2; i++) {
        Ciphertext ct_zero(context, ct1.parms_id());
        encryptor.encrypt(pt_zero, ct_zero);
        evaluator.add_inplace(ct1, ct_zero);
    }
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  CIPHERTEXT DECOMPOSITION                     ║\n";
    cout << "║  Binary Exponentiation + ZANS                ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

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

    Plaintext pt_val("42");
    Plaintext pt_zero("0");

    // === TEST 1: Standard Squaring (baseline) ===
    cout << "=== TEST 1: Standard Squaring ===\n";
    Ciphertext ct_std(context);
    encryptor.encrypt(pt_val, ct_std);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct_std) << " bits\n";
    
    int std_steps = 0;
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct_std;
        evaluator.multiply_inplace(ct_std, ct_copy);
        evaluator.relinearize_inplace(ct_std, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct_std);
        cout << "  S" << i << ": " << noise;
        if (noise > 0) std_steps++; 
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "  Survived: " << std_steps << "\n\n";

    // === TEST 2: ZANS=10 ===
    cout << "=== TEST 2: Decomposed Multiply (ZANS=10) ===\n";
    Ciphertext ct10(context);
    encryptor.encrypt(pt_val, ct10);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct10) << " bits\n";
    
    int steps10 = 0;
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct10;
        decomposed_multiply(ct10, ct_copy, evaluator, encryptor, relin_keys, context, decryptor, 10);
        int noise = decryptor.invariant_noise_budget(ct10);
        cout << "  S" << i << ": " << noise;
        if (noise > 0) steps10++;
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "  Survived: " << steps10 << "\n\n";

    // === TEST 3: ZANS=34 (Fibonacci) ===
    cout << "=== TEST 3: Decomposed Multiply (ZANS=34) ===\n";
    Ciphertext ct34(context);
    encryptor.encrypt(pt_val, ct34);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct34) << " bits\n";
    
    int steps34 = 0;
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct34;
        decomposed_multiply(ct34, ct_copy, evaluator, encryptor, relin_keys, context, decryptor, 34);
        int noise = decryptor.invariant_noise_budget(ct34);
        cout << "  S" << i << ": " << noise;
        if (noise > 0) steps34++;
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "  Survived: " << steps34 << "\n\n";

    // === TEST 4: ZANS=100 (Aggressive) ===
    cout << "=== TEST 4: Decomposed Multiply (ZANS=100) ===\n";
    Ciphertext ct100(context);
    encryptor.encrypt(pt_val, ct100);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct100) << " bits\n";
    
    int steps100 = 0;
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct100;
        decomposed_multiply(ct100, ct_copy, evaluator, encryptor, relin_keys, context, decryptor, 100);
        int noise = decryptor.invariant_noise_budget(ct100);
        cout << "  S" << i << ": " << noise;
        if (noise > 0) steps100++;
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "  Survived: " << steps100 << "\n\n";

    // === TEST 5: Progressive ZANS ===
    cout << "=== TEST 5: Progressive ZANS (φ-scaled) ===\n";
    Ciphertext ct_phi(context);
    encryptor.encrypt(pt_val, ct_phi);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct_phi) << " bits\n";
    
    vector<int> zans_schedule = {5, 8, 13, 21, 34, 55}; // Fibonacci
    int steps_phi = 0;
    for (int i = 1; i <= 6; i++) {
        int zans = zans_schedule[i-1];
        Ciphertext ct_copy = ct_phi;
        decomposed_multiply(ct_phi, ct_copy, evaluator, encryptor, relin_keys, context, decryptor, zans);
        int noise = decryptor.invariant_noise_budget(ct_phi);
        cout << "  S" << i << "(ZANS=" << zans << "): " << noise;
        if (noise > 0) steps_phi++;
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "  Survived: " << steps_phi << "\n\n";

    // === COMPARISON ===
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  COMPARISON                                   ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  Standard:   " << std_steps << " steps\n";
    cout << "║  ZANS=10:    " << steps10 << " steps\n";
    cout << "║  ZANS=34:    " << steps34 << " steps\n";
    cout << "║  ZANS=100:   " << steps100 << " steps\n";
    cout << "║  Progressive: " << steps_phi << " steps\n";
    
    int best = max({std_steps, steps10, steps34, steps100, steps_phi});
    if (best > std_steps) {
        cout << "║  ✅ DECOMPOSITION EXTENDS DEPTH!              ║\n";
    } else {
        cout << "║  ⚠️ Decomposition = Standard depth           ║\n";
    }
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}

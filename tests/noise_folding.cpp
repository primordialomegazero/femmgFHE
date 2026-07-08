#include "seal/seal.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace seal;
using namespace std;

const double PHI = 1.6180339887498948482;
const double PHI_INV = 0.6180339887498948482;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  NOISE FOLDING via KEY SWITCHING              ║\n";
    cout << "║  Multiple relinearizations after multiply    ║\n";
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
    
    // Create a SECOND set of relin keys for folding
    KeyGenerator keygen2(context);
    SecretKey sk2 = keygen2.secret_key();
    RelinKeys relin_keys2;
    keygen2.create_relin_keys(relin_keys2);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    Decryptor decryptor2(context, sk2);

    Plaintext pt_val("42");

    // === TEST 1: Standard ===
    cout << "=== TEST 1: Standard (1 relinearization) ===\n";
    Ciphertext ct1(context);
    encryptor.encrypt(pt_val, ct1);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct1) << " bits\n";
    
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct1;
        evaluator.multiply_inplace(ct1, ct_copy);
        evaluator.relinearize_inplace(ct1, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct1);
        cout << "  Step " << i << ": " << noise;
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "\n";

    // === TEST 2: Double Relinearization ===
    cout << "=== TEST 2: Double Relinearization (fold) ===\n";
    Ciphertext ct2(context);
    encryptor.encrypt(pt_val, ct2);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct2) << " bits\n";
    
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct2;
        evaluator.multiply_inplace(ct2, ct_copy);
        evaluator.relinearize_inplace(ct2, relin_keys);
        // SECOND relinearization — noise folding attempt
        evaluator.relinearize_inplace(ct2, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct2);
        cout << "  Step " << i << ": " << noise << " (double relin)";
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "\n";

    // === TEST 3: Triple Relinearization ===
    cout << "=== TEST 3: Triple Relinearization ===\n";
    Ciphertext ct3(context);
    encryptor.encrypt(pt_val, ct3);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct3) << " bits\n";
    
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct3;
        evaluator.multiply_inplace(ct3, ct_copy);
        evaluator.relinearize_inplace(ct3, relin_keys);
        evaluator.relinearize_inplace(ct3, relin_keys);
        evaluator.relinearize_inplace(ct3, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct3);
        cout << "  Step " << i << ": " << noise << " (triple relin)";
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "\n";

    // === TEST 4: Cross-key Folding ===
    cout << "=== TEST 4: Cross-Key Folding ===\n";
    Ciphertext ct4(context);
    encryptor.encrypt(pt_val, ct4);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct4) << " bits\n";
    
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct4;
        evaluator.multiply_inplace(ct4, ct_copy);
        evaluator.relinearize_inplace(ct4, relin_keys);
        // Switch to second key then back
        evaluator.relinearize_inplace(ct4, relin_keys2);
        evaluator.relinearize_inplace(ct4, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct4);
        cout << "  Step " << i << ": " << noise << " (cross-key)";
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  φ = 1 + 1/φ → Key Switching = Regeneration  ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}

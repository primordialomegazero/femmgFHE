/*
 * FIX TENSOR: I-account ang scaling factor
 * Ang BFV multiplication ay may Δ = q/t scaling
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;
using namespace std;

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  FIX TENSOR: ACCOUNT SCALING FACTOR\n";
    cout << "========================================\n\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 30));
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

    // Test values: 12 × 34 = 408
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

    // ========================================
    // APPROACH 1: Get the scaling factor
    // ========================================
    cout << "--- APPROACH 1: Get Scaling Factor ---\n";
    
    Plaintext plain_1("1");
    Ciphertext ct_1;
    encryptor.encrypt(plain_1, ct_1);
    
    Ciphertext ct_scaled = ct_1;
    evaluator.multiply_inplace(ct_scaled, ct_1);
    evaluator.relinearize_inplace(ct_scaled, relin_keys);
    
    Plaintext result_scaled;
    decryptor.decrypt(ct_scaled, result_scaled);
    cout << "  1 × 1 = " << result_scaled.to_string() << " (scaling factor)\n\n";

    // ========================================
    // APPROACH 2: Tensor Decomp with scaling correction
    // ========================================
    cout << "--- APPROACH 2: Tensor Decomp with Scaling ---\n";
    
    Ciphertext term1 = ct_6;
    evaluator.multiply_inplace(term1, ct_17);
    evaluator.relinearize_inplace(term1, relin_keys);
    
    Ciphertext term2 = ct_6;
    evaluator.multiply_inplace(term2, ct_17);
    evaluator.relinearize_inplace(term2, relin_keys);
    
    Ciphertext term3 = ct_6;
    evaluator.multiply_inplace(term3, ct_17);
    evaluator.relinearize_inplace(term3, relin_keys);
    
    Ciphertext term4 = ct_6;
    evaluator.multiply_inplace(term4, ct_17);
    evaluator.relinearize_inplace(term4, relin_keys);

    Ciphertext sum_scaled = term1;
    evaluator.add_inplace(sum_scaled, term2);
    evaluator.add_inplace(sum_scaled, term3);
    evaluator.add_inplace(sum_scaled, term4);
    
    print_result(decryptor, sum_scaled, "Sum of 4 terms (scaled)");
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_scaled) << " bits\n\n";

    // ========================================
    // APPROACH 3: Direct Multiplication
    // ========================================
    cout << "--- APPROACH 3: Direct Multiplication ---\n";
    
    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);
    
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    
    print_result(decryptor, ct_direct, "Direct 12×34");
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_direct) << " bits\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Tensor Decomp (scaled): " << decryptor.invariant_noise_budget(sum_scaled) << " bits\n";
    cout << "  Direct 12×34:           " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";
    cout << "========================================\n\n";

    return 0;
}

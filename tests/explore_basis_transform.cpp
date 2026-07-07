/*
 * PHASE 3: POLYNOMIAL BASIS TRANSFORMATION
 * Iba't ibang representation ng ciphertext
 * Hanapin ang basis na "friendly" sa multiplication
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;

void print_noise(const char* label, int noise) {
    std::cout << "  " << label << ": " << noise << " bits\n";
}

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  PHASE 3: POLYNOMIAL BASIS TRANSFORM\n";
    std::cout << "  Hanapin ang basis na friendly sa mult\n";
    std::cout << "========================================\n\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 20));
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

    // Pre-compute Enc(0)
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    // Test values
    Plaintext plain_a("7");
    Plaintext plain_b("11");
    Ciphertext ct_a, ct_b;
    encryptor.encrypt(plain_a, ct_a);
    encryptor.encrypt(plain_b, ct_b);

    int start_noise = decryptor.invariant_noise_budget(ct_a);
    print_noise("Start (Enc(7))", start_noise);

    // ========================================
    // Standard basis (x^N + 1)
    // ========================================
    std::cout << "\n--- Standard Basis (x^N + 1) ---\n";
    
    Ciphertext ct_std = ct_a;
    evaluator.multiply_inplace(ct_std, ct_b);
    evaluator.relinearize_inplace(ct_std, relin_keys);
    
    int noise_std = decryptor.invariant_noise_budget(ct_std);
    print_noise("Standard basis multiply", noise_std);

    // ========================================
    // Transformed basis: multiply by a constant
    // Idea: I-transform ang ciphertext sa ibang basis
    // where multiplication is cheaper
    // ========================================
    std::cout << "\n--- Transformed Basis (Multiply by constant) ---\n";
    
    // I-transform ang ct_a at ct_b
    Ciphertext ct_a_trans = ct_a;
    Ciphertext ct_b_trans = ct_b;
    
    // Multiply by a constant to change basis
    Plaintext const_2("2");
    Plaintext const_3("3");
    
    evaluator.multiply_plain_inplace(ct_a_trans, const_2);
    evaluator.multiply_plain_inplace(ct_b_trans, const_3);
    
    // Multiply in transformed basis
    evaluator.multiply_inplace(ct_a_trans, ct_b_trans);
    evaluator.relinearize_inplace(ct_a_trans, relin_keys);
    
    int noise_trans = decryptor.invariant_noise_budget(ct_a_trans);
    print_noise("Transformed basis multiply", noise_trans);

    // ========================================
    // Sparse basis: I-zero out some coefficients
    // ========================================
    std::cout << "\n--- Sparse Basis (Zero out coefficients) ---\n";
    
    // I-extract ang first few coefficients
    // (Simplified: just use plaintext multiplication)
    Ciphertext ct_sparse = ct_a;
    evaluator.multiply_plain_inplace(ct_sparse, Plaintext("1"));
    
    int noise_sparse = decryptor.invariant_noise_budget(ct_sparse);
    print_noise("Sparse basis (simplified)", noise_sparse);

    // ========================================
    // SUMMARY
    // ========================================
    std::cout << "\n========================================\n";
    std::cout << "  PHASE 3 SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "  Standard basis:     " << noise_std << " bits\n";
    std::cout << "  Transformed basis:  " << noise_trans << " bits\n";
    std::cout << "  Sparse basis:       " << noise_sparse << " bits\n";
    std::cout << "========================================\n\n";

    return 0;
}

/*
 * FIX TENSOR: I-scale ang mga terms
 * I-account ang scaling factor ng SEAL
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    std::cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  FIX TENSOR: SCALED APPROACH\n";
    std::cout << "========================================\n\n";

    // Setup SEAL na may larger plaintext modulus (30 bits)
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 30));  // 30 bits instead of 20
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

    // Pre-compute Enc(0)
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    // ========================================
    // APPROACH 1: Tensor Decomposition with 30-bit modulus
    // ========================================
    std::cout << "--- APPROACH 1: Tensor Decomp (30-bit modulus) ---\n";
    
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

    // Sum all terms
    Ciphertext sum_30bit = term1;
    evaluator.add_inplace(sum_30bit, term2);
    evaluator.add_inplace(sum_30bit, term3);
    evaluator.add_inplace(sum_30bit, term4);
    
    print_result(decryptor, sum_30bit, "Sum of 4 terms (30-bit)");
    std::cout << "  Noise: " << decryptor.invariant_noise_budget(sum_30bit) << " bits\n\n";

    // ========================================
    // APPROACH 2: Direct multiplication for comparison
    // ========================================
    std::cout << "--- APPROACH 2: Direct Multiplication (30-bit) ---\n";
    
    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);
    
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    
    print_result(decryptor, ct_direct, "Direct 12×34 (30-bit)");
    std::cout << "  Noise: " << decryptor.invariant_noise_budget(ct_direct) << " bits\n\n";

    // ========================================
    // APPROACH 3: I-scale ang result pabalik
    // ========================================
    std::cout << "--- APPROACH 3: I-scale pabalik ang result ---\n";
    std::cout << "  (Kung may scaling factor, i-divide)\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    std::cout << "========================================\n";
    std::cout << "  SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "  30-bit modulus: " << decryptor.invariant_noise_budget(sum_30bit) << " bits\n";
    std::cout << "  Direct 12×34:   " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";
    std::cout << "========================================\n\n";

    return 0;
}

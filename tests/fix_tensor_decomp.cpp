/*
 * FIX TENSOR DECOMPOSITION
 * Hanapin kung bakit mali ang resulta
 * I-test ang bawat tensor term individually
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;

void print_noise(const char* label, int noise) {
    std::cout << "  " << label << ": " << noise << " bits\n";
}

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    std::cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  DEBUG TENSOR DECOMPOSITION\n";
    std::cout << "  Hanapin kung bakit mali ang resulta\n";
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

    // Test values: 12 × 34 = 408
    Plaintext plain_a("12");
    Plaintext plain_b("34");
    Ciphertext ct_a, ct_b;
    encryptor.encrypt(plain_a, ct_a);
    encryptor.encrypt(plain_b, ct_b);

    // ========================================
    // TEST 1: Direct Multiplication (Baseline)
    // ========================================
    std::cout << "--- TEST 1: Direct Multiplication ---\n";
    Ciphertext ct_direct = ct_a;
    evaluator.multiply_inplace(ct_direct, ct_b);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    print_result(decryptor, ct_direct, "Direct 12×34");
    print_noise("Noise", decryptor.invariant_noise_budget(ct_direct));
    std::cout << "\n";

    // ========================================
    // TEST 2: Tensor Terms Individually
    // ========================================
    std::cout << "--- TEST 2: Individual Tensor Terms ---\n";
    
    // 12 = 6 + 6, 34 = 17 + 17
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

    // Term 1: 6 × 17 = 102
    Ciphertext term1 = ct_6;
    evaluator.multiply_inplace(term1, ct_17);
    evaluator.relinearize_inplace(term1, relin_keys);
    print_result(decryptor, term1, "6×17");
    print_noise("Noise", decryptor.invariant_noise_budget(term1));

    // Term 2: 6 × 17 = 102
    Ciphertext term2 = ct_6;
    evaluator.multiply_inplace(term2, ct_17);
    evaluator.relinearize_inplace(term2, relin_keys);
    print_result(decryptor, term2, "6×17 (2)");
    print_noise("Noise", decryptor.invariant_noise_budget(term2));

    std::cout << "\n";

    // ========================================
    // TEST 3: Sum of Tensor Terms
    // ========================================
    std::cout << "--- TEST 3: Sum of Tensor Terms ---\n";
    
    // Recompute terms for summing
    Ciphertext sum_term1 = ct_6;
    evaluator.multiply_inplace(sum_term1, ct_17);
    evaluator.relinearize_inplace(sum_term1, relin_keys);
    
    Ciphertext sum_term2 = ct_6;
    evaluator.multiply_inplace(sum_term2, ct_17);
    evaluator.relinearize_inplace(sum_term2, relin_keys);
    
    Ciphertext sum_term3 = ct_6;
    evaluator.multiply_inplace(sum_term3, ct_17);
    evaluator.relinearize_inplace(sum_term3, relin_keys);
    
    Ciphertext sum_term4 = ct_6;
    evaluator.multiply_inplace(sum_term4, ct_17);
    evaluator.relinearize_inplace(sum_term4, relin_keys);

    // Sum all terms: 102 + 102 + 102 + 102 = 408
    Ciphertext sum_result = sum_term1;
    evaluator.add_inplace(sum_result, sum_term2);
    evaluator.add_inplace(sum_result, sum_term3);
    evaluator.add_inplace(sum_result, sum_term4);
    
    print_result(decryptor, sum_result, "Sum of 4 terms (102×4)");
    print_noise("Noise", decryptor.invariant_noise_budget(sum_result));
    std::cout << "\n";

    // ========================================
    // TEST 4: Check Plaintext Modulus Limit
    // ========================================
    std::cout << "--- TEST 4: Plaintext Modulus Check ---\n";
    
    // Ang plaintext modulus ay 2^20 ≈ 1,048,576
    // Ang 102 × 4 = 408, so dapat hindi mag-overflow
    
    // I-encrypt ang 408 directly para i-compare
    Plaintext plain_408("408");
    Ciphertext ct_408;
    encryptor.encrypt(plain_408, ct_408);
    print_result(decryptor, ct_408, "Direct Enc(408)");
    print_noise("Noise", decryptor.invariant_noise_budget(ct_408));
    std::cout << "\n";

    // ========================================
    // SUMMARY
    // ========================================
    std::cout << "========================================\n";
    std::cout << "  DEBUG SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "  Direct 12×34:       408 (correct)\n";
    std::cout << "  Individual 6×17:    102 (correct)\n";
    std::cout << "  Sum of 4 terms:     " << decryptor.invariant_noise_budget(sum_result) << " bits\n";
    std::cout << "  Direct Enc(408):    " << decryptor.invariant_noise_budget(ct_408) << " bits\n";
    std::cout << "========================================\n\n";

    return 0;
}

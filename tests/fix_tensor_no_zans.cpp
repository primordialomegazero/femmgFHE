/*
 * FIX TENSOR DECOMPOSITION
 * I-remove ang ZANS sa tensor terms
 * I-sum lang ang terms nang walang ZANS
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
    std::cout << "  FIX TENSOR: REMOVE ZANS\n";
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

    // Test values: 12 × 34 = 408
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

    // ========================================
    // APPROACH 1: Sum without ZANS
    // ========================================
    std::cout << "--- APPROACH 1: Sum without ZANS ---\n";
    
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

    // Sum without ZANS
    Ciphertext sum_no_zans = term1;
    evaluator.add_inplace(sum_no_zans, term2);
    evaluator.add_inplace(sum_no_zans, term3);
    evaluator.add_inplace(sum_no_zans, term4);
    
    print_result(decryptor, sum_no_zans, "Sum of 4 terms (no ZANS)");
    std::cout << "  Noise: " << decryptor.invariant_noise_budget(sum_no_zans) << " bits\n\n";

    // ========================================
    // APPROACH 2: Sum with ZANS only at the end
    // ========================================
    std::cout << "--- APPROACH 2: Sum with ZANS at end ---\n";
    
    // Pre-compute Enc(0)
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    Ciphertext sum_zans_end = term1;
    evaluator.add_inplace(sum_zans_end, term2);
    evaluator.add_inplace(sum_zans_end, term3);
    evaluator.add_inplace(sum_zans_end, term4);
    
    // Apply ZANS only at the end
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(sum_zans_end, enc_zero);
    }
    
    print_result(decryptor, sum_zans_end, "Sum + 1000 ZANS at end");
    std::cout << "  Noise: " << decryptor.invariant_noise_budget(sum_zans_end) << " bits\n\n";

    // ========================================
    // APPROACH 3: Re-encrypt after sum (para ma-reset ang noise)
    // ========================================
    std::cout << "--- APPROACH 3: Sum then re-encrypt ---\n";
    
    // I-decrypt muna? Hindi pwedeng i-decrypt kasi may secret key
    // Alternative: I-multiply ng 1 para ma-reset ang noise
    Ciphertext sum_reset = term1;
    evaluator.add_inplace(sum_reset, term2);
    evaluator.add_inplace(sum_reset, term3);
    evaluator.add_inplace(sum_reset, term4);
    
    // Multiply by 1 to "reset" noise structure
    Plaintext plain_1("1");
    evaluator.multiply_plain_inplace(sum_reset, plain_1);
    
    print_result(decryptor, sum_reset, "Sum + multiply by 1");
    std::cout << "  Noise: " << decryptor.invariant_noise_budget(sum_reset) << " bits\n\n";

    // ========================================
    // APPROACH 4: Use larger plaintext modulus
    // ========================================
    std::cout << "--- APPROACH 4: Larger plaintext modulus ---\n";
    std::cout << "  (Kailangan i-reconfigure ang SEAL para dito)\n";
    std::cout << "  Suggestion: Gumamit ng 30-bit plaintext modulus\n";
    std::cout << "  para maiwasan ang overflow sa tensor terms\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    std::cout << "========================================\n";
    std::cout << "  SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "  Sum without ZANS:      " << decryptor.invariant_noise_budget(sum_no_zans) << " bits\n";
    std::cout << "  Sum + ZANS at end:     " << decryptor.invariant_noise_budget(sum_zans_end) << " bits\n";
    std::cout << "  Sum + multiply by 1:   " << decryptor.invariant_noise_budget(sum_reset) << " bits\n";
    std::cout << "========================================\n\n";

    return 0;
}

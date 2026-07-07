/*
 * EXPLORE MULTIPLICATION ANCHORS
 * Hanapin kung may "zero" para sa multiplication
 * I-test: Enc(1), Enc(phi), Enc(phi^-1)
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace seal;

void print_noise(const char* label, int noise) {
    std::cout << "  " << label << ": " << noise << " bits\n";
}

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  EXPLORE MULTIPLICATION ANCHORS\n";
    std::cout << "  Hanapin ang anchor for multiplication\n";
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

    // Pre-compute Enc(0) for ZANS
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    // I-encrypt ang test values
    Plaintext plain_5("5");
    Ciphertext ct5;
    encryptor.encrypt(plain_5, ct5);

    // ========================================
    // TEST 1: Multiplication by Enc(1)
    // ========================================
    std::cout << "TEST 1: Multiplication by Enc(1)\n";
    std::cout << "  (Kung ang Enc(1) ay anchor for multiplication)\n\n";

    Plaintext plain_1("1");
    Ciphertext enc1;
    encryptor.encrypt(plain_1, enc1);

    int start_noise = decryptor.invariant_noise_budget(ct5);
    print_noise("Start (Enc(5))", start_noise);

    // Multiply: Enc(5) * Enc(1)
    Ciphertext ct5_x_1 = ct5;
    evaluator.multiply_inplace(ct5_x_1, enc1);
    evaluator.relinearize_inplace(ct5_x_1, relin_keys);

    int noise_after_mul = decryptor.invariant_noise_budget(ct5_x_1);
    print_noise("After x Enc(1)", noise_after_mul);

    // Apply ZANS after multiplication
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct5_x_1, enc_zero);
    }
    int noise_after_zans = decryptor.invariant_noise_budget(ct5_x_1);
    print_noise("After 1000 ZANS", noise_after_zans);

    std::cout << "\n  Result: ";
    Plaintext result1;
    decryptor.decrypt(ct5_x_1, result1);
    std::cout << result1.to_string() << "\n\n";

    // ========================================
    // TEST 2: Multiplication by Enc(phi)
    // ========================================
    std::cout << "TEST 2: Multiplication by Enc(phi)\n";
    std::cout << "  (Golden ratio anchor - speculative)\n\n";

    // phi ≈ 1.618, so phi * 1000 = 1618
    Plaintext plain_phi("1618");
    Ciphertext enc_phi;
    encryptor.encrypt(plain_phi, enc_phi);

    // Re-encrypt 5
    Ciphertext ct5_phi;
    encryptor.encrypt(plain_5, ct5_phi);

    // Multiply: Enc(5) * Enc(phi)
    evaluator.multiply_inplace(ct5_phi, enc_phi);
    evaluator.relinearize_inplace(ct5_phi, relin_keys);

    int noise_phi = decryptor.invariant_noise_budget(ct5_phi);
    print_noise("After x Enc(phi)", noise_phi);

    // Apply ZANS
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct5_phi, enc_zero);
    }
    int noise_phi_zans = decryptor.invariant_noise_budget(ct5_phi);
    print_noise("After 1000 ZANS", noise_phi_zans);

    std::cout << "\n  Result: ";
    Plaintext result_phi;
    decryptor.decrypt(ct5_phi, result_phi);
    std::cout << result_phi.to_string() << " (5 x phi x 1000)\n\n";

    // ========================================
    // TEST 3: Multiplication by Enc(phi^-1)
    // ========================================
    std::cout << "TEST 3: Multiplication by Enc(phi^-1)\n";
    std::cout << "  (Inverse golden ratio anchor)\n\n";

    // phi^-1 ≈ 0.618, so phi^-1 * 1000 = 618
    Plaintext plain_phi_inv("618");
    Ciphertext enc_phi_inv;
    encryptor.encrypt(plain_phi_inv, enc_phi_inv);

    Ciphertext ct5_phi_inv;
    encryptor.encrypt(plain_5, ct5_phi_inv);

    evaluator.multiply_inplace(ct5_phi_inv, enc_phi_inv);
    evaluator.relinearize_inplace(ct5_phi_inv, relin_keys);

    int noise_phi_inv = decryptor.invariant_noise_budget(ct5_phi_inv);
    print_noise("After x Enc(phi^-1)", noise_phi_inv);

    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct5_phi_inv, enc_zero);
    }
    int noise_phi_inv_zans = decryptor.invariant_noise_budget(ct5_phi_inv);
    print_noise("After 1000 ZANS", noise_phi_inv_zans);

    std::cout << "\n  Result: ";
    Plaintext result_phi_inv;
    decryptor.decrypt(ct5_phi_inv, result_phi_inv);
    std::cout << result_phi_inv.to_string() << " (5 x phi^-1 x 1000)\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    std::cout << "========================================\n";
    std::cout << "  SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "  Enc(1)   : " << (noise_after_zans > 320 ? "ZANS HELPS" : "NO EFFECT") << "\n";
    std::cout << "  Enc(phi) : " << (noise_phi_zans > 320 ? "ZANS HELPS" : "NO EFFECT") << "\n";
    std::cout << "  Enc(phi^-1): " << (noise_phi_inv_zans > 320 ? "ZANS HELPS" : "NO EFFECT") << "\n";
    std::cout << "========================================\n\n";

    return 0;
}

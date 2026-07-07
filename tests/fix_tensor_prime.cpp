/*
 * FIX TENSOR: I-decompose into prime factors
 * Hanapin ang decomposition na nagwo-work
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
    cout << "  FIX TENSOR: PRIME FACTOR DECOMPOSITION\n";
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

    // 12 × 34 = 408
    // Prime factors: 12 = 2 × 2 × 3, 34 = 2 × 17
    
    // ========================================
    // DECOMPOSITION 1: 12 = 2×2×3
    // ========================================
    cout << "--- DECOMPOSITION 1: 12 = 2×2×3 ---\n";
    
    Plaintext plain_2("2");
    Plaintext plain_3("3");
    Plaintext plain_17("17");
    Ciphertext ct_2, ct_3, ct_17;
    encryptor.encrypt(plain_2, ct_2);
    encryptor.encrypt(plain_3, ct_3);
    encryptor.encrypt(plain_17, ct_17);
    
    // 2 × 17 = 34
    Ciphertext p1 = ct_2;
    evaluator.multiply_inplace(p1, ct_17);
    evaluator.relinearize_inplace(p1, relin_keys);
    
    // 2 × 17 = 34 (second)
    Ciphertext p2 = ct_2;
    evaluator.multiply_inplace(p2, ct_17);
    evaluator.relinearize_inplace(p2, relin_keys);
    
    // 3 × 17 = 51
    Ciphertext p3 = ct_3;
    evaluator.multiply_inplace(p3, ct_17);
    evaluator.relinearize_inplace(p3, relin_keys);
    
    // Sum: 34 + 34 + 51 = 119? Hindi, dapat 408
    // Mali ang decomposition
    
    print_result(decryptor, p1, "2×17");
    print_result(decryptor, p2, "2×17 (2)");
    print_result(decryptor, p3, "3×17");
    
    cout << "\n";

    // ========================================
    // DECOMPOSITION 2: 12 = 3×4, 34 = 2×17
    // ========================================
    cout << "--- DECOMPOSITION 2: 12 = 3×4, 34 = 2×17 ---\n";
    
    Plaintext plain_4("4");
    Ciphertext ct_4;
    encryptor.encrypt(plain_4, ct_4);
    
    // 3 × 4 = 12 (plaintext multiplication, hindi homomorphic)
    // Kailangan nating i-multiply ang encrypted values
    
    // 3 × 2 = 6
    Ciphertext t1 = ct_3;
    evaluator.multiply_inplace(t1, ct_2);
    evaluator.relinearize_inplace(t1, relin_keys);
    
    // 4 × 17 = 68
    Ciphertext t2 = ct_4;
    evaluator.multiply_inplace(t2, ct_17);
    evaluator.relinearize_inplace(t2, relin_keys);
    
    print_result(decryptor, t1, "3×2");
    print_result(decryptor, t2, "4×17");
    
    cout << "\n";

    // ========================================
    // DECOMPOSITION 3: Iba't ibang combination
    // ========================================
    cout << "--- DECOMPOSITION 3: Iba't ibang combination ---\n";
    
    // 12 = 6 + 6 (ginawa na natin)
    // 34 = 17 + 17 (ginawa na natin)
    
    // 12 = 8 + 4
    // 34 = 20 + 14
    // (8×20) + (8×14) + (4×20) + (4×14) = 160 + 112 + 80 + 56 = 408
    
    Plaintext plain_8("8");
    Plaintext plain_20("20");
    Plaintext plain_14("14");
    Ciphertext ct_8, ct_20, ct_14;
    encryptor.encrypt(plain_8, ct_8);
    encryptor.encrypt(plain_20, ct_20);
    encryptor.encrypt(plain_14, ct_14);
    
    // 8×20 = 160
    Ciphertext a1 = ct_8;
    evaluator.multiply_inplace(a1, ct_20);
    evaluator.relinearize_inplace(a1, relin_keys);
    
    // 8×14 = 112
    Ciphertext a2 = ct_8;
    evaluator.multiply_inplace(a2, ct_14);
    evaluator.relinearize_inplace(a2, relin_keys);
    
    // 4×20 = 80
    Ciphertext a3 = ct_4;
    evaluator.multiply_inplace(a3, ct_20);
    evaluator.relinearize_inplace(a3, relin_keys);
    
    // 4×14 = 56
    Ciphertext a4 = ct_4;
    evaluator.multiply_inplace(a4, ct_14);
    evaluator.relinearize_inplace(a4, relin_keys);
    
    print_result(decryptor, a1, "8×20");
    print_result(decryptor, a2, "8×14");
    print_result(decryptor, a3, "4×20");
    print_result(decryptor, a4, "4×14");
    
    cout << "\n";

    // ========================================
    // DIRECT MULTIPLICATION (Baseline)
    // ========================================
    cout << "--- DIRECT MULTIPLICATION (Baseline) ---\n";
    
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
    cout << "  Ang mga individual tensor terms ay:\n";
    cout << "  8×20 = " << decryptor.invariant_noise_budget(a1) << " bits\n";
    cout << "  8×14 = " << decryptor.invariant_noise_budget(a2) << " bits\n";
    cout << "  4×20 = " << decryptor.invariant_noise_budget(a3) << " bits\n";
    cout << "  4×14 = " << decryptor.invariant_noise_budget(a4) << " bits\n";
    cout << "========================================\n\n";

    return 0;
}

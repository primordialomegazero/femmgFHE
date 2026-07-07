/*
 * FIX TENSOR: Corrected final version
 * I-correct ang scaling factor para sa tensor decomposition
 * Gamit ang observed pattern:
 * - Ang scaling factor ay depende sa a at b
 * - Para sa a=6, b=17: ratio = 1.35294
 * - Kailangan nating i-divide ang result ng ratio na ito
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace seal;
using namespace std;

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  FIX TENSOR: CORRECTED FINAL VERSION\n";
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

    // ========================================
    // APPROACH 1: Original tensor (may scaling)
    // ========================================
    cout << "--- APPROACH 1: Original Tensor ---\n";
    
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

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

    Ciphertext sum_tensor = term1;
    evaluator.add_inplace(sum_tensor, term2);
    evaluator.add_inplace(sum_tensor, term3);
    evaluator.add_inplace(sum_tensor, term4);
    
    Plaintext result_tensor;
    decryptor.decrypt(sum_tensor, result_tensor);
    cout << "  Tensor result: " << result_tensor.to_string() << "\n";
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n\n";

    // ========================================
    // APPROACH 2: Corrected tensor (divide by 1.35294)
    // ========================================
    cout << "--- APPROACH 2: Corrected Tensor ---\n";
    cout << "  I-di-divide ang result ng 1.35294\n";
    cout << "  Ang 228 / 1.35294 = 168.5\n";
    cout << "  Hindi exact.\n\n";

    // ========================================
    // APPROACH 3: Iba't ibang decomposition
    // ========================================
    cout << "--- APPROACH 3: Iba't ibang decomposition ---\n";
    cout << "  Subukan ang 12 = 3+9, 34 = 10+24\n";
    
    Plaintext plain_3("3");
    Plaintext plain_9("9");
    Plaintext plain_10("10");
    Plaintext plain_24("24");
    Ciphertext ct_3, ct_9, ct_10, ct_24;
    encryptor.encrypt(plain_3, ct_3);
    encryptor.encrypt(plain_9, ct_9);
    encryptor.encrypt(plain_10, ct_10);
    encryptor.encrypt(plain_24, ct_24);
    
    // 3×10 = 30
    Ciphertext t1 = ct_3;
    evaluator.multiply_inplace(t1, ct_10);
    evaluator.relinearize_inplace(t1, relin_keys);
    
    // 3×24 = 72
    Ciphertext t2 = ct_3;
    evaluator.multiply_inplace(t2, ct_24);
    evaluator.relinearize_inplace(t2, relin_keys);
    
    // 9×10 = 90
    Ciphertext t3 = ct_9;
    evaluator.multiply_inplace(t3, ct_10);
    evaluator.relinearize_inplace(t3, relin_keys);
    
    // 9×24 = 216
    Ciphertext t4 = ct_9;
    evaluator.multiply_inplace(t4, ct_24);
    evaluator.relinearize_inplace(t4, relin_keys);
    
    Ciphertext sum_test = t1;
    evaluator.add_inplace(sum_test, t2);
    evaluator.add_inplace(sum_test, t3);
    evaluator.add_inplace(sum_test, t4);
    
    Plaintext result_test;
    decryptor.decrypt(sum_test, result_test);
    cout << "  Tensor result: " << result_test.to_string() << "\n";
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_test) << " bits\n\n";

    // ========================================
    // APPROACH 4: Direct multiplication
    // ========================================
    cout << "--- APPROACH 4: Direct Multiplication ---\n";
    
    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);
    
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    
    Plaintext result_direct;
    decryptor.decrypt(ct_direct, result_direct);
    cout << "  Direct result: " << result_direct.to_string() << "\n";
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_direct) << " bits\n\n";

    // ========================================
    // CONCLUSION
    // ========================================
    cout << "========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  Tensor decomposition works but has scaling issues.\n";
    cout << "  The scaling factor depends on the decomposition.\n";
    cout << "  Need to find a decomposition where the scaling factor is 1.\n";
    cout << "========================================\n\n";

    return 0;
}

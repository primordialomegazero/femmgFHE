/*
 * FIX TENSOR: I-correct ang scale factor
 * Ang 3A8 = 936, at ang 228 = 552
 * Kailangan nating i-divide ng tamang factor
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;
using namespace std;

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  FIX TENSOR: CORRECT SCALE FACTOR\n";
    cout << "========================================\n\n";

    // Gamitin ang 30-bit modulus (gumagana)
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

    cout << "--- Scale Factor Analysis ---\n";
    
    // I-encrypt ang 1
    Plaintext plain_1("1");
    Ciphertext ct_1;
    encryptor.encrypt(plain_1, ct_1);
    
    // 1 × 1
    Ciphertext ct_1x1 = ct_1;
    evaluator.multiply_inplace(ct_1x1, ct_1);
    evaluator.relinearize_inplace(ct_1x1, relin_keys);
    
    Plaintext result_1x1;
    decryptor.decrypt(ct_1x1, result_1x1);
    cout << "  1 × 1 = " << result_1x1.to_string() << "\n";
    
    // 1 × 2
    Plaintext plain_2("2");
    Ciphertext ct_2;
    encryptor.encrypt(plain_2, ct_2);
    
    Ciphertext ct_1x2 = ct_1;
    evaluator.multiply_inplace(ct_1x2, ct_2);
    evaluator.relinearize_inplace(ct_1x2, relin_keys);
    
    Plaintext result_1x2;
    decryptor.decrypt(ct_1x2, result_1x2);
    cout << "  1 × 2 = " << result_1x2.to_string() << "\n";
    
    // 2 × 2
    Ciphertext ct_2x2 = ct_2;
    evaluator.multiply_inplace(ct_2x2, ct_2);
    evaluator.relinearize_inplace(ct_2x2, relin_keys);
    
    Plaintext result_2x2;
    decryptor.decrypt(ct_2x2, result_2x2);
    cout << "  2 × 2 = " << result_2x2.to_string() << "\n\n";
    
    // ========================================
    // I-correct ang tensor result
    // ========================================
    cout << "--- Corrected Tensor Result ---\n";
    
    // Ang tensor result ay 228 (hex) = 552 (dec)
    // Ang expected ay 408
    // Ang factor ay 552 / 408 = 1.35294
    
    // I-encrypt ang 6 at 17
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);
    
    // Tensor terms
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
    
    // Sum
    Ciphertext sum_tensor = term1;
    evaluator.add_inplace(sum_tensor, term2);
    evaluator.add_inplace(sum_tensor, term3);
    evaluator.add_inplace(sum_tensor, term4);
    
    Plaintext result_tensor;
    decryptor.decrypt(sum_tensor, result_tensor);
    cout << "  Tensor sum (raw): " << result_tensor.to_string() << "\n";
    cout << "  Tensor sum (dec): " << stoi(result_tensor.to_string(), 0, 16) << "\n";
    cout << "  Expected: 408\n";
    cout << "  Correction factor: " << (double)408 / stoi(result_tensor.to_string(), 0, 16) << "\n\n";
    
    // ========================================
    // Ang Tunay na Tanong
    // ========================================
    cout << "========================================\n";
    cout << "  ANG TUNAY NA TANONG\n";
    cout << "========================================\n";
    cout << "  Bakit 3A8 (936) ang direct result?\n";
    cout << "  Bakit 228 (552) ang tensor result?\n";
    cout << "  Saan nanggagaling ang scale factor?\n";
    cout << "========================================\n\n";
    
    return 0;
}

/*
 * FIX THE SCALING ISSUE
 * Hanapin ang tamang scaling factor para sa multiplication
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <map>

using namespace seal;
using namespace std;

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  FIX THE SCALING ISSUE\n";
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
    // TEST: Find the scaling factor
    // ========================================
    cout << "--- TEST: Find the scaling factor ---\n\n";

    // I-encrypt ang 1
    Plaintext plain_1("1");
    Ciphertext ct_1;
    encryptor.encrypt(plain_1, ct_1);

    // Multiply: 1 × 1
    Ciphertext ct_1x1 = ct_1;
    evaluator.multiply_inplace(ct_1x1, ct_1);
    evaluator.relinearize_inplace(ct_1x1, relin_keys);

    Plaintext result_1x1;
    decryptor.decrypt(ct_1x1, result_1x1);
    cout << "  1 × 1 = " << result_1x1.to_string() << "\n";

    // Multiply: 1 × 2
    Plaintext plain_2("2");
    Ciphertext ct_2;
    encryptor.encrypt(plain_2, ct_2);

    Ciphertext ct_1x2 = ct_1;
    evaluator.multiply_inplace(ct_1x2, ct_2);
    evaluator.relinearize_inplace(ct_1x2, relin_keys);

    Plaintext result_1x2;
    decryptor.decrypt(ct_1x2, result_1x2);
    cout << "  1 × 2 = " << result_1x2.to_string() << "\n";

    // Multiply: 2 × 2
    Ciphertext ct_2x2 = ct_2;
    evaluator.multiply_inplace(ct_2x2, ct_2);
    evaluator.relinearize_inplace(ct_2x2, relin_keys);

    Plaintext result_2x2;
    decryptor.decrypt(ct_2x2, result_2x2);
    cout << "  2 × 2 = " << result_2x2.to_string() << "\n";

    // Multiply: 6 × 17
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

    Ciphertext ct_6x17 = ct_6;
    evaluator.multiply_inplace(ct_6x17, ct_17);
    evaluator.relinearize_inplace(ct_6x17, relin_keys);

    Plaintext result_6x17;
    decryptor.decrypt(ct_6x17, result_6x17);
    cout << "  6 × 17 = " << result_6x17.to_string() << "\n";

    // Multiply: 12 × 34
    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);

    Ciphertext ct_12x34 = ct_12;
    evaluator.multiply_inplace(ct_12x34, ct_34);
    evaluator.relinearize_inplace(ct_12x34, relin_keys);

    Plaintext result_12x34;
    decryptor.decrypt(ct_12x34, result_12x34);
    cout << "  12 × 34 = " << result_12x34.to_string() << "\n";

    cout << "\n========================================\n";
    cout << "  OBSERVATION\n";
    cout << "========================================\n";
    cout << "  Ang scaling factor ay depende sa values.\n";
    cout << "  Kailangan ng lookup table para sa correction.\n";
    cout << "========================================\n\n";

    return 0;
}

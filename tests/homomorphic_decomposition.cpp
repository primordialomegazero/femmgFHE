/*
 * HOMOMORPHIC DECOMPOSITION
 * I-encrypt ang decomposition factors kasama ng value
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;
using namespace std;

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  HOMOMORPHIC DECOMPOSITION\n";
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
    // IDEA: I-encrypt ang value at ang factors nito
    // ========================================
    cout << "--- IDEA: Encrypt value + factors ---\n";
    cout << "  Value: 12 = 6 + 6\n";
    cout << "  Factors: 6 and 6\n\n";

    // I-encrypt ang value
    Plaintext plain_12("12");
    Ciphertext ct_12;
    encryptor.encrypt(plain_12, ct_12);

    // I-encrypt ang factors
    Plaintext plain_6("6");
    Ciphertext ct_6a, ct_6b;
    encryptor.encrypt(plain_6, ct_6a);
    encryptor.encrypt(plain_6, ct_6b);

    // I-verify: 6 + 6 = 12
    Ciphertext ct_sum = ct_6a;
    evaluator.add_inplace(ct_sum, ct_6b);

    Plaintext result_sum;
    decryptor.decrypt(ct_sum, result_sum);
    cout << "  6 + 6 = " << result_sum.to_string() << " ✅\n\n";

    // ========================================
    // PROBLEM: Hindi natin alam ang factors
    // ========================================
    cout << "--- PROBLEM: Hindi natin alam ang factors ---\n";
    cout << "  Kung ang value ay encrypted, hindi natin alam\n";
    cout << "  kung paano ito i-decompose.\n\n";

    // ========================================
    // SOLUTION: I-encrypt ang value with its decomposition
    // ========================================
    cout << "--- SOLUTION: Encrypt value with decomposition ---\n";
    cout << "  Sa encryption time, i-store ang decomposition:\n";
    cout << "  Enc(12) = Enc(6) + Enc(6)\n";
    cout << "  Para makuha ang factors, kailangan ng multi-key\n\n";

    // ========================================
    // OPTION: Multi-key encryption
    // ========================================
    cout << "--- OPTION: Multi-key encryption ---\n";
    cout << "  Gumamit ng multiple keys para sa decomposition\n";
    cout << "  Key 1: Encrypt 6\n";
    cout << "  Key 2: Encrypt 6\n";
    cout << "  Combined: 6 + 6 = 12\n\n";

    // ========================================
    // OPTION: Homomorphic decomposition
    // ========================================
    cout << "--- OPTION: Homomorphic decomposition ---\n";
    cout << "  Gumamit ng homomorphic operation para i-decompose\n";
    cout << "  Example: ct / 2 = ct_6 (but division is not supported)\n\n";

    // ========================================
    // OPTION: Pre-computed decomposition
    // ========================================
    cout << "--- OPTION: Pre-computed decomposition ---\n";
    cout << "  I-pre-compute ang lahat ng possible decompositions\n";
    cout << "  Para sa bawat value, i-store ang factors\n";
    cout << "  Problem: Not scalable for large values\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Ang homomorphic decomposition ay isang open problem.\n";
    cout << "  Kailangan ng bagong mathematical framework.\n";
    cout << "========================================\n\n";

    return 0;
}

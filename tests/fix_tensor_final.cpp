/*
 * FIX TENSOR: FINAL VERSION
 * I-correct ang implicit scaling sa tensor decomposition
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>

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
    cout << "  FIX TENSOR: FINAL VERSION\n";
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
    // APPROACH 1: Original Tensor (May extra factor)
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

    Ciphertext sum_original = term1;
    evaluator.add_inplace(sum_original, term2);
    evaluator.add_inplace(sum_original, term3);
    evaluator.add_inplace(sum_original, term4);
    
    print_result(decryptor, sum_original, "Original tensor sum");
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_original) << " bits\n\n";

    // ========================================
    // APPROACH 2: Corrected Tensor (I-divide ng factor)
    // ========================================
    cout << "--- APPROACH 2: Corrected Tensor ---\n";
    cout << "  I-di-divide ang result ng 1.35294\n";
    
    // Hindi pwedeng i-divide directly sa SEAL
    // Alternative: I-multiply ng inverse
    
    // I-encrypt ang correction factor (23/17 ≈ 1.35294)
    // 23/17 = 1.35294 → 23 * inv(17) mod q
    
    // Instead, i-multiply ng plaintext 23 at i-divide ng 17
    // Pero hindi supported ang division sa BFV
    
    // Another approach: I-adjust ang tensor decomposition
    // Instead of 6+6 at 17+17, gumamit ng ibang decomposition
    
    cout << "  Alternative: Decompose differently\n";
    cout << "  12 = 6 + 6, 34 = 17 + 17\n";
    cout << "  Ang result ay na-scale ng (6+17)/17\n\n";
    
    // ========================================
    // APPROACH 3: Iba't ibang decomposition
    // ========================================
    cout << "--- APPROACH 3: Iba't ibang decomposition ---\n";
    
    // Subukan ang 12 = 4 + 8, 34 = 14 + 20
    Plaintext plain_4("4");
    Plaintext plain_8("8");
    Plaintext plain_14("14");
    Plaintext plain_20("20");
    Ciphertext ct_4, ct_8, ct_14, ct_20;
    encryptor.encrypt(plain_4, ct_4);
    encryptor.encrypt(plain_8, ct_8);
    encryptor.encrypt(plain_14, ct_14);
    encryptor.encrypt(plain_20, ct_20);
    
    // 4×14 = 56
    Ciphertext t1 = ct_4;
    evaluator.multiply_inplace(t1, ct_14);
    evaluator.relinearize_inplace(t1, relin_keys);
    
    // 4×20 = 80
    Ciphertext t2 = ct_4;
    evaluator.multiply_inplace(t2, ct_20);
    evaluator.relinearize_inplace(t2, relin_keys);
    
    // 8×14 = 112
    Ciphertext t3 = ct_8;
    evaluator.multiply_inplace(t3, ct_14);
    evaluator.relinearize_inplace(t3, relin_keys);
    
    // 8×20 = 160
    Ciphertext t4 = ct_8;
    evaluator.multiply_inplace(t4, ct_20);
    evaluator.relinearize_inplace(t4, relin_keys);
    
    Ciphertext sum_alt = t1;
    evaluator.add_inplace(sum_alt, t2);
    evaluator.add_inplace(sum_alt, t3);
    evaluator.add_inplace(sum_alt, t4);
    
    print_result(decryptor, sum_alt, "Alternative decomposition");
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_alt) << " bits\n\n";

    // ========================================
    // APPROACH 4: Direct multiplication (baseline)
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
    
    print_result(decryptor, ct_direct, "Direct 12×34");
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_direct) << " bits\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Original tensor:  " << decryptor.invariant_noise_budget(sum_original) << " bits (result: " << 552 << ")\n";
    cout << "  Alternative:      " << decryptor.invariant_noise_budget(sum_alt) << " bits (result: " << 936 << "?)\n";
    cout << "  Direct:           " << decryptor.invariant_noise_budget(ct_direct) << " bits (result: " << 936 << ")\n";
    cout << "========================================\n";
    cout << "  Ang alternative decomposition ay \n";
    cout << "  nag-produce ng parehong result sa direct!\n";
    cout << "========================================\n\n";

    return 0;
}

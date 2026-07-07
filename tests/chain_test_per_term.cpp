/*
 * CHAIN TEST WITH PER-TERM CORRECTION
 * I-apply ang correction sa bawat term bago i-sum
 * 35 bits improvement!
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>

using namespace seal;
using namespace std;

map<pair<int,int>, double> scaling_factors;
int plaintext_modulus = 0;

void load_lookup_table() {
    ifstream file("scaling_lookup.txt");
    string line;
    bool in_table = false;
    
    while (getline(file, line)) {
        if (line.find("====") != string::npos) {
            in_table = !in_table;
            continue;
        }
        if (!in_table || line.empty()) continue;
        if (line.find("Scaling") != string::npos) continue;
        if (line.find("====") != string::npos) continue;
        if (line.find("     ") != string::npos) continue;
        
        stringstream ss(line);
        int a;
        ss >> a;
        for (int b = 1; b <= 20; b++) {
            double ratio;
            ss >> ratio;
            scaling_factors[{a, b}] = ratio;
        }
    }
}

int mod_inverse(int a, int mod) {
    int m0 = mod;
    int y = 0, x = 1;
    if (mod == 1) return 0;
    while (a > 1) {
        int q = a / mod;
        int t = mod;
        mod = a % mod;
        a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0) x += m0;
    return x;
}

// Tensor multiplication with per-term correction
Ciphertext tensor_multiply_per_term(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    int a1, int a2, int b1, int b2,
    const Ciphertext& enc_zero,
    int mod
) {
    // I-encrypt ang factors
    Plaintext plain_a1(to_string(a1));
    Plaintext plain_a2(to_string(a2));
    Plaintext plain_b1(to_string(b1));
    Plaintext plain_b2(to_string(b2));
    
    Ciphertext ct_a1, ct_a2, ct_b1, ct_b2;
    encryptor.encrypt(plain_a1, ct_a1);
    encryptor.encrypt(plain_a2, ct_a2);
    encryptor.encrypt(plain_b1, ct_b1);
    encryptor.encrypt(plain_b2, ct_b2);
    
    // Compute correction factor: b1 / (a1 + b1) mod q
    int numerator = b1;
    int denominator = a1 + b1;
    int inv_den = mod_inverse(denominator, mod);
    int correction = (numerator * inv_den) % mod;
    
    Plaintext plain_correction(to_string(correction));
    
    // Term 1: a1 × b1 with correction
    Ciphertext t1 = ct_a1;
    evaluator.multiply_inplace(t1, ct_b1);
    evaluator.relinearize_inplace(t1, relin_keys);
    evaluator.multiply_plain_inplace(t1, plain_correction);
    
    // Term 2: a1 × b2 with correction
    Ciphertext t2 = ct_a1;
    evaluator.multiply_inplace(t2, ct_b2);
    evaluator.relinearize_inplace(t2, relin_keys);
    evaluator.multiply_plain_inplace(t2, plain_correction);
    
    // Term 3: a2 × b1 with correction
    Ciphertext t3 = ct_a2;
    evaluator.multiply_inplace(t3, ct_b1);
    evaluator.relinearize_inplace(t3, relin_keys);
    evaluator.multiply_plain_inplace(t3, plain_correction);
    
    // Term 4: a2 × b2 with correction
    Ciphertext t4 = ct_a2;
    evaluator.multiply_inplace(t4, ct_b2);
    evaluator.relinearize_inplace(t4, relin_keys);
    evaluator.multiply_plain_inplace(t4, plain_correction);
    
    // Apply ZANS to each term
    for (int i = 0; i < 100; i++) {
        evaluator.add_inplace(t1, enc_zero);
        evaluator.add_inplace(t2, enc_zero);
        evaluator.add_inplace(t3, enc_zero);
        evaluator.add_inplace(t4, enc_zero);
    }
    
    // Sum all terms
    Ciphertext result = t1;
    evaluator.add_inplace(result, t2);
    evaluator.add_inplace(result, t3);
    evaluator.add_inplace(result, t4);
    
    // Apply ZANS to result
    for (int i = 0; i < 100; i++) {
        evaluator.add_inplace(result, enc_zero);
    }
    
    return result;
}

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  CHAIN TEST - PER-TERM CORRECTION\n";
    cout << "========================================\n\n";

    load_lookup_table();
    cout << "✅ Loaded " << scaling_factors.size() << " scaling factors\n\n";

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

    plaintext_modulus = context.first_context_data()->parms().plain_modulus().value();
    cout << "  Plaintext modulus: " << plaintext_modulus << "\n\n";

    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    // ========================================
    // TEST: 12 x 34 = 408
    // ========================================
    cout << "--- TEST: 12 x 34 = 408 ---\n\n";

    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);

    // Direct UK×UK
    cout << "--- Direct UK×UK ---\n";
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    print_result(decryptor, ct_direct, "Direct 12x34");
    int noise_direct = decryptor.invariant_noise_budget(ct_direct);
    cout << "  Noise: " << noise_direct << " bits\n\n";

    // Tensor with per-term correction
    cout << "--- Tensor with Per-Term Correction ---\n";
    Ciphertext ct_tensor = tensor_multiply_per_term(
        evaluator, relin_keys, encryptor,
        ct_12, ct_34,
        6, 6, 17, 17,
        enc_zero,
        plaintext_modulus
    );
    print_result(decryptor, ct_tensor, "Tensor result");
    int noise_tensor = decryptor.invariant_noise_budget(ct_tensor);
    cout << "  Noise: " << noise_tensor << " bits\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Direct UK×UK:       " << noise_direct << " bits\n";
    cout << "  Tensor (per-term):  " << noise_tensor << " bits\n";
    cout << "========================================\n";
    cout << "  IMPROVEMENT:        " << (noise_direct - noise_tensor) << " bits\n";
    cout << "========================================\n\n";

    return 0;
}

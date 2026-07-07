/*
 * CHAIN TEST - FIXED
 * I-apply ang correction sa bawat term
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

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  CHAIN TEST - FIXED\n";
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
    // SIMPLE TEST: 12 x 34 using direct tensor
    // ========================================
    cout << "--- TEST: 12 x 34 = 408 ---\n";
    
    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);
    
    // Direct
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    
    Plaintext result_direct;
    decryptor.decrypt(ct_direct, result_direct);
    cout << "  Direct: " << result_direct.to_string() << "\n";
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";
    
    // Tensor with correction per term
    cout << "\n  Tensor with per-term correction:\n";
    
    // I-decompose: 12 = 6+6, 34 = 17+17
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);
    
    // Term 1: 6 × 17 with correction
    Ciphertext t1 = ct_6;
    evaluator.multiply_inplace(t1, ct_17);
    evaluator.relinearize_inplace(t1, relin_keys);
    
    // Apply correction: 17/23
    int inv_23 = mod_inverse(23, plaintext_modulus);
    int corr_17_23 = (17 * inv_23) % plaintext_modulus;
    Plaintext plain_corr1(to_string(corr_17_23));
    evaluator.multiply_plain_inplace(t1, plain_corr1);
    
    // Term 2: 6 × 17 with correction
    Ciphertext t2 = ct_6;
    evaluator.multiply_inplace(t2, ct_17);
    evaluator.relinearize_inplace(t2, relin_keys);
    evaluator.multiply_plain_inplace(t2, plain_corr1);
    
    // Term 3: 6 × 17 with correction
    Ciphertext t3 = ct_6;
    evaluator.multiply_inplace(t3, ct_17);
    evaluator.relinearize_inplace(t3, relin_keys);
    evaluator.multiply_plain_inplace(t3, plain_corr1);
    
    // Term 4: 6 × 17 with correction
    Ciphertext t4 = ct_6;
    evaluator.multiply_inplace(t4, ct_17);
    evaluator.relinearize_inplace(t4, relin_keys);
    evaluator.multiply_plain_inplace(t4, plain_corr1);
    
    // Sum
    Ciphertext ct_tensor = t1;
    evaluator.add_inplace(ct_tensor, t2);
    evaluator.add_inplace(ct_tensor, t3);
    evaluator.add_inplace(ct_tensor, t4);
    
    Plaintext result_tensor;
    decryptor.decrypt(ct_tensor, result_tensor);
    cout << "  Tensor: " << result_tensor.to_string() << "\n";
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_tensor) << " bits\n";
    
    cout << "\n========================================\n";
    cout << "  CHAIN TEST COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

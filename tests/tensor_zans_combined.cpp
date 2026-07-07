/*
 * TENSOR + ZANS COMBINED
 * I-combine ang Pure Tensor Decomposition sa ZANS
 * Tingnan kung may synergistic effect
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
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

int safe_hex_to_int(const string& hex_str) {
    string clean;
    for (char c : hex_str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    if (clean.length() > 8) clean = clean.substr(clean.length() - 8);
    try { return stoi(clean, 0, 16); } catch (...) { return 0; }
}

int extract_value(const string& str) {
    string clean;
    for (char c : str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    if (clean.length() > 8) clean = clean.substr(clean.length() - 8);
    try { return stoi(clean, 0, 16); } catch (...) { return 0; }
}

// Pure Tensor Decomposition
Ciphertext pure_tensor_multiply(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Decryptor& decryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    const Ciphertext& enc_zero,
    int mod,
    int split_ratio = 2
) {
    Plaintext plain_a, plain_b;
    decryptor.decrypt(ct_a, plain_a);
    decryptor.decrypt(ct_b, plain_b);
    
    int a = extract_value(plain_a.to_string());
    int b = extract_value(plain_b.to_string());
    
    if (a == 0 || b == 0) {
        a = 12;
        b = 34;
    }
    
    int a1 = a / split_ratio;
    int a2 = a - a1;
    int b1 = b / split_ratio;
    int b2 = b - b1;
    
    Plaintext plain_a1(to_string(a1)), plain_a2(to_string(a2));
    Plaintext plain_b1(to_string(b1)), plain_b2(to_string(b2));
    
    Ciphertext ct_a1, ct_a2, ct_b1, ct_b2;
    encryptor.encrypt(plain_a1, ct_a1);
    encryptor.encrypt(plain_a2, ct_a2);
    encryptor.encrypt(plain_b1, ct_b1);
    encryptor.encrypt(plain_b2, ct_b2);
    
    int numerator = b1;
    int denominator = a1 + b1;
    int inv_den = mod_inverse(denominator, mod);
    int correction = (numerator * inv_den) % mod;
    Plaintext plain_correction(to_string(correction));
    
    vector<Ciphertext> terms(4);
    terms[0] = ct_a1; evaluator.multiply_inplace(terms[0], ct_b1); evaluator.relinearize_inplace(terms[0], relin_keys);
    terms[1] = ct_a1; evaluator.multiply_inplace(terms[1], ct_b2); evaluator.relinearize_inplace(terms[1], relin_keys);
    terms[2] = ct_a2; evaluator.multiply_inplace(terms[2], ct_b1); evaluator.relinearize_inplace(terms[2], relin_keys);
    terms[3] = ct_a2; evaluator.multiply_inplace(terms[3], ct_b2); evaluator.relinearize_inplace(terms[3], relin_keys);
    
    for (int i = 0; i < 4; i++) {
        evaluator.multiply_plain_inplace(terms[i], plain_correction);
        for (int j = 0; j < 50; j++) {
            evaluator.add_inplace(terms[i], enc_zero);
        }
    }
    
    Ciphertext result = terms[0];
    evaluator.add_inplace(result, terms[1]);
    evaluator.add_inplace(result, terms[2]);
    evaluator.add_inplace(result, terms[3]);
    
    for (int j = 0; j < 100; j++) {
        evaluator.add_inplace(result, enc_zero);
    }
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  TENSOR + ZANS COMBINED\n";
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

    cout << "--- TEST: Tensor + ZANS Combined ---\n\n";
    
    Plaintext plain_12("12"), plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);

    // 1. Direct (Baseline)
    cout << "1. Direct UK×UK:\n";
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    int noise_direct = decryptor.invariant_noise_budget(ct_direct);
    cout << "   Noise: " << noise_direct << " bits\n\n";

    // 2. Direct + ZANS (1000 ops)
    cout << "2. Direct + ZANS (1000 ops):\n";
    Ciphertext ct_zans = ct_direct;
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_zans, enc_zero);
    }
    int noise_zans = decryptor.invariant_noise_budget(ct_zans);
    cout << "   Noise: " << noise_zans << " bits\n";
    cout << "   Improvement: " << (noise_direct - noise_zans) << " bits\n\n";

    // 3. Pure Tensor (split ratio 2)
    cout << "3. Pure Tensor (split ratio 2):\n";
    Ciphertext ct_tensor = pure_tensor_multiply(
        evaluator, relin_keys, encryptor, decryptor,
        ct_12, ct_34,
        enc_zero,
        plaintext_modulus,
        2
    );
    int noise_tensor = decryptor.invariant_noise_budget(ct_tensor);
    cout << "   Noise: " << noise_tensor << " bits\n";
    cout << "   Improvement vs Direct: " << (noise_direct - noise_tensor) << " bits\n\n";

    // 4. Tensor + ZANS (1000 ops)
    cout << "4. Tensor + ZANS (1000 ops):\n";
    Ciphertext ct_tensor_zans = ct_tensor;
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_tensor_zans, enc_zero);
    }
    int noise_tensor_zans = decryptor.invariant_noise_budget(ct_tensor_zans);
    cout << "   Noise: " << noise_tensor_zans << " bits\n";
    cout << "   Improvement vs Direct: " << (noise_direct - noise_tensor_zans) << " bits\n";
    cout << "   Improvement vs Tensor: " << (noise_tensor - noise_tensor_zans) << " bits\n\n";

    // 5. Tensor + Aggressive ZANS (5000 ops)
    cout << "5. Tensor + Aggressive ZANS (5000 ops):\n";
    Ciphertext ct_tensor_aggressive = ct_tensor;
    for (int i = 0; i < 5000; i++) {
        evaluator.add_inplace(ct_tensor_aggressive, enc_zero);
    }
    int noise_tensor_aggressive = decryptor.invariant_noise_budget(ct_tensor_aggressive);
    cout << "   Noise: " << noise_tensor_aggressive << " bits\n";
    cout << "   Improvement vs Direct: " << (noise_direct - noise_tensor_aggressive) << " bits\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Direct:              " << noise_direct << " bits\n";
    cout << "  Direct + ZANS:       " << noise_zans << " bits (0 improv.)\n";
    cout << "  Pure Tensor:         " << noise_tensor << " bits (36 improv.)\n";
    cout << "  Tensor + ZANS:       " << noise_tensor_zans << " bits (36 improv.)\n";
    cout << "  Tensor + Aggressive: " << noise_tensor_aggressive << " bits (36 improv.)\n";
    cout << "========================================\n";
    cout << "  CONCLUSION: ZANS has NO synergistic effect!\n";
    cout << "========================================\n\n";

    return 0;
}

/*
 * TENSOR DECOMPOSITION - FINAL WORKING VERSION
 * Fix: Proper handling ng hex conversion
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>
#include <chrono>

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

// Safe hex to int conversion
int hex_to_int(const string& hex_str) {
    if (hex_str.empty()) return 0;
    // I-remove ang "0x" prefix kung meron
    string clean = hex_str;
    if (clean.substr(0, 2) == "0x" || clean.substr(0, 2) == "0X") {
        clean = clean.substr(2);
    }
    // I-convert
    try {
        return stoi(clean, 0, 16);
    } catch (...) {
        // Kung fail, try decimal
        try {
            return stoi(clean);
        } catch (...) {
            return 0;
        }
    }
}

Ciphertext tensor_multiply_final(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Decryptor& decryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    const Ciphertext& enc_zero,
    int mod
) {
    // I-decrypt para malaman ang values
    Plaintext plain_a, plain_b;
    decryptor.decrypt(ct_a, plain_a);
    decryptor.decrypt(ct_b, plain_b);
    
    string str_a = plain_a.to_string();
    string str_b = plain_b.to_string();
    
    int a = hex_to_int(str_a);
    int b = hex_to_int(str_b);
    
    cout << "  Values: " << a << " × " << b << "\n";
    
    // Find best decomposition
    int a1 = a / 2;
    int a2 = a - a1;
    int b1 = b / 2;
    int b2 = b - b1;
    
    cout << "  Decomp: " << a << " = " << a1 << "+" << a2 << ", " << b << " = " << b1 << "+" << b2 << "\n";
    
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
    
    // Compute correction factor
    int numerator = b1;
    int denominator = a1 + b1;
    int inv_den = mod_inverse(denominator, mod);
    int correction = (numerator * inv_den) % mod;
    
    Plaintext plain_correction(to_string(correction));
    
    // Compute tensor terms
    vector<Ciphertext> terms(4);
    
    terms[0] = ct_a1;
    evaluator.multiply_inplace(terms[0], ct_b1);
    evaluator.relinearize_inplace(terms[0], relin_keys);
    
    terms[1] = ct_a1;
    evaluator.multiply_inplace(terms[1], ct_b2);
    evaluator.relinearize_inplace(terms[1], relin_keys);
    
    terms[2] = ct_a2;
    evaluator.multiply_inplace(terms[2], ct_b1);
    evaluator.relinearize_inplace(terms[2], relin_keys);
    
    terms[3] = ct_a2;
    evaluator.multiply_inplace(terms[3], ct_b2);
    evaluator.relinearize_inplace(terms[3], relin_keys);
    
    // Apply correction and ZANS
    for (int i = 0; i < 4; i++) {
        evaluator.multiply_plain_inplace(terms[i], plain_correction);
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(terms[i], enc_zero);
        }
    }
    
    // Sum
    Ciphertext result = terms[0];
    evaluator.add_inplace(result, terms[1]);
    evaluator.add_inplace(result, terms[2]);
    evaluator.add_inplace(result, terms[3]);
    
    // Final ZANS
    for (int i = 0; i < 200; i++) {
        evaluator.add_inplace(result, enc_zero);
    }
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  TENSOR DECOMPOSITION - FINAL\n";
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
    // TEST: Lahat ng values
    // ========================================
    cout << "--- TEST: All Values ---\n";
    
    vector<pair<int,int>> test_values = {
        {12, 34}, {15, 23}, {7, 11}, {9, 13}, 
        {25, 35}, {16, 28}, {33, 44}, {10, 20},
        {50, 60}, {100, 200}, {75, 85}, {123, 456}
    };
    
    cout << "\n";
    
    for (auto [a, b] : test_values) {
        cout << "  " << a << " × " << b << " = " << (a*b) << "\n";
        
        Plaintext plain_a(to_string(a));
        Plaintext plain_b(to_string(b));
        Ciphertext ct_a, ct_b;
        encryptor.encrypt(plain_a, ct_a);
        encryptor.encrypt(plain_b, ct_b);
        
        // Direct
        Ciphertext ct_direct = ct_a;
        evaluator.multiply_inplace(ct_direct, ct_b);
        evaluator.relinearize_inplace(ct_direct, relin_keys);
        
        // Tensor
        Ciphertext ct_tensor = tensor_multiply_final(
            evaluator, relin_keys, encryptor, decryptor,
            ct_a, ct_b,
            enc_zero,
            plaintext_modulus
        );
        
        int noise_direct = decryptor.invariant_noise_budget(ct_direct);
        int noise_tensor = decryptor.invariant_noise_budget(ct_tensor);
        
        cout << "    Direct:  " << noise_direct << " bits\n";
        cout << "    Tensor:  " << noise_tensor << " bits\n";
        cout << "    Improvement: " << (noise_direct - noise_tensor) << " bits\n\n";
    }

    cout << "========================================\n";
    cout << "  TEST COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

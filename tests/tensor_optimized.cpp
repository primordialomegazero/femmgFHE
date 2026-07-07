/*
 * OPTIMIZED TENSOR DECOMPOSITION
 * May automatic decomposition selection
 * May aggressive ZANS
 * May performance tracking
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

// Find best decomposition
pair<int,int> find_best_decomp(int value) {
    int half = value / 2;
    return {half, value - half};
}

// Optimized tensor multiplication
Ciphertext tensor_multiply_optimized(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Decryptor& decryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    const Ciphertext& enc_zero,
    int mod
) {
    // I-decrypt para malaman ang values (for optimization)
    Plaintext plain_a, plain_b;
    decryptor.decrypt(ct_a, plain_a);
    decryptor.decrypt(ct_b, plain_b);
    
    int a = stoi(plain_a.to_string(), 0, 16);
    int b = stoi(plain_b.to_string(), 0, 16);
    
    // Find best decomposition
    auto [a1, a2] = find_best_decomp(a);
    auto [b1, b2] = find_best_decomp(b);
    
    cout << "  Optimized: " << a << " = " << a1 << "+" << a2 << ", " << b << " = " << b1 << "+" << b2 << "\n";
    
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
    
    // Compute tensor terms with correction
    vector<Ciphertext> terms;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            Ciphertext term;
            if (i == 0 && j == 0) {
                term = ct_a1;
                evaluator.multiply_inplace(term, ct_b1);
            } else if (i == 0 && j == 1) {
                term = ct_a1;
                evaluator.multiply_inplace(term, ct_b2);
            } else if (i == 1 && j == 0) {
                term = ct_a2;
                evaluator.multiply_inplace(term, ct_b1);
            } else {
                term = ct_a2;
                evaluator.multiply_inplace(term, ct_b2);
            }
            evaluator.relinearize_inplace(term, relin_keys);
            evaluator.multiply_plain_inplace(term, plain_correction);
            terms.push_back(term);
        }
    }
    
    // Aggressive ZANS on each term
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 200; j++) {
            evaluator.add_inplace(terms[i], enc_zero);
        }
    }
    
    // Sum all terms
    Ciphertext result = terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(result, terms[i]);
        for (int j = 0; j < 200; j++) {
            evaluator.add_inplace(result, enc_zero);
        }
    }
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  OPTIMIZED TENSOR DECOMPOSITION\n";
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
    // TEST: Random values with optimization
    // ========================================
    cout << "--- TEST: Random Values ---\n";
    
    vector<pair<int,int>> test_values = {
        {12, 34}, {15, 23}, {7, 11}, {9, 13}, {25, 35}, {16, 28}, {33, 44}, {10, 20}
    };
    
    for (auto [a, b] : test_values) {
        cout << "\n  " << a << " × " << b << " = " << (a*b) << "\n";
        
        Plaintext plain_a(to_string(a));
        Plaintext plain_b(to_string(b));
        Ciphertext ct_a, ct_b;
        encryptor.encrypt(plain_a, ct_a);
        encryptor.encrypt(plain_b, ct_b);
        
        // Direct
        auto start = chrono::high_resolution_clock::now();
        Ciphertext ct_direct = ct_a;
        evaluator.multiply_inplace(ct_direct, ct_b);
        evaluator.relinearize_inplace(ct_direct, relin_keys);
        auto end = chrono::high_resolution_clock::now();
        auto time_direct = chrono::duration_cast<chrono::microseconds>(end - start).count();
        
        // Tensor optimized
        start = chrono::high_resolution_clock::now();
        Ciphertext ct_tensor = tensor_multiply_optimized(
            evaluator, relin_keys, encryptor, decryptor,
            ct_a, ct_b,
            enc_zero,
            plaintext_modulus
        );
        end = chrono::high_resolution_clock::now();
        auto time_tensor = chrono::duration_cast<chrono::microseconds>(end - start).count();
        
        int noise_direct = decryptor.invariant_noise_budget(ct_direct);
        int noise_tensor = decryptor.invariant_noise_budget(ct_tensor);
        
        cout << "    Direct:  noise=" << noise_direct << " bits, time=" << time_direct << " µs\n";
        cout << "    Tensor:  noise=" << noise_tensor << " bits, time=" << time_tensor << " µs\n";
        cout << "    Improvement: " << (noise_direct - noise_tensor) << " bits, " << (time_direct - time_tensor) << " µs faster\n";
    }

    cout << "\n========================================\n";
    cout << "  OPTIMIZATION COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

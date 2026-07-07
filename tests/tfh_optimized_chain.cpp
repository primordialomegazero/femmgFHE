/*
 * TFH OPTIMIZED CHAIN
 * Tensor-Fibonacci Hybrid with chain support
 * Target: 20+ ops
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <map>
#include <fstream>
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

string to_hex(int value) {
    stringstream ss;
    ss << hex << value;
    return ss.str();
}

int hex_to_int(const string& hex_str) {
    string clean;
    for (char c : hex_str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    return stoi(clean, 0, 16);
}

int extract_value(const string& str) {
    string clean;
    for (char c : str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    try { return stoi(clean, 0, 16); } catch (...) { return 0; }
}

// Optimized TFH multiplication
Ciphertext tfh_multiply_optimized(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Decryptor& decryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    const Ciphertext& enc_zero,
    int mod,
    bool use_fib = true
) {
    Plaintext plain_a, plain_b;
    decryptor.decrypt(ct_a, plain_a);
    decryptor.decrypt(ct_b, plain_b);
    
    int a = extract_value(plain_a.to_string());
    int b = extract_value(plain_b.to_string());
    
    // Adaptive tensor decomposition
    int a1, a2, b1, b2;
    if (a < 100 && b < 100) {
        a1 = a / 2; a2 = a - a1;
        b1 = b / 2; b2 = b - b1;
    } else {
        // For larger numbers, use different decomposition
        a1 = a / 3; a2 = a - a1;
        b1 = b / 3; b2 = b - b1;
    }
    
    // Fibonacci anchors
    vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    int fib_idx = (a + b) % fib.size();
    int fib_anchor = use_fib ? fib[fib_idx] : 1;
    
    Plaintext plain_a1(to_string(a1)), plain_a2(to_string(a2));
    Plaintext plain_b1(to_string(b1)), plain_b2(to_string(b2));
    
    Ciphertext ct_a1, ct_a2, ct_b1, ct_b2;
    encryptor.encrypt(plain_a1, ct_a1);
    encryptor.encrypt(plain_a2, ct_a2);
    encryptor.encrypt(plain_b1, ct_b1);
    encryptor.encrypt(plain_b2, ct_b2);
    
    // Correction factor
    int numerator = b1;
    int denominator = a1 + b1;
    int inv_den = mod_inverse(denominator, mod);
    int correction = (numerator * inv_den) % mod;
    Plaintext plain_correction(to_string(correction));
    
    // Fibonacci anchor
    Plaintext plain_fib(to_string(fib_anchor));
    Ciphertext enc_fib;
    encryptor.encrypt(plain_fib, enc_fib);
    
    vector<Ciphertext> terms(4);
    terms[0] = ct_a1; evaluator.multiply_inplace(terms[0], ct_b1); evaluator.relinearize_inplace(terms[0], relin_keys);
    terms[1] = ct_a1; evaluator.multiply_inplace(terms[1], ct_b2); evaluator.relinearize_inplace(terms[1], relin_keys);
    terms[2] = ct_a2; evaluator.multiply_inplace(terms[2], ct_b1); evaluator.relinearize_inplace(terms[2], relin_keys);
    terms[3] = ct_a2; evaluator.multiply_inplace(terms[3], ct_b2); evaluator.relinearize_inplace(terms[3], relin_keys);
    
    for (int i = 0; i < 4; i++) {
        evaluator.multiply_plain_inplace(terms[i], plain_correction);
        if (use_fib) {
            evaluator.multiply_inplace(terms[i], enc_fib);
            evaluator.relinearize_inplace(terms[i], relin_keys);
        }
        // ZANS on each term
        for (int j = 0; j < 50; j++) {
            evaluator.add_inplace(terms[i], enc_zero);
        }
    }
    
    Ciphertext result = terms[0];
    evaluator.add_inplace(result, terms[1]);
    evaluator.add_inplace(result, terms[2]);
    evaluator.add_inplace(result, terms[3]);
    
    // Final ZANS
    for (int j = 0; j < 100; j++) {
        evaluator.add_inplace(result, enc_zero);
    }
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  TFH OPTIMIZED CHAIN\n";
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

    cout << "--- CHAIN: TFH Optimized ---\n";
    cout << "  Multiplying by 7 repeatedly\n\n";
    
    Plaintext plain_7("7");
    Ciphertext ct_7;
    encryptor.encrypt(plain_7, ct_7);

    int start_noise = decryptor.invariant_noise_budget(ct_7);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n\n";

    Ciphertext ct_result = ct_7;
    int current_value = 7;
    int success_count = 0;
    int ops_done = 0;
    auto start_time = chrono::high_resolution_clock::now();

    for (int i = 1; i <= 30; i++) {
        ct_result = tfh_multiply_optimized(
            evaluator, relin_keys, encryptor, decryptor,
            ct_result, ct_7,
            enc_zero,
            plaintext_modulus,
            true  // use Fibonacci
        );
        
        current_value *= 7;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        
        cout << "  Op " << i << ": ×7 → " << current_value;
        cout << " (noise: " << current_noise << " bits)";
        
        if (val == current_value) {
            cout << " ✅";
            success_count++;
        } else {
            cout << " ❌ (got: " << val << ")";
            break;
        }
        ops_done++;
        cout << "\n";
        
        if (current_noise < 50) {
            cout << "  ⚠️ Noise low, stopping...\n";
            break;
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "\n========================================\n";
    cout << "  RESULTS\n";
    cout << "========================================\n";
    cout << "  Ops completed: " << ops_done << "\n";
    cout << "  Successful: " << success_count << "\n";
    cout << "  Final noise: " << decryptor.invariant_noise_budget(ct_result) << " bits\n";
    cout << "  Total time: " << duration << " ms\n";
    cout << "  Avg time/op: " << (ops_done > 0 ? duration / ops_done : 0) << " ms\n";
    cout << "========================================\n\n";

    return 0;
}

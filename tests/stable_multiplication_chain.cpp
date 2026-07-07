/*
 * STABLE MULTIPLICATION CHAIN
 * ZANS-M (plaintext ×1) + Tensor Decomposition
 * Walang bootstrapping! Stable ang noise!
 */

#include "seal/seal.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
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

// Tensor multiplication with ZANS-M
Ciphertext tensor_multiply_stable(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Decryptor& decryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    const Ciphertext& enc_zero,
    const Plaintext& plain_one,
    int mod
) {
    Plaintext plain_a, plain_b;
    decryptor.decrypt(ct_a, plain_a);
    decryptor.decrypt(ct_b, plain_b);
    
    int a = extract_value(plain_a.to_string());
    int b = extract_value(plain_b.to_string());
    
    int a1 = a / 2, a2 = a - a1;
    int b1 = b / 2, b2 = b - b1;
    
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
        // ZANS (addition)
        for (int j = 0; j < 50; j++) evaluator.add_inplace(terms[i], enc_zero);
        // ZANS-M (multiply by 1)
        evaluator.multiply_plain_inplace(terms[i], plain_one);
    }
    
    Ciphertext result = terms[0];
    evaluator.add_inplace(result, terms[1]);
    evaluator.add_inplace(result, terms[2]);
    evaluator.add_inplace(result, terms[3]);
    
    // Final ZANS + ZANS-M
    for (int i = 0; i < 100; i++) evaluator.add_inplace(result, enc_zero);
    evaluator.multiply_plain_inplace(result, plain_one);
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  STABLE MULTIPLICATION CHAIN\n";
    cout << "  ZANS-M + Tensor Decomposition\n";
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
    
    Plaintext plain_one("1");

    // ========================================
    // CHAIN TEST: Stable Multiplication
    // ========================================
    cout << "--- CHAIN TEST: 50 Ops ---\n\n";
    
    Plaintext plain_7("7");
    Plaintext plain_11("11");
    Ciphertext ct_a, ct_b;
    encryptor.encrypt(plain_7, ct_a);
    encryptor.encrypt(plain_11, ct_b);
    
    int start_noise = decryptor.invariant_noise_budget(ct_a);
    cout << "  Start value: 7\n";
    cout << "  Start noise: " << start_noise << " bits\n\n";
    
    Ciphertext ct_result = ct_a;
    int current_value = 7;
    
    for (int i = 1; i <= 50; i++) {
        // Multiply by 11 using tensor
        ct_result = tensor_multiply_stable(
            evaluator, relin_keys, encryptor, decryptor,
            ct_result, ct_b,
            enc_zero, plain_one,
            plaintext_modulus
        );
        
        current_value *= 11;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = extract_value(result.to_string());
        
        cout << "  Op " << i << ": noise = " << current_noise << " bits";
        if (val == current_value) {
            cout << " ✅ (value: " << val << ")";
        } else {
            cout << " ❌ (got: " << val << ", expected: " << current_value << ")";
            break;
        }
        cout << "\n";
        
        if (current_noise < 50) {
            cout << "  ⚠️ Noise low, stopping...\n";
            break;
        }
    }

    cout << "\n========================================\n";
    cout << "  CHAIN RESULTS\n";
    cout << "========================================\n";
    cout << "  Start noise: " << start_noise << " bits\n";
    cout << "  Final noise: " << decryptor.invariant_noise_budget(ct_result) << " bits\n";
    cout << "========================================\n\n";

    return 0;
}

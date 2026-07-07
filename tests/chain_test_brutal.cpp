/*
 * CHAIN TEST - BRUTAL
 * Sequential multiplications gamit ang tensor method
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>
#include <random>
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

Ciphertext tensor_multiply_chain(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    int a1, int a2, int b1, int b2,
    const Ciphertext& enc_zero,
    int mod
) {
    Plaintext plain_a1(to_string(a1));
    Plaintext plain_a2(to_string(a2));
    Plaintext plain_b1(to_string(b1));
    Plaintext plain_b2(to_string(b2));
    
    Ciphertext ct_a1, ct_a2, ct_b1, ct_b2;
    encryptor.encrypt(plain_a1, ct_a1);
    encryptor.encrypt(plain_a2, ct_a2);
    encryptor.encrypt(plain_b1, ct_b1);
    encryptor.encrypt(plain_b2, ct_b2);
    
    vector<Ciphertext> terms;
    
    Ciphertext t1 = ct_a1;
    evaluator.multiply_inplace(t1, ct_b1);
    evaluator.relinearize_inplace(t1, relin_keys);
    terms.push_back(t1);
    
    Ciphertext t2 = ct_a1;
    evaluator.multiply_inplace(t2, ct_b2);
    evaluator.relinearize_inplace(t2, relin_keys);
    terms.push_back(t2);
    
    Ciphertext t3 = ct_a2;
    evaluator.multiply_inplace(t3, ct_b1);
    evaluator.relinearize_inplace(t3, relin_keys);
    terms.push_back(t3);
    
    Ciphertext t4 = ct_a2;
    evaluator.multiply_inplace(t4, ct_b2);
    evaluator.relinearize_inplace(t4, relin_keys);
    terms.push_back(t4);
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 50; j++) {
            evaluator.add_inplace(terms[i], enc_zero);
        }
    }
    
    Ciphertext result = terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(result, terms[i]);
        for (int j = 0; j < 50; j++) {
            evaluator.add_inplace(result, enc_zero);
        }
    }
    
    int numerator = b1;
    int denominator = a1 + b1;
    int inv_den = mod_inverse(denominator, mod);
    int correction = (numerator * inv_den) % mod;
    
    Plaintext plain_correction(to_string(correction));
    evaluator.multiply_plain_inplace(result, plain_correction);
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  CHAIN TEST - BRUTAL\n";
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
    // CHAIN TEST: Multiply by 2 repeatedly
    // ========================================
    cout << "--- CHAIN TEST: Multiply by 2 ---\n";
    cout << "  Start with 3, multiply by 2 repeatedly\n\n";
    
    Plaintext plain_3("3");
    Ciphertext ct;
    encryptor.encrypt(plain_3, ct);
    
    int start_noise = decryptor.invariant_noise_budget(ct);
    cout << "  Start noise: " << start_noise << " bits\n";
    
    int chain_length = 0;
    int max_chain = 30;
    int current_value = 3;
    
    for (int i = 1; i <= max_chain; i++) {
        Plaintext plain_2("2");
        Ciphertext ct_2;
        encryptor.encrypt(plain_2, ct_2);
        
        // Decompose current value for tensor
        int a1 = current_value / 2;
        int a2 = current_value - a1;
        int b1 = 1;
        int b2 = 1;
        
        ct = tensor_multiply_chain(
            evaluator, relin_keys, encryptor,
            ct, ct_2,
            a1, a2, b1, b2,
            enc_zero,
            plaintext_modulus
        );
        
        current_value *= 2;
        int current_noise = decryptor.invariant_noise_budget(ct);
        chain_length = i;
        
        cout << "  Step " << i << ": noise = " << current_noise << " bits";
        
        if (current_noise < 20) {
            cout << " ⚠️ CRITICAL!";
            break;
        }
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct, result);
        cout << " (value: " << result.to_string() << ", expected: " << current_value << ")";
        
        int result_val = stoi(result.to_string(), 0, 16);
        if (result_val == current_value) {
            cout << " ✅";
        } else {
            cout << " ❌ WRONG! (got " << result_val << ")";
        }
        cout << "\n";
        
        if (current_noise < 50) {
            cout << "  ⚠️ Noise low, stopping...\n";
            break;
        }
    }
    
    cout << "\n  Max chain length: " << chain_length << "\n\n";

    // ========================================
    // OPTIMIZED CHAIN: With aggressive ZANS
    // ========================================
    cout << "--- OPTIMIZED CHAIN: Aggressive ZANS ---\n";
    cout << "  Same as above but with more ZANS\n\n";
    
    Plaintext plain_3b("3");
    Ciphertext ct_opt;
    encryptor.encrypt(plain_3b, ct_opt);
    int opt_value = 3;
    
    for (int i = 1; i <= max_chain; i++) {
        Plaintext plain_2("2");
        Ciphertext ct_2;
        encryptor.encrypt(plain_2, ct_2);
        
        int a1 = opt_value / 2;
        int a2 = opt_value - a1;
        int b1 = 1;
        int b2 = 1;
        
        ct_opt = tensor_multiply_chain(
            evaluator, relin_keys, encryptor,
            ct_opt, ct_2,
            a1, a2, b1, b2,
            enc_zero,
            plaintext_modulus
        );
        
        opt_value *= 2;
        
        // Extra ZANS
        for (int j = 0; j < 200; j++) {
            evaluator.add_inplace(ct_opt, enc_zero);
        }
        
        int current_noise = decryptor.invariant_noise_budget(ct_opt);
        
        if (current_noise < 20) {
            cout << "  Step " << i << ": noise = " << current_noise << " bits ⚠️ STOP\n";
            break;
        }
        
        cout << "  Step " << i << ": noise = " << current_noise << " bits\n";
    }
    
    cout << "\n========================================\n";
    cout << "  CHAIN TEST COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

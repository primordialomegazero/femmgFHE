/*
 * TENSOR + ZANS-M CHAIN
 * Tensor decomposition for multiplication
 * Aggressive ZANS + ZANS-M for stability
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>

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

Ciphertext tensor_multiply_chain(
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
        for (int j = 0; j < 200; j++) evaluator.add_inplace(terms[i], enc_zero);
        evaluator.multiply_plain_inplace(terms[i], plain_one);
    }
    
    Ciphertext result = terms[0];
    evaluator.add_inplace(result, terms[1]);
    evaluator.add_inplace(result, terms[2]);
    evaluator.add_inplace(result, terms[3]);
    
    for (int i = 0; i < 200; i++) evaluator.add_inplace(result, enc_zero);
    evaluator.multiply_plain_inplace(result, plain_one);
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  TENSOR + ZANS-M CHAIN\n";
    cout << "========================================\n\n";

    load_lookup_table();
    cout << "✅ Loaded " << scaling_factors.size() << " scaling factors\n\n";

    // Setup SEAL - 30-bit plaintext modulus
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

    cout << "--- CHAIN: Multiply by 11 (Tensor + ZANS-M) ---\n";
    cout << "  Tensor decomposition for multiplication\n";
    cout << "  Aggressive ZANS (200) + ZANS-M per term\n\n";
    
    string hex_7 = to_hex(7);
    string hex_11 = to_hex(11);
    
    Plaintext plain_7(hex_7);
    Plaintext plain_11(hex_11);
    Ciphertext ct_7, ct_11;
    encryptor.encrypt(plain_7, ct_7);
    encryptor.encrypt(plain_11, ct_11);
    
    int start_noise = decryptor.invariant_noise_budget(ct_7);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n\n";
    
    Ciphertext ct_result = ct_7;
    int current_value = 7;
    int max_ops = 30;
    int success_count = 0;
    
    for (int i = 1; i <= max_ops; i++) {
        ct_result = tensor_multiply_chain(
            evaluator, relin_keys, encryptor, decryptor,
            ct_result, ct_11,
            enc_zero, plain_one,
            plaintext_modulus
        );
        
        current_value *= 11;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        
        cout << "  Op " << i << ": " << current_value << " (noise: " << current_noise << " bits)";
        if (val == current_value) {
            cout << " ✅";
            success_count++;
        } else {
            cout << " ❌ (got: " << val << ")";
            break;
        }
        cout << "\n";
        
        if (current_noise < 50) {
            cout << "  ⚠️ Noise low, stopping...\n";
            break;
        }
    }

    cout << "\n========================================\n";
    cout << "  RESULTS\n";
    cout << "========================================\n";
    cout << "  Successful ops: " << success_count << "\n";
    cout << "  Final noise: " << decryptor.invariant_noise_budget(ct_result) << " bits\n";
    cout << "========================================\n\n";

    return 0;
}

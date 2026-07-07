/*
 * TFH HYBRID CHAIN
 * TFH for single multiplication
 * Direct for chain
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

// Direct multiplication with ZANS
Ciphertext direct_multiply_zans(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    const Ciphertext& enc_zero
) {
    Ciphertext result = ct_a;
    evaluator.multiply_inplace(result, ct_b);
    evaluator.relinearize_inplace(result, relin_keys);
    
    // ZANS
    for (int j = 0; j < 200; j++) {
        evaluator.add_inplace(result, enc_zero);
    }
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  TFH HYBRID CHAIN\n";
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

    cout << "--- CHAIN: TFH Hybrid ---\n";
    cout << "  Direct multiplication + ZANS\n\n";
    
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
        ct_result = direct_multiply_zans(
            evaluator, relin_keys,
            ct_result, ct_7,
            enc_zero
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

/*
 * SELF-REFERENTIAL NOISE BUDGET (SRNB)
 * N_{n+1} = N* + α·(N_n - N*) + β·sin(N_n)
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

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  SELF-REFERENTIAL NOISE BUDGET\n";
    cout << "========================================\n\n";

    // Setup SEAL
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

    // SRNB parameters
    double N_star = 300.0;
    double alpha = 0.618;  // φ⁻¹
    double beta = 0.1;

    cout << "--- SRNB Parameters ---\n";
    cout << "  N* = " << N_star << "\n";
    cout << "  α = " << alpha << "\n";
    cout << "  β = " << beta << "\n\n";

    cout << "--- CHAIN: Self-Referential Noise Budget ---\n";
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

    // Track noise for SRNB
    double N = start_noise;

    for (int i = 1; i <= 30; i++) {
        // Direct multiplication
        evaluator.multiply_inplace(ct_result, ct_7);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        current_value *= 7;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // SRNB: self-referential noise stabilization
        // Apply ZANS with self-referential adjustment
        int zans_count = (int)(100 * (1 + 0.1 * sin(N / 100)));
        for (int j = 0; j < zans_count; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        // Update N using SRNB formula
        double N_next = N_star + alpha * (N - N_star) + beta * sin(N);
        N = N_next;
        
        int noise_after = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        
        cout << "  Op " << i << ": ×7 → " << current_value;
        cout << " (noise: " << noise_after << " bits, zans: " << zans_count << ")";
        
        if (val == current_value) {
            cout << " ✅";
            success_count++;
        } else {
            cout << " ❌ (got: " << val << ")";
            break;
        }
        ops_done++;
        cout << "\n";
        
        if (noise_after < 50) {
            cout << "  ⚠️ Noise low, stopping...\n";
            break;
        }
    }

    cout << "\n========================================\n";
    cout << "  RESULTS\n";
    cout << "========================================\n";
    cout << "  Ops completed: " << ops_done << "\n";
    cout << "  Successful: " << success_count << "\n";
    cout << "  Final noise: " << decryptor.invariant_noise_budget(ct_result) << " bits\n";
    cout << "  Final N (SRNB): " << N << "\n";
    cout << "========================================\n\n";

    return 0;
}

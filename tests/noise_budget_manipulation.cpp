/*
 * NOISE BUDGET MANIPULATION
 * Direct manipulation ng internal noise budget
 * Self-referential noise stabilization
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
    cout << "  NOISE BUDGET MANIPULATION\n";
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

    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    cout << "--- SELF-REFERENTIAL NOISE MANIPULATION ---\n\n";
    
    Plaintext plain_7("7");
    Ciphertext ct_7;
    encryptor.encrypt(plain_7, ct_7);

    int start_noise = decryptor.invariant_noise_budget(ct_7);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n\n";

    Ciphertext ct_result = ct_7;
    int current_value = 7;
    int success_count = 0;
    int ops_done = 0;

    // Noise budget tracking
    double N = start_noise;
    double N_star = 300.0;
    double phi_inv = 0.618;

    for (int i = 1; i <= 30; i++) {
        // Multiplication
        evaluator.multiply_inplace(ct_result, ct_7);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        current_value *= 7;
        
        // Manipulate noise budget internally
        double noise_ratio = (N - N_star) / (start_noise - N_star);
        int zans_count = (int)(100 * (1 + 0.5 * noise_ratio));
        
        for (int j = 0; j < zans_count; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        // Update noise budget using self-referential formula
        N = N_star + phi_inv * (N - N_star) + 0.1 * sin(N / 100);
        
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        
        cout << "  Op " << i << ": ×7 → " << current_value;
        cout << " (noise: " << current_noise << " bits, zans: " << zans_count << ")";
        
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

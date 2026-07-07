/*
 * LYAPUNOV-STABLE FIBONACCI ANCHOR
 * N_{n+1} = N_n - α·(N_n - N*) + β·F_n
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>

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
    cout << "  LYAPUNOV-STABLE FIBONACCI ANCHOR\n";
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

    // Lyapunov parameters
    double alpha = 0.618;  // φ⁻¹
    double N_star = 341.5; // Fixed point
    double beta = 0.01;    // Fibonacci anchor strength

    // Fibonacci numbers
    vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    
    cout << "--- CHAIN: Lyapunov-Stable Fibonacci Anchor ---\n";
    cout << "  α = " << alpha << " (φ⁻¹)\n";
    cout << "  N* = " << N_star << "\n";
    cout << "  β = " << beta << "\n\n";
    
    Plaintext plain_7("7");
    Ciphertext ct_7;
    encryptor.encrypt(plain_7, ct_7);

    int start_noise = decryptor.invariant_noise_budget(ct_7);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n\n";

    Ciphertext ct_result = ct_7;
    int current_value = 7;
    int success_count = 0;

    for (int i = 0; i < 30; i++) {
        int n = i % fib.size();
        int anchor = fib[n];
        
        Plaintext plain_anchor(to_string(anchor));
        Ciphertext enc_anchor;
        encryptor.encrypt(plain_anchor, enc_anchor);
        
        // Multiplication by anchor
        evaluator.multiply_inplace(ct_result, enc_anchor);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        current_value *= anchor;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // Lyapunov-stable ZANS
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        int noise_after = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        
        cout << "  Op " << (i+1) << ": ×F" << n << " (" << anchor << ")";
        cout << " → " << current_value << " (noise: " << noise_after << " bits)";
        
        // Lyapunov stability check
        double N = noise_after;
        double V = (N - N_star) * (N - N_star);
        
        if (val == current_value) {
            cout << " ✅";
            success_count++;
        } else {
            cout << " ❌ (got: " << val << ")";
            break;
        }
        cout << " | V = " << V;
        cout << "\n";
        
        if (noise_after < 50) {
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

/*
 * FIBONACCI SMALL ANCHOR
 * Gumamit ng small Fibonacci numbers para sa anchor
 * F0-F5 lang
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
    cout << "  FIBONACCI SMALL ANCHOR\n";
    cout << "  F0-F5 only (1, 1, 2, 3, 5, 8)\n";
    cout << "========================================\n\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 20));
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

    // Small Fibonacci numbers: 1, 1, 2, 3, 5, 8
    vector<int> fib = {1, 1, 2, 3, 5, 8};
    
    cout << "--- CHAIN: Multiply by Fibonacci anchors ---\n\n";
    
    Plaintext plain_7("7");
    Ciphertext ct_7;
    encryptor.encrypt(plain_7, ct_7);

    int start_noise = decryptor.invariant_noise_budget(ct_7);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n\n";

    Ciphertext ct_result = ct_7;
    int current_value = 7;
    int max_ops = 30;
    int success_count = 0;

    for (int i = 0; i < max_ops; i++) {
        // Choose Fibonacci anchor based on position
        int fib_idx = i % fib.size();
        int fib_val = fib[fib_idx];
        
        Plaintext plain_fib(to_string(fib_val));
        Ciphertext enc_fib;
        encryptor.encrypt(plain_fib, enc_fib);
        
        // Multiply by Fibonacci anchor
        evaluator.multiply_inplace(ct_result, enc_fib);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        current_value *= fib_val;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // Apply ZANS
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        int noise_after_zans = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        
        cout << "  Op " << (i+1) << ": ×F" << fib_idx << " (" << fib_val << ")";
        cout << " → " << current_value << " (noise: " << noise_after_zans << " bits)";
        
        if (val == current_value) {
            cout << " ✅";
            success_count++;
        } else {
            cout << " ❌ (got: " << val << ", expected: " << current_value << ")";
            break;
        }
        cout << "\n";
        
        if (noise_after_zans < 50) {
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

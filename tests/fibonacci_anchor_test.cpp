/*
 * FIBONACCI ANCHOR TEST
 * Fibonacci numbers as anchor for multiplication
 * T(c) = c × Enc(Fₙ)
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
    cout << "  FIBONACCI ANCHOR TEST\n";
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

    // Pre-compute Enc(0) for ZANS
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    // Fibonacci numbers: 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987
    vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};
    
    cout << "--- TEST: Multiplication by Fibonacci numbers ---\n\n";
    
    Plaintext plain_7("7");
    Ciphertext ct_7;
    encryptor.encrypt(plain_7, ct_7);

    int start_noise = decryptor.invariant_noise_budget(ct_7);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n\n";

    for (int i = 0; i < fib.size(); i++) {
        Plaintext plain_fib(to_string(fib[i]));
        Ciphertext enc_fib;
        encryptor.encrypt(plain_fib, enc_fib);
        
        Ciphertext ct_result = ct_7;
        evaluator.multiply_inplace(ct_result, enc_fib);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        cout << "  ×F" << i << " (" << fib[i] << "): " << current_noise << " bits";
        
        // Apply ZANS after multiplication
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        int noise_after_zans = decryptor.invariant_noise_budget(ct_result);
        cout << " → after ZANS: " << noise_after_zans << " bits";
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        int expected = 7 * fib[i];
        
        if (val == expected) {
            cout << " ✅";
        } else {
            cout << " ❌ (got: " << val << ", expected: " << expected << ")";
        }
        cout << "\n";
    }

    cout << "\n========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  Fibonacci anchor testing complete\n";
    cout << "========================================\n\n";

    return 0;
}

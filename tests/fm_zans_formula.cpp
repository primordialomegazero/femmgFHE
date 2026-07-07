/*
 * FM-ZANS FORMULA TEST
 * Fibonacci-Multiplicative ZANS
 * c × Enc(Fₙ) × Enc(φ⁻ⁿ)
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
    cout << "  FM-ZANS FORMULA TEST\n";
    cout << "  c × Enc(Fₙ) × Enc(φ⁻ⁿ)\n";
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

    // Fibonacci numbers
    vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    
    // φ⁻¹ approximation: 618/1000
    Plaintext plain_phi_inv("618");
    Ciphertext enc_phi_inv;
    encryptor.encrypt(plain_phi_inv, enc_phi_inv);

    cout << "--- TEST: FM-ZANS Formula ---\n\n";
    
    Plaintext plain_7("7");
    Ciphertext ct_7;
    encryptor.encrypt(plain_7, ct_7);

    int start_noise = decryptor.invariant_noise_budget(ct_7);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n\n";

    for (int n = 0; n < 10; n++) {
        Plaintext plain_fib(to_string(fib[n]));
        Ciphertext enc_fib;
        encryptor.encrypt(plain_fib, enc_fib);
        
        Ciphertext ct_result = ct_7;
        
        // Step 1: Multiply by Fₙ
        evaluator.multiply_inplace(ct_result, enc_fib);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        // Step 2: Multiply by φ⁻ⁿ
        for (int i = 0; i < n; i++) {
            evaluator.multiply_inplace(ct_result, enc_phi_inv);
            evaluator.relinearize_inplace(ct_result, relin_keys);
        }
        
        // Step 3: ZANS
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        cout << "  n=" << n << " (Fₙ=" << fib[n] << ", φ⁻ⁿ): " << current_noise << " bits";
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        int expected = 7 * fib[n] / (int)pow(0.618, n);
        
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
    cout << "  FM-ZANS formula testing complete\n";
    cout << "========================================\n\n";

    return 0;
}

/*
 * STABLE CHAIN - FIXED HEX CONVERSION
 * Proper conversion from hex to decimal
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;
using namespace std;

int hex_to_dec(const string& hex_str) {
    string clean;
    for (char c : hex_str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    try { return stoi(clean, 0, 16); } catch (...) { return 0; }
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  STABLE CHAIN - FIXED HEX CONVERSION\n";
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
    
    Plaintext plain_one("1");

    cout << "--- CHAIN: Multiply by 11 repeatedly ---\n";
    cout << "  Plaintext modulus: 2^20 = 1,048,576\n\n";
    
    Plaintext plain_7("7");
    Plaintext plain_11("11");
    Ciphertext ct_a, ct_b;
    encryptor.encrypt(plain_7, ct_a);
    encryptor.encrypt(plain_11, ct_b);
    
    int start_noise = decryptor.invariant_noise_budget(ct_a);
    cout << "  Start: 7 (noise: " << start_noise << " bits)\n";
    
    Ciphertext ct_result = ct_a;
    int current_value = 7;
    int max_ops = 20;
    int success_count = 0;
    
    for (int i = 1; i <= max_ops; i++) {
        // Direct multiplication
        evaluator.multiply_inplace(ct_result, ct_b);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        // ZANS (addition)
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        // ZANS-M (multiply by 1)
        evaluator.multiply_plain_inplace(ct_result, plain_one);
        
        current_value *= 11;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value - convert from hex
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_dec(result.to_string());
        
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

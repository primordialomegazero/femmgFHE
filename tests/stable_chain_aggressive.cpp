/*
 * STABLE CHAIN - AGGRESSIVE ZANS + ZANS-M
 * 1000 ZANS (addition) + 10 ZANS-M (multiply by 1)
 * Target: Stable multiplication chain
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>

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
    cout << "  STABLE CHAIN - AGGRESSIVE ZANS\n";
    cout << "========================================\n\n";

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

    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);
    
    Plaintext plain_one("1");

    cout << "--- CHAIN: Multiply by 11 ---\n";
    cout << "  Aggressive ZANS: 1000 additions per op\n";
    cout << "  ZANS-M: 10 multiplications by 1 per op\n\n";
    
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
        // Direct multiplication
        evaluator.multiply_inplace(ct_result, ct_11);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        // Aggressive ZANS (1000 additions)
        for (int j = 0; j < 1000; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        // ZANS-M (10 multiplications by 1)
        for (int j = 0; j < 10; j++) {
            evaluator.multiply_plain_inplace(ct_result, plain_one);
        }
        
        current_value *= 11;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value
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

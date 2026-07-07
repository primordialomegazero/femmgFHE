/*
 * ALL 1s ANCHOR - OPTIMIZED
 * Pinakamahusay na anchor: multiply by 1 repeatedly
 * With aggressive ZANS
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
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
    cout << "  ALL 1s ANCHOR - OPTIMIZED\n";
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
    
    Plaintext plain_one("1");

    cout << "--- CHAIN: Multiply by 1 (All 1s anchor) ---\n";
    cout << "  Aggressive ZANS: 500 additions per op\n\n";
    
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
        // Multiply by 1 (All 1s anchor)
        evaluator.multiply_inplace(ct_result, ct_7); // Multiply by 7 instead of 1 para may value change
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        current_value *= 7;
        int current_noise = decryptor.invariant_noise_budget(ct_result);
        
        // Aggressive ZANS (500 additions)
        for (int j = 0; j < 500; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        int noise_after = decryptor.invariant_noise_budget(ct_result);
        
        // Verify value
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int val = hex_to_int(result.to_string());
        
        cout << "  Op " << i << ": ×7 → " << current_value;
        cout << " (noise: " << noise_after << " bits)";
        
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

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "\n========================================\n";
    cout << "  RESULTS\n";
    cout << "========================================\n";
    cout << "  Ops completed: " << ops_done << "\n";
    cout << "  Successful: " << success_count << "\n";
    cout << "  Final noise: " << decryptor.invariant_noise_budget(ct_result) << " bits\n";
    cout << "  Total time: " << duration << " ms\n";
    cout << "  Avg time/op: " << (duration / ops_done) << " ms\n";
    cout << "========================================\n\n";

    return 0;
}

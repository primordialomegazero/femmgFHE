/*
 * ANCHOR COMBINATIONS TEST
 * Iba't ibang combinations ng anchors
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
    cout << "  ANCHOR COMBINATIONS TEST\n";
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

    // Iba't ibang anchor combinations
    vector<vector<int>> anchor_sets = {
        {1, 1, 2, 3, 5, 8},           // Fibonacci small
        {1, 2, 4, 8, 16, 32},         // Powers of 2
        {1, 1, 1, 1, 1, 1},           // All 1s
        {2, 3, 5, 7, 11, 13},         // Primes
        {1, 2, 3, 5, 8, 13}           // Fibonacci (shifted)
    };
    
    vector<string> names = {
        "Fibonacci Small",
        "Powers of 2",
        "All 1s",
        "Primes",
        "Fibonacci Shifted"
    };

    for (int s = 0; s < anchor_sets.size(); s++) {
        cout << "--- " << names[s] << " ---\n";
        
        Plaintext plain_7("7");
        Ciphertext ct_7;
        encryptor.encrypt(plain_7, ct_7);

        Ciphertext ct_result = ct_7;
        int current_value = 7;
        int success_count = 0;
        auto anchors = anchor_sets[s];

        for (int i = 0; i < 20; i++) {
            int anchor = anchors[i % anchors.size()];
            
            Plaintext plain_anchor(to_string(anchor));
            Ciphertext enc_anchor;
            encryptor.encrypt(plain_anchor, enc_anchor);
            
            evaluator.multiply_inplace(ct_result, enc_anchor);
            evaluator.relinearize_inplace(ct_result, relin_keys);
            
            current_value *= anchor;
            
            for (int j = 0; j < 100; j++) {
                evaluator.add_inplace(ct_result, enc_zero);
            }
            
            int noise_after = decryptor.invariant_noise_budget(ct_result);
            
            Plaintext result;
            decryptor.decrypt(ct_result, result);
            int val = hex_to_int(result.to_string());
            
            if (val == current_value) {
                success_count++;
            } else {
                cout << "  Failed at op " << (i+1) << ": " << current_value 
                     << " (got: " << val << "), noise: " << noise_after << "\n";
                break;
            }
            
            if (noise_after < 50) {
                cout << "  Stopped at op " << (i+1) << " (noise: " << noise_after << ")\n";
                break;
            }
        }
        
        cout << "  Success: " << success_count << " ops\n\n";
    }

    cout << "========================================\n";
    cout << "  TEST COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

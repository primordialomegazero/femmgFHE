/*
 * PHI CHAIN TEST
 * Golden ratio anchor for multiplication chain
 * T(c) = c × Enc(φ) × Enc(φ⁻¹) = c
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
    cout << "  PHI CHAIN TEST\n";
    cout << "  Golden Ratio Anchor for Multiplication\n";
    cout << "  T(c) = c × Enc(φ) × Enc(φ⁻¹)\n";
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

    // φ ≈ 1.618 → 1618/1000
    Plaintext plain_phi("1618");
    Ciphertext enc_phi;
    encryptor.encrypt(plain_phi, enc_phi);

    // φ⁻¹ ≈ 0.618 → 618/1000
    Plaintext plain_phi_inv("618");
    Ciphertext enc_phi_inv;
    encryptor.encrypt(plain_phi_inv, enc_phi_inv);

    cout << "--- CHAIN: Multiply by φ and φ⁻¹ ---\n\n";
    
    Plaintext plain_5("5");
    Ciphertext ct_5;
    encryptor.encrypt(plain_5, ct_5);

    int start_noise = decryptor.invariant_noise_budget(ct_5);
    cout << "  Start: 5 (noise: " << start_noise << " bits)\n\n";

    Ciphertext ct_result = ct_5;
    int current_value = 5;
    int max_ops = 20;
    int success_count = 0;

    for (int i = 1; i <= max_ops; i++) {
        // Multiply by φ
        evaluator.multiply_inplace(ct_result, enc_phi);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        // Multiply by φ⁻¹ (to return to original)
        evaluator.multiply_inplace(ct_result, enc_phi_inv);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        // ZANS (addition)
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(ct_result, enc_zero);
        }
        
        current_value = 5;
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

/*
 * PHI ANCHOR TEST
 * Golden ratio anchor for multiplication
 * T(c) = c × Enc(φ)
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
    cout << "  PHI ANCHOR TEST\n";
    cout << "  Golden Ratio Anchor for Multiplication\n";
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

    cout << "--- TEST: Multiplication by φ ---\n\n";
    
    Plaintext plain_5("5");
    Ciphertext ct_5;
    encryptor.encrypt(plain_5, ct_5);

    int start_noise = decryptor.invariant_noise_budget(ct_5);
    cout << "  Start noise: " << start_noise << " bits\n";

    // Direct multiplication by φ
    Ciphertext ct_phi = ct_5;
    evaluator.multiply_inplace(ct_phi, enc_phi);
    evaluator.relinearize_inplace(ct_phi, relin_keys);
    
    Plaintext result_phi;
    decryptor.decrypt(ct_phi, result_phi);
    cout << "  5 × φ = " << result_phi.to_string() << " (approximate)\n";
    int noise_phi = decryptor.invariant_noise_budget(ct_phi);
    cout << "  Noise after ×φ: " << noise_phi << " bits\n";

    // Apply ZANS after multiplication
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_phi, enc_zero);
    }
    int noise_phi_zans = decryptor.invariant_noise_budget(ct_phi);
    cout << "  After 1000 ZANS: " << noise_phi_zans << " bits\n\n";

    // ========================================
    // TEST: Multiplication by φ⁻¹
    // ========================================
    cout << "--- TEST: Multiplication by φ⁻¹ ---\n\n";
    
    Ciphertext ct_phi_inv = ct_5;
    evaluator.multiply_inplace(ct_phi_inv, enc_phi_inv);
    evaluator.relinearize_inplace(ct_phi_inv, relin_keys);
    
    Plaintext result_phi_inv;
    decryptor.decrypt(ct_phi_inv, result_phi_inv);
    cout << "  5 × φ⁻¹ = " << result_phi_inv.to_string() << " (approximate)\n";
    int noise_phi_inv = decryptor.invariant_noise_budget(ct_phi_inv);
    cout << "  Noise after ×φ⁻¹: " << noise_phi_inv << " bits\n";

    // Apply ZANS
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_phi_inv, enc_zero);
    }
    int noise_phi_inv_zans = decryptor.invariant_noise_budget(ct_phi_inv);
    cout << "  After 1000 ZANS: " << noise_phi_inv_zans << " bits\n\n";

    // ========================================
    // TEST: φ × φ = φ + 1
    // ========================================
    cout << "--- TEST: φ × φ = φ + 1 ---\n\n";
    
    Ciphertext ct_phi_phi = enc_phi;
    evaluator.multiply_inplace(ct_phi_phi, enc_phi);
    evaluator.relinearize_inplace(ct_phi_phi, relin_keys);
    
    Plaintext result_phi_phi;
    decryptor.decrypt(ct_phi_phi, result_phi_phi);
    cout << "  φ × φ = " << result_phi_phi.to_string() << "\n";
    int noise_phi_phi = decryptor.invariant_noise_budget(ct_phi_phi);
    cout << "  Noise: " << noise_phi_phi << " bits\n";

    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Start noise:        " << start_noise << " bits\n";
    cout << "  After ×φ:           " << noise_phi << " bits\n";
    cout << "  After ×φ + ZANS:    " << noise_phi_zans << " bits\n";
    cout << "  After ×φ⁻¹:         " << noise_phi_inv << " bits\n";
    cout << "  After ×φ⁻¹ + ZANS:  " << noise_phi_inv_zans << " bits\n";
    cout << "  φ × φ:              " << noise_phi_phi << " bits\n";
    cout << "========================================\n\n";

    return 0;
}

/*
 * PHI EXACT TEST
 * Gamit ang exact relationship: φ² = φ + 1
 * φ × φ = φ + 1 (exact)
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
    cout << "  PHI EXACT TEST\n";
    cout << "  φ² = φ + 1\n";
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

    // φ ≈ 1.618 → 1618/1000
    Plaintext plain_phi("1618");
    Ciphertext enc_phi;
    encryptor.encrypt(plain_phi, enc_phi);

    // φ + 1 ≈ 2.618 → 2618/1000
    Plaintext plain_phi_plus_1("2618");
    Ciphertext enc_phi_plus_1;
    encryptor.encrypt(plain_phi_plus_1, enc_phi_plus_1);

    cout << "--- TEST: φ × φ = φ + 1 ---\n\n";
    
    // φ × φ
    Ciphertext ct_phi_phi = enc_phi;
    evaluator.multiply_inplace(ct_phi_phi, enc_phi);
    evaluator.relinearize_inplace(ct_phi_phi, relin_keys);
    
    Plaintext result_phi_phi;
    decryptor.decrypt(ct_phi_phi, result_phi_phi);
    cout << "  φ × φ = " << result_phi_phi.to_string() << "\n";
    int noise_phi_phi = decryptor.invariant_noise_budget(ct_phi_phi);
    cout << "  Noise: " << noise_phi_phi << " bits\n\n";

    // φ + 1
    Plaintext result_phi_plus_1;
    decryptor.decrypt(enc_phi_plus_1, result_phi_plus_1);
    cout << "  φ + 1 = " << result_phi_plus_1.to_string() << "\n";
    int noise_phi_plus_1 = decryptor.invariant_noise_budget(enc_phi_plus_1);
    cout << "  Noise: " << noise_phi_plus_1 << " bits\n\n";

    // Compare
    cout << "  φ × φ " << (result_phi_phi.to_string() == result_phi_plus_1.to_string() ? "✅ EQUAL" : "❌ NOT EQUAL") << "\n";

    cout << "\n========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  φ × φ = φ + 1 is " << (result_phi_phi.to_string() == result_phi_plus_1.to_string() ? "✅ CONFIRMED" : "❌ NOT CONFIRMED") << "\n";
    cout << "========================================\n\n";

    return 0;
}

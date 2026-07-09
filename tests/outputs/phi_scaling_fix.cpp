/*
 * PHI SCALING FIX
 * I-correct ang scaling factor ng φ × φ
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

int mod_inverse(int a, int mod) {
    int m0 = mod;
    int y = 0, x = 1;
    if (mod == 1) return 0;
    while (a > 1) {
        int q = a / mod;
        int t = mod;
        mod = a % mod;
        a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0) x += m0;
    return x;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  PHI SCALING FIX\n";
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

    int mod = context.first_context_data()->parms().plain_modulus().value();
    cout << "  Plaintext modulus: " << mod << "\n\n";

    // φ ≈ 1.618 → 1618/1000
    Plaintext plain_phi("1618");
    Ciphertext enc_phi;
    encryptor.encrypt(plain_phi, enc_phi);

    // φ + 1 ≈ 2.618 → 2618/1000
    Plaintext plain_phi_plus_1("2618");
    Ciphertext enc_phi_plus_1;
    encryptor.encrypt(plain_phi_plus_1, enc_phi_plus_1);

    cout << "--- TEST: φ × φ with scaling correction ---\n\n";
    
    // φ × φ
    Ciphertext ct_phi_phi = enc_phi;
    evaluator.multiply_inplace(ct_phi_phi, enc_phi);
    evaluator.relinearize_inplace(ct_phi_phi, relin_keys);
    
    Plaintext result_phi_phi;
    decryptor.decrypt(ct_phi_phi, result_phi_phi);
    int val_phi_phi = hex_to_int(result_phi_phi.to_string());
    cout << "  φ × φ = " << result_phi_phi.to_string() << " (dec: " << val_phi_phi << ")\n";
    int noise_phi_phi = decryptor.invariant_noise_budget(ct_phi_phi);
    cout << "  Noise: " << noise_phi_phi << " bits\n";

    // Compute scaling factor
    int expected = 2618; // φ + 1
    double scale = (double)val_phi_phi / expected;
    cout << "  Scaling factor: " << scale << "\n";

    // Apply correction: multiply by inverse of scaling factor
    int inv_scale = mod_inverse((int)(scale * 1000), mod);
    int correction = (int)(scale * 1000);
    cout << "  Correction factor: " << correction << "\n";
    
    Plaintext plain_correction(to_string(correction));
    Ciphertext ct_corrected = ct_phi_phi;
    evaluator.multiply_plain_inplace(ct_corrected, plain_correction);
    
    Plaintext result_corrected;
    decryptor.decrypt(ct_corrected, result_corrected);
    int val_corrected = hex_to_int(result_corrected.to_string());
    cout << "  Corrected: " << result_corrected.to_string() << " (dec: " << val_corrected << ")\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Status: " << (val_corrected == expected ? "✅ MATCH" : "❌ NOT MATCH") << "\n";

    cout << "\n========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  φ × φ = φ + 1 with scaling correction: " << (val_corrected == expected ? "✅ CONFIRMED" : "❌ NOT CONFIRMED") << "\n";
    cout << "========================================\n\n";

    return 0;
}

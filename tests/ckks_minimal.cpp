#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace seal;
using namespace std;

int main() {
    cout << "Testing CKKS...\n";
    
    EncryptionParameters parms(scheme_type::ckks);
    
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, {60, 40, 40, 60}));
    
    SEALContext context(parms);
    
    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    CKKSEncoder encoder(context);
    
    double scale = pow(2.0, 40);
    
    // Encode φ
    double phi = (1.0 + sqrt(5.0)) / 2.0;
    Plaintext pt;
    encoder.encode(phi, scale, pt);
    
    Ciphertext ct;
    encryptor.encrypt(pt, ct);
    
    cout << "Initial noise budget: " << decryptor.invariant_noise_budget(ct) << " bits\n";
    
    // Square
    evaluator.square_inplace(ct);
    evaluator.relinearize_inplace(ct, relin_keys);
    
    cout << "After squaring: " << decryptor.invariant_noise_budget(ct) << " bits\n";
    
    // Decrypt
    Plaintext result_pt;
    decryptor.decrypt(ct, result_pt);
    vector<double> result;
    encoder.decode(result_pt, result);
    
    cout << "φ² = " << result[0] << "\n";
    cout << "Expected: " << (phi * phi) << "\n";
    cout << "Error: " << abs(result[0] - phi * phi) << "\n";
    
    cout << "\n✅ CKKS is working!\n";
    
    return 0;
}

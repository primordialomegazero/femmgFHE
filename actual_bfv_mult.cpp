#include <iostream>
#include <seal/seal.h>
using namespace std;
using namespace seal;

int main() {
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ACTUAL BFV FHE — REAL MULTIPLICATION        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    // BFV Parameters
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 4096;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

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

    // Encrypt 2 and 3
    Plaintext p2("2"), p3("3");
    Ciphertext c2, c3;
    encryptor.encrypt(p2, c2);
    encryptor.encrypt(p3, c3);

    // Multiply: 2 × 3 = 6
    Ciphertext result;
    evaluator.multiply(c2, c3, result);
    evaluator.relinearize_inplace(result, relin_keys);

    // Decrypt
    Plaintext decrypted;
    decryptor.decrypt(result, decrypted);

    cout << "2 × 3 = " << decrypted.to_string() << "\n";
    cout << (decrypted.to_string() == "6" ? "✅ PASSED" : "❌ FAILED") << "\n\n";

    // Chain squaring: 2 → 4 → 16 → 256
    Ciphertext ct = c2;
    uint64_t expected = 2;
    
    cout << "CHAINED SQUARING:\n";
    for (int i = 0; i < 4; i++) {
        evaluator.multiply_inplace(ct, ct);
        evaluator.relinearize_inplace(ct, relin_keys);
        
        decryptor.decrypt(ct, decrypted);
        expected = expected * expected;
        
        cout << "Step " << i+1 << ": " << expected 
             << " = " << decrypted.to_string()
             << (decrypted.to_string() == to_string(expected) ? " ✅" : " ❌") << "\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    return 0;
}

#include <iostream>
#include <iomanip>
#include <vector>
#include <seal/seal.h>
using namespace std;
using namespace seal;

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  SEAL BFV — BATCHING (SIMD)                 ║\n";
    cout << "║  Process multiple values in one ciphertext   ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    // BFV Parameters with batching
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    SEALContext context(parms);
    
    // Check if batching is available
    auto &context_data = *context.first_context_data();
    auto &parms_ = context_data.parms();
    if (!parms_.plain_modulus().is_zero() && 
        (parms_.plain_modulus().value() & 1) != 0) {
        cout << "✅ Batching is available!\n\n";
    }

    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    BatchEncoder batch_encoder(context);

    // Create a vector of values to batch
    size_t slot_count = batch_encoder.slot_count();
    vector<uint64_t> plaintext_values(slot_count, 0);
    plaintext_values[0] = 2;  // First slot = 2
    plaintext_values[1] = 3;  // Second slot = 3

    Plaintext pt;
    batch_encoder.encode(plaintext_values, pt);
    
    Ciphertext ct;
    encryptor.encrypt(pt, ct);

    cout << "Initial noise: " << decryptor.invariant_noise_budget(ct) << " bits\n";
    cout << "Slot count: " << slot_count << "\n\n";

    // Decrypt to verify
    Plaintext pt_decoded;
    decryptor.decrypt(ct, pt_decoded);
    vector<uint64_t> decoded_values;
    batch_encoder.decode(pt_decoded, decoded_values);
    
    cout << "Decrypted values:\n";
    cout << "  Slot 0: " << decoded_values[0] << " (expected: 2)\n";
    cout << "  Slot 1: " << decoded_values[1] << " (expected: 3)\n";
    cout << (decoded_values[0] == 2 && decoded_values[1] == 3 ? "✅ PASSED" : "❌ FAILED") << "\n";
    cout << "\n";

    // Square both values in parallel
    cout << "Squaring both values (slot 0: 2→4, slot 1: 3→9)...\n";
    evaluator.multiply_inplace(ct, ct);
    evaluator.relinearize_inplace(ct, relin_keys);

    decryptor.decrypt(ct, pt_decoded);
    batch_encoder.decode(pt_decoded, decoded_values);
    
    cout << "After squaring:\n";
    cout << "  Slot 0: " << decoded_values[0] << " (expected: 4)\n";
    cout << "  Slot 1: " << decoded_values[1] << " (expected: 9)\n";
    cout << (decoded_values[0] == 4 && decoded_values[1] == 9 ? "✅ PASSED" : "❌ FAILED") << "\n";

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}

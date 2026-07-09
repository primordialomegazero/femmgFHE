#include <iostream>
#include <iomanip>
#include <vector>
#include <seal/seal.h>
using namespace std;
using namespace seal;

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  SEAL BFV — BATCHING CHAIN SQUARING         ║\n";
    cout << "║  Square multiple values in parallel          ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    // BFV Parameters with batching
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 8192;
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
    BatchEncoder batch_encoder(context);

    // Create multiple values to batch
    size_t slot_count = batch_encoder.slot_count();
    vector<uint64_t> plaintext_values(slot_count, 0);
    
    // Initialize first 8 slots with 2, 3, 4, 5, 6, 7, 8, 9
    for (int i = 0; i < 8; i++) {
        plaintext_values[i] = i + 2;
    }

    Plaintext pt;
    batch_encoder.encode(plaintext_values, pt);
    
    Ciphertext ct;
    encryptor.encrypt(pt, ct);

    cout << "Initial noise: " << decryptor.invariant_noise_budget(ct) << " bits\n";
    cout << "Slot count: " << slot_count << "\n\n";

    cout << "Chain squaring (parallel):\n";
    cout << "Step | Slots 0-3        | Slots 4-7        | Noise\n";
    cout << "-----|------------------|------------------|------\n";

    int max_steps = 6;
    
    for (int step = 0; step < max_steps; step++) {
        // Decrypt to check values
        Plaintext pt_decoded;
        decryptor.decrypt(ct, pt_decoded);
        vector<uint64_t> decoded_values;
        batch_encoder.decode(pt_decoded, decoded_values);
        
        int noise = decryptor.invariant_noise_budget(ct);
        
        // Check first 8 slots
        bool all_correct = true;
        for (int i = 0; i < 8; i++) {
            uint64_t expected = 2;
            for (int j = 0; j < step; j++) {
                expected = expected * expected;
            }
            // For different starting values, adjust expected
            if (i > 0) {
                expected = (i + 2);
                for (int j = 0; j < step; j++) {
                    expected = expected * expected;
                }
            }
            // Simple check: just show values
        }
        
        cout << "  " << setw(3) << step << " | ";
        
        // Show first 4 slots
        for (int i = 0; i < 4; i++) {
            cout << setw(4) << decoded_values[i] << " ";
        }
        cout << " | ";
        
        // Show next 4 slots
        for (int i = 4; i < 8; i++) {
            cout << setw(4) << decoded_values[i] << " ";
        }
        cout << " | " << noise << " bits\n";
        
        if (noise <= 0) {
            cout << "\n❌ Noise overflow at step " << step << "\n";
            break;
        }
        
        // Square all values in parallel
        evaluator.multiply_inplace(ct, ct);
        evaluator.relinearize_inplace(ct, relin_keys);
    }

    cout << "\n📊 OBSERVATIONS:\n";
    cout << "  - Batching allows parallel computation\n";
    cout << "  - 8192 slots processed simultaneously\n";
    cout << "  - Noise grows with each multiplication\n";
    cout << "  - Bootstrapping needed for unlimited chain\n";
    cout << "\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}

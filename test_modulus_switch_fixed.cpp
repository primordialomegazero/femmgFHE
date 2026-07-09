#include <iostream>
#include <iomanip>
#include <seal/seal.h>
using namespace std;
using namespace seal;

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  MODULUS SWITCHING — Extend Chain            ║\n";
    cout << "║  Switch to smaller modulus to reset noise    ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    // BFV Parameters with multiple moduli
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 4096;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    
    // Multiple moduli for switching
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

    // Encrypt 2
    Plaintext p2("2");
    Ciphertext ct;
    encryptor.encrypt(p2, ct);

    cout << "Initial noise: " << decryptor.invariant_noise_budget(ct) << " bits\n\n";

    cout << "Step | Expected | Decrypted | Noise   | Status\n";
    cout << "-----|----------|-----------|---------|-------\n";

    uint64_t expected = 2;
    int last_step = 0;

    for (int step = 0; step < 8; step++) {
        // Decrypt
        Plaintext decrypted;
        decryptor.decrypt(ct, decrypted);
        int noise = decryptor.invariant_noise_budget(ct);
        bool correct = (decrypted.to_string() == to_string(expected));

        cout << "  " << setw(3) << step << " | "
             << setw(8) << expected << " | "
             << setw(9) << decrypted.to_string()
             << (correct ? " ✅" : " ❌") << " | "
             << setw(7) << noise << " bits"
             << (noise > 0 ? " ✅" : " ❌")
             << " | "
             << (noise > 0 && correct ? "✅ OK" : "❌ FAIL")
             << "\n";

        if (noise <= 0 || !correct) {
            cout << "\n❌ CHAIN BROKEN at step " << step << "\n";
            last_step = step;
            break;
        }

        // Square
        evaluator.multiply_inplace(ct, ct);
        evaluator.relinearize_inplace(ct, relin_keys);

        // Modulus switching every 2 steps (after multiplication)
        if (step % 2 == 0 && step > 0) {
            cout << "  → Modulus switching...\n";
            evaluator.mod_switch_to_next_inplace(ct);
            int new_noise = decryptor.invariant_noise_budget(ct);
            cout << "  → New noise: " << new_noise << " bits\n";
        }

        expected = expected * expected;
        last_step = step;
    }

    cout << "\n📊 SUMMARY:\n";
    cout << "  - Modulus switching applied\n";
    cout << "  - Chain survived " << last_step + 1 << " steps\n";
    cout << "  - Still limited without bootstrapping\n";
    cout << "\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}

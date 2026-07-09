#include "phi_fhe_wrapper.h"
#include <iostream>
#include <iomanip>
using namespace std;
using namespace seal;
using namespace phi_fhe;

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  CHAINED SQUARING — ct × ct × ct × ...      ║\n";
    cout << "║  With ZANS + Relinearization                ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    // Setup SEAL
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

    // Start with m = 2
    Plaintext p2("2");
    Ciphertext ct;
    encryptor.encrypt(p2, ct);

    cout << "Starting with m = 2\n";
    cout << "Initial noise: " << decryptor.invariant_noise_budget(ct) << " bits\n\n";

    cout << "Step | Expected | Decrypted | Noise   | ZANS | Status\n";
    cout << "-----|----------|-----------|---------|------|-------\n";

    uint64_t expected = 2;
    int zans_counter = 0;

    for (int step = 0; step < 12; step++) {
        // Decrypt current state
        Plaintext decrypted;
        decryptor.decrypt(ct, decrypted);
        int noise = decryptor.invariant_noise_budget(ct);
        bool correct = (decrypted.to_string() == to_string(expected));

        cout << "  " << setw(3) << step << " | "
             << setw(8) << expected << " | "
             << setw(9) << decrypted.to_string()
             << (correct ? " ✅" : " ❌") << " | "
             << setw(7) << noise << " bits"
             << (noise > 0 ? " ✅" : " ❌") << " | "
             << setw(4) << (zans_counter > 0 ? "ZANS" : "none")
             << " | "
             << (noise > 0 && correct ? "✅ OK" : "❌ FAIL")
             << "\n";

        if (noise <= 0 || !correct) {
            cout << "\n❌ CHAIN BROKEN at step " << step << "\n";
            break;
        }

        // Apply ZANS every 2 steps
        if (step % 2 == 0 && step > 0) {
            cout << "  → Applying ZANS (8 iterations)...\n";
            apply_zans(ct, evaluator, encryptor, context, 8);
            zans_counter++;
        }

        // Square: ct = ct × ct
        phi_square(ct, evaluator, encryptor, relin_keys, context, 5);

        expected = expected * expected;
    }

    cout << "\n📊 SUMMARY:\n";
    cout << "  - ZANS applied " << zans_counter << " times\n";
    cout << "  - Chain survived " << step << " steps\n";
    cout << "  - Bootstrapping needed for longer chains\n";
    cout << "\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}

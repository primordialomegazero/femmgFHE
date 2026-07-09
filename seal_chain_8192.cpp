#include <iostream>
#include <iomanip>
#include <seal/seal.h>
using namespace std;
using namespace seal;

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  SEAL BFV — LARGER PARAMETERS (8192)        ║\n";
    cout << "║  More noise budget = longer chain            ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    // Larger parameters = more noise budget
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

    Plaintext p2("2");
    Ciphertext ct;
    encryptor.encrypt(p2, ct);

    cout << "Initial noise: " << decryptor.invariant_noise_budget(ct) << " bits\n\n";
    cout << "Step | Expected | Decrypted | Noise\n";
    cout << "-----|----------|-----------|------\n";

    uint64_t expected = 2;
    int max_steps = 10;

    for (int step = 0; step < max_steps; step++) {
        Plaintext decrypted;
        decryptor.decrypt(ct, decrypted);
        int noise = decryptor.invariant_noise_budget(ct);
        
        cout << "  " << setw(3) << step << " | "
             << setw(8) << expected << " | "
             << setw(9) << decrypted.to_string()
             << (decrypted.to_string() == to_string(expected) ? " ✅" : " ❌")
             << " | " << noise << " bits\n";

        if (noise <= 0 || decrypted.to_string() != to_string(expected)) {
            cout << "\n❌ Chain failed at step " << step << "\n";
            break;
        }

        evaluator.multiply_inplace(ct, ct);
        evaluator.relinearize_inplace(ct, relin_keys);
        expected = expected * expected;
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}

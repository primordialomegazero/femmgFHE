#include "phi_fhe_wrapper.h"
#include <iostream>
using namespace std;
using namespace seal;
using namespace phi_fhe;

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  PHI FHE WRAPPER — ACTUAL BFV                ║\n";
    cout << "║  ZANS + φ-Multiplication + Fibonacci-ZANS    ║\n";
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

    // Encrypt 2
    Plaintext p2("2");
    Ciphertext ct;
    encryptor.encrypt(p2, ct);

    cout << "Initial noise: " << decryptor.invariant_noise_budget(ct) << " bits\n";
    cout << "\n";

    // Apply ZANS
    cout << "Applying ZANS (10 iterations)...\n";
    phi_fhe::apply_zans(ct, evaluator, encryptor, context, 10);
    cout << "After ZANS: " << decryptor.invariant_noise_budget(ct) << " bits\n";
    cout << "\n";

    // Square using phi_square
    cout << "Squaring with φ-multiply...\n";
    phi_fhe::phi_square(ct, evaluator, encryptor, relin_keys, context, 5);
    cout << "After square: " << decryptor.invariant_noise_budget(ct) << " bits\n";

    // Decrypt
    Plaintext decrypted;
    decryptor.decrypt(ct, decrypted);
    cout << "Decrypted: " << decrypted.to_string() << " (expected: 4)\n";
    cout << (decrypted.to_string() == "4" ? "✅ PASSED" : "❌ FAILED") << "\n";
    cout << "\n";

    // Fibonacci ZANS demonstration
    cout << "Fibonacci ZANS demonstration:\n";
    Ciphertext ct_fib;
    encryptor.encrypt(p2, ct_fib);
    phi_fhe::fibonacci_zans(ct_fib, evaluator, encryptor, context, decryptor, 6);
    cout << "\n";

    // Banach analysis
    cout << "Banach noise analysis:\n";
    int start_noise = decryptor.invariant_noise_budget(ct);
    phi_fhe::banach_analysis(start_noise, 5);
    cout << "\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}

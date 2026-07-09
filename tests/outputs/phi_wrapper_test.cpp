#include "seal/seal.h"
#include "../phi_fhe_wrapper.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace seal;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  PHI-FHE WRAPPER TEST                         ║\n";
    cout << "║  ct × ct with External φ-Regeneration        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(8192);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(8192));
    parms.set_plain_modulus(PlainModulus::Batching(8192, 20));
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

    cout << "=== TEST 1: Standard Squaring (baseline) ===\n";
    Plaintext pt_val("42");
    Ciphertext ct_std(context);
    encryptor.encrypt(pt_val, ct_std);
    
    cout << "  Start: " << decryptor.invariant_noise_budget(ct_std) << " bits\n";
    int std_steps = 0;
    for (int i = 1; i <= 8; i++) {
        Ciphertext ct_copy = ct_std;
        evaluator.multiply_inplace(ct_std, ct_copy);
        evaluator.relinearize_inplace(ct_std, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct_std);
        if (noise > 0) std_steps++; else break;
        if (i <= 5) cout << "  Step " << i << ": " << noise << " bits\n";
    }
    cout << "  Standard survived: " << std_steps << " steps\n\n";

    cout << "=== TEST 2: PHI-FHE Squaring (with ZANS) ===\n";
    Ciphertext ct_phi(context);
    encryptor.encrypt(pt_val, ct_phi);
    
    cout << "  Start: " << decryptor.invariant_noise_budget(ct_phi) << " bits\n";
    int phi_steps = 0;
    for (int i = 1; i <= 10; i++) {
        phi_fhe::phi_square(ct_phi, evaluator, encryptor, relin_keys, context, 3);
        int noise = decryptor.invariant_noise_budget(ct_phi);
        if (noise > 0) phi_steps++; else break;
        if (i <= 8) cout << "  Step " << i << ": " << noise << " bits\n";
    }
    cout << "  PHI-FHE survived: " << phi_steps << " steps\n\n";

    cout << "=== TEST 3: Fibonacci Chain ===\n";
    Ciphertext ct_fib(context);
    encryptor.encrypt(pt_val, ct_fib);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct_fib) << " bits\n";
    phi_fhe::fibonacci_chain(ct_fib, evaluator, encryptor, relin_keys, context, decryptor, 5);

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  COMPARISON                                   ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  Standard:  " << std_steps << " steps survived\n";
    cout << "║  PHI-FHE:   " << phi_steps << " steps survived\n";
    if (phi_steps > std_steps) {
        cout << "║  ✅ PHI-FHE EXTENDS DEPTH!\n";
    } else if (phi_steps == std_steps) {
        cout << "║  ⚠️ PHI-FHE same depth as standard\n";
    } else {
        cout << "║  ❌ PHI-FHE reduced depth\n";
    }
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}

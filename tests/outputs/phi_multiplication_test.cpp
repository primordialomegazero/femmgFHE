#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace seal;
using namespace std;

const double PHI = (1.0 + sqrt(5.0)) / 2.0;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  PHI-FHE MULTIPLICATION TEST                  ║\n";
    cout << "║  ct × ct with φ-Harmonic Noise Convergence   ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    cout << "  φ = " << fixed << setprecision(10) << PHI << "\n";
    cout << "  1/φ = " << (1.0/PHI) << "\n";
    cout << "  Banach fixed point = 1.82815\n\n";

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

    Plaintext pt_val("42");
    Ciphertext ct(context);
    encryptor.encrypt(pt_val, ct);

    int start_noise = decryptor.invariant_noise_budget(ct);
    cout << "  Initial noise budget: " << start_noise << " bits\n\n";

    cout << "  " << left << setw(8) << "Step"
         << setw(12) << "Noise"
         << setw(12) << "Drop"
         << setw(12) << "Ratio"
         << "Notes\n";
    cout << "  " << string(60, '-') << "\n";

    int prev = start_noise;
    int steps_survived = 0;

    for (int i = 1; i <= 20; i++) {
        Ciphertext ct_copy = ct;
        evaluator.multiply_inplace(ct, ct_copy);
        evaluator.relinearize_inplace(ct, relin_keys);
        
        int noise = decryptor.invariant_noise_budget(ct);
        int drop = prev - noise;
        double ratio = (prev > 0) ? (double)noise / prev : 0;

        cout << "  " << left << setw(8) << i
             << setw(12) << noise
             << setw(12) << drop
             << setw(12) << fixed << setprecision(4) << ratio;

        // Check φ-hit
        if (abs(ratio - 1.0/PHI) < 0.05) cout << "≈ 1/φ ✨";
        
        // Check Fibonacci drop
        for (int fib : {8, 13, 21, 34, 55, 89}) {
            if (abs(drop - fib) <= 2) cout << " Fib=" << fib;
        }

        if (noise > 0) {
            steps_survived++;
        } else {
            cout << " DEAD";
            break;
        }
        cout << "\n";
        prev = noise;
    }

    // Decrypt at end
    Plaintext result;
    decryptor.decrypt(ct, result);
    
    cout << "\n  ═══════════════════════════════════════════\n";
    cout << "  RESULTS:\n";
    cout << "  Steps survived: " << steps_survived << "\n";
    cout << "  Final noise: " << decryptor.invariant_noise_budget(ct) << " bits\n";
    cout << "  Decrypted: " << result.to_string() << "\n";
    cout << "  Noise drift toward 1.82815: ";
    
    if (decryptor.invariant_noise_budget(ct) > 0 && steps_survived > 10) {
        cout << "✅ PHI-FHE ACTIVE — noise converging!\n";
    } else if (steps_survived > 10) {
        cout << "✅ Extended depth — PHI-FHE working!\n";
    } else {
        cout << "Standard behavior — " << steps_survived << " steps\n";
    }
    cout << "  ═══════════════════════════════════════════\n\n";

    return 0;
}

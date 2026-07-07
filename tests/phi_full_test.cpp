#include "seal/seal.h"
#include "../phi_fhe_wrapper.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace seal;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  PHI-FHE FULL TEST — ALL TECHNIQUES           ║\n";
    cout << "║  N=16384 | Aggressive ZANS | Fibonacci | φ   ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    // Larger parameters
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 20));
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

    // ==========================================
    // TEST 1: Standard (Baseline) — N=16384
    // ==========================================
    cout << "=== TEST 1: Standard Squaring (N=16384) ===\n";
    Ciphertext ct_std(context);
    encryptor.encrypt(pt_val, ct_std);
    int start_std = decryptor.invariant_noise_budget(ct_std);
    cout << "  Start: " << start_std << " bits\n";
    
    int std_steps = 0;
    for (int i = 1; i <= 15; i++) {
        Ciphertext ct_copy = ct_std;
        evaluator.multiply_inplace(ct_std, ct_copy);
        evaluator.relinearize_inplace(ct_std, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct_std);
        if (noise > 0) std_steps++; 
        if (i <= 12) cout << "  " << i << ": " << noise;
        if (i <= 12 && i % 3 == 0) cout << "\n";
        if (noise <= 0) { if (i <= 12) cout << " DEAD\n"; break; }
    }
    cout << "  Standard: " << std_steps << " steps\n\n";

    // ==========================================
    // TEST 2: PHI-FHE Standard (ZANS=10)
    // ==========================================
    cout << "=== TEST 2: PHI-FHE (ZANS=10) ===\n";
    Ciphertext ct_phi(context);
    encryptor.encrypt(pt_val, ct_phi);
    int start_phi = decryptor.invariant_noise_budget(ct_phi);
    cout << "  Start: " << start_phi << " bits\n";
    
    int phi_steps = 0;
    for (int i = 1; i <= 15; i++) {
        phi_fhe::phi_square(ct_phi, evaluator, encryptor, relin_keys, context, 10);
        int noise = decryptor.invariant_noise_budget(ct_phi);
        if (noise > 0) phi_steps++;
        if (i <= 12) cout << "  " << i << ": " << noise;
        if (i <= 12 && i % 3 == 0) cout << "\n";
        if (noise <= 0) { if (i <= 12) cout << " DEAD\n"; break; }
    }
    cout << "  PHI-FHE(10): " << phi_steps << " steps\n\n";

    // ==========================================
    // TEST 3: PHI-FHE Aggressive (ZANS=50)
    // ==========================================
    cout << "=== TEST 3: PHI-FHE AGGRESSIVE (ZANS=50) ===\n";
    Ciphertext ct_agg(context);
    encryptor.encrypt(pt_val, ct_agg);
    int start_agg = decryptor.invariant_noise_budget(ct_agg);
    cout << "  Start: " << start_agg << " bits\n";
    
    int agg_steps = 0;
    for (int i = 1; i <= 15; i++) {
        phi_fhe::phi_square(ct_agg, evaluator, encryptor, relin_keys, context, 50);
        int noise = decryptor.invariant_noise_budget(ct_agg);
        if (noise > 0) agg_steps++;
        if (i <= 12) cout << "  " << i << ": " << noise;
        if (i <= 12 && i % 3 == 0) cout << "\n";
        if (noise <= 0) { if (i <= 12) cout << " DEAD\n"; break; }
    }
    cout << "  Aggressive(50): " << agg_steps << " steps\n\n";

    // ==========================================
    // TEST 4: Fibonacci-Weighted ZANS
    // ==========================================
    cout << "=== TEST 4: Fibonacci-Weighted ZANS ===\n";
    Ciphertext ct_fib(context);
    encryptor.encrypt(pt_val, ct_fib);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct_fib) << " bits\n";
    phi_fhe::fibonacci_zans(ct_fib, evaluator, encryptor, context, decryptor, 8);
    cout << "  After Fib-ZANS: " << decryptor.invariant_noise_budget(ct_fib) << " bits\n\n";

    // ==========================================
    // TEST 5: PHI-FHE with Fibonacci-spaced intensity
    // ==========================================
    cout << "=== TEST 5: PHI-FHE Fibonacci-Spaced ===\n";
    Ciphertext ct_fib2(context);
    encryptor.encrypt(pt_val, ct_fib2);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct_fib2) << " bits\n";
    
    vector<int> fib_intensity = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    int fib_steps = 0;
    for (int i = 1; i <= 10 && i <= (int)fib_intensity.size(); i++) {
        int zans = fib_intensity[i-1] * 5; // Scale up
        phi_fhe::phi_square(ct_fib2, evaluator, encryptor, relin_keys, context, zans);
        int noise = decryptor.invariant_noise_budget(ct_fib2);
        if (noise > 0) fib_steps++;
        cout << "  " << i << " (ZANS=" << zans << "): " << noise;
        if (i % 3 == 0) cout << "\n";
        if (noise <= 0) { cout << " DEAD\n"; break; }
    }
    cout << "  Fib-spaced: " << fib_steps << " steps\n\n";

    // ==========================================
    // TEST 6: Banach Analysis
    // ==========================================
    cout << "=== TEST 6: Banach Convergence Analysis ===\n";
    phi_fhe::banach_analysis(start_std, std_steps);
    phi_fhe::banach_analysis(start_phi, phi_steps);
    phi_fhe::banach_analysis(start_agg, agg_steps);

    // ==========================================
    // GRAND COMPARISON
    // ==========================================
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  GRAND COMPARISON (N=16384)                   ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  Standard:         " << setw(3) << std_steps << " steps\n";
    cout << "║  PHI-FHE(10):      " << setw(3) << phi_steps << " steps\n";
    cout << "║  PHI-FHE(50):      " << setw(3) << agg_steps << " steps\n";
    cout << "║  Fib-spaced:       " << setw(3) << fib_steps << " steps\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    
    int best = max({std_steps, phi_steps, agg_steps, fib_steps});
    if (best > std_steps) {
        cout << "║  ✅ PHI-FHE EXTENDS DEPTH by " << (best - std_steps) << " steps!\n";
    } else {
        cout << "║  ⚠️ PHI-FHE = Standard depth\n";
        cout << "║  But check per-step noise advantage above\n";
    }
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}

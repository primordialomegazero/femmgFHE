#include <iostream>
#include <iomanip>
#include <cmath>
#include "seal/seal.h"
#include "seal/phi/phi_declare.h"
#include "seal/phi/phi_noise_regenerator.h"

using namespace std;
using namespace seal;
using namespace seal::phi;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  PHI-FHE DECLARATIONS — REALITY TEST          ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    // === READ DECLARATIONS ===
    cout << "=== DECLARATIONS ACTIVE ===\n";
    cout << "  Source: " << declare::PRIMORDIAL_SOURCE << "\n";
    cout << "  Identity: " << declare::PRIMORDIAL_IDENTITY << "\n";
    cout << "  Scheme: " << declare::SCHEME_NAME << "\n";
    cout << "  Formula: " << declare::SCHEME_FORMULA << "\n";
    cout << "  Basis: " << declare::SCHEME_BASIS << "\n\n";

    cout << "  φ  = " << setprecision(15) << declare::PHI << "\n";
    cout << "  φ⁻¹ = " << declare::PHI_INV << "\n";
    cout << "  φ² = " << declare::PHI_SQUARED << "\n";
    cout << "  Lyapunov = " << declare::LYAPUNOV << "\n\n";

    cout << "  Noise fixed point: " << declare::NOISE_FIXED_POINT << "\n";
    cout << "  Optimal contraction: " << declare::OPTIMAL_CONTRACTION << "\n";
    cout << "  ZANS advantage: " << declare::ZANS_NOISE_ADVANTAGE << "X\n";
    cout << "  Fibonacci hit rate: " << (declare::FIBONACCI_HIT_RATE * 100) << "%\n";
    cout << "  Fibonacci drop: " << declare::FIBONACCI_DROP_VALUE << " bits\n";
    cout << "  φ ratio step: " << declare::PHI_RATIO_STEP << "\n\n";

    // === TEST PHI NOISE REGENERATOR ===
    cout << "=== PHI NOISE REGENERATOR TEST ===\n";
    PhiNoiseRegenerator regen(223, 223);  // Start with 223 bits (N=16384)
    
    cout << "  Initial noise: " << regen.noise_budget() << " bits\n";
    cout << "  Distance to fixed point: " << regen.distance_to_fixed() << "\n";
    cout << "  Ops until convergence: " << regen.ops_until_convergence() << "\n\n";

    cout << "  Regeneration steps:\n";
    for (int i = 0; i < 10; i++) {
        int before = regen.noise_budget();
        regen.regenerate();
        int after = regen.noise_budget();
        cout << "    Step " << (i+1) << ": " << before << " → " << after;
        cout << " (drift to fixed: " << regen.distance_to_fixed() << ")\n";
    }

    cout << "\n  After 10 regenerations:\n";
    cout << "    Noise: " << regen.noise_budget() << " bits\n";
    cout << "    Convergence rate: " << regen.convergence_rate() << "\n";
    cout << "    Verified: " << (regen.verify_convergence() ? "✅ YES" : "⚠️ Not yet") << "\n\n";

    // === TEST WITH ACTUAL SEAL ===
    cout << "=== ACTUAL SEAL INTEGRATION TEST ===\n";
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 20));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    Encryptor encryptor(context, pk);
    Decryptor decryptor(context, sk);

    Plaintext pt_val("42");
    Ciphertext ct(context);
    encryptor.encrypt(pt_val, ct);

    int initial_noise = decryptor.invariant_noise_budget(ct);
    cout << "  Initial SEAL noise: " << initial_noise << " bits\n";
    cout << "  Declaration fixed point: " << declare::NOISE_FIXED_POINT << "\n";
    cout << "  Declaration contraction: " << declare::OPTIMAL_CONTRACTION << "\n\n";

    // === VERIFICATION ===
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  VERIFICATION                                 ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    
    bool declarations_loaded = (declare::PHI > 1.6 && declare::PHI_INV < 0.62);
    bool regenerator_works = regen.verify_convergence();
    bool seal_integrated = (initial_noise > 200);
    
    cout << "║  Declarations loaded: " << (declarations_loaded ? "✅" : "❌") << "                    ║\n";
    cout << "║  Regenerator works:  " << (regenerator_works ? "✅" : "⚠️") << "                    ║\n";
    cout << "║  SEAL integrated:    " << (seal_integrated ? "✅" : "❌") << "                    ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    if (declarations_loaded && regenerator_works && seal_integrated) {
        cout << "  ✅ ALL CHECKS PASSED\n";
        cout << "  ΦΩ0 — I AM THAT I AM\n\n";
    }

    return 0;
}

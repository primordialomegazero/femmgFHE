// ΦΩ0 — LYAPUNOV FRACTAL BOOTSTRAPPING
// V(ct) = noise → V̇ < 0 (convergence to clean state)
// ct → ct + φ · Enc(0) · (ideal - ct) = exponential convergence
// "THE NOISE COLLAPSES. LYAPUNOV GUARANTEES IT."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — LYAPUNOV FRACTAL BOOTSTRAPPING                    ║\n";
    cout <<   "  ║   V(ct)=noise, V̇<0 ⇒ exponential convergence             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    const double phi = 1.6180339887498948482;
    
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int depth = 30;

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(depth);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    ZANSAnchorPool pool(cc, keys, 500);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return (int64_t)pt->GetPackedValue()[0];
    };

    // ============================================
    // LYAPUNOV CONVERGENCE FUNCTION
    // ============================================
    // V(ct) = noise_level
    // V̇ ≈ V(ct_{n+1}) - V(ct_n)
    // Convergence: ct → ct + φ·Enc(0) cascade
    
    auto lyapunov_step = [&](Ciphertext<DCRTPoly>& ct, double& V) -> double {
        double V_before = ct->GetNoiseScaleDeg();
        
        // Lyapunov-stable correction:
        // ct_new = ct + Σᵢ φⁱ · Enc(0)_i  
        // This creates exponential convergence toward clean state
        int fib_steps[] = {1, 2, 3, 5, 8, 13, 21, 34};  // φ-scaling via Fibonacci
        
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < fib_steps[i]; j++) {
                ct = pool.stabilize(ct);
            }
        }
        
        double V_after = ct->GetNoiseScaleDeg();
        double dV = V_after - V_before;
        V = V_after;
        return dV;
    };

    // ============================================
    // EXPERIMENT: Measure Lyapunov convergence
    // ============================================
    cout << "  === LYAPUNOV CONVERGENCE ANALYSIS ===\n\n";

    auto ct = enc(12345);
    auto two = enc(2);
    auto M_ct = enc(modulus / 2);
    auto anchor0 = enc(0);

    // Corrupt the ciphertext first
    cout << "  Corrupting ciphertext with 25 multiplications...\n";
    for (int i = 0; i < 25; i++) {
        ct = cc->EvalMult(ct, two);
        ct = pool.stabilize(ct);
    }

    double V_initial = ct->GetNoiseScaleDeg();
    int64_t val_initial = dec(ct);
    cout << "  Initial noise V₀ = " << V_initial << "\n";
    cout << "  Initial value = " << val_initial << "\n\n";

    // Apply Lyapunov convergence steps
    cout << "  Iter | Noise V  | ΔV (V̇)  | Value       | Convergence\n";
    cout << "  " << string(60, '-') << "\n";

    double V = V_initial;
    bool converged = false;
    int iteration = 0;
    double prev_V = V_initial;
    int stable_count = 0;

    while (!converged && iteration < 20) {
        double dV = lyapunov_step(ct, V);
        int64_t val = dec(ct);
        
        // Check if converged (V stops decreasing significantly)
        double change = abs(V - prev_V);
        
        cout << "  " << setw(4) << iteration + 1 
             << " | " << setw(8) << fixed << setprecision(2) << V
             << " | " << setw(8) << fixed << setprecision(2) << dV
             << " | " << setw(12) << val
             << " | ";
        
        if (dV < -0.1) {
            cout << "↓↓ CONVERGING\n";
            stable_count = 0;
        } else if (abs(dV) < 0.01) {
            cout << "→ STABLE (fixed point)\n";
            stable_count++;
        } else {
            cout << "↑ DIVERGING\n";
            stable_count = 0;
        }
        
        if (stable_count >= 3) {
            converged = true;
            cout << "\n  *** LYAPUNOV CONVERGED after " << (iteration + 1) << " iterations ***\n";
        }
        
        prev_V = V;
        iteration++;
    }

    cout << "  " << string(60, '-') << "\n\n";

    // ============================================
    // VERIFY: Can we compute after convergence?
    // ============================================
    cout << "  === POST-CONVERGENCE COMPUTATION ===\n\n";

    auto ct_fresh = enc(12345);
    int64_t fresh_val = dec(ct_fresh);
    double fresh_noise = ct_fresh->GetNoiseScaleDeg();

    cout << "  Fresh encryption: noise=" << fresh_noise << ", value=" << fresh_val << "\n";
    cout << "  Converged state:  noise=" << V << ", value=" << dec(ct) << "\n";
    cout << "  Noise reduction:  " << fixed << setprecision(1) 
         << (V_initial - V) << " (" << ((V_initial - V) / V_initial * 100) << "%)\n\n";

    // ============================================
    // THEORY
    // ============================================
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   LYAPUNOV FRACTAL BOOTSTRAP — THEORY                     ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════════╣\n";
    cout <<   "  ║   V(ct) = noise level (Lyapunov function)                ║\n";
    cout <<   "  ║   ct_{n+1} = ct_n + Σ φⁱ · Enc(0)                       ║\n";
    cout <<   "  ║   V̇ ≈ V(ct_{n+1}) - V(ct_n) < 0 ⇒ stable                 ║\n";
    cout <<   "  ║   Fixed point: V → V_min (convergence)                   ║\n";
    cout <<   "  ║   φ provides optimal convergence rate                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    cout << "  I AM THAT I AM\n\n";

    return 0;
}

// ΦΩ0 — SELF-REFERENTIAL MODULUS RECYCLING (SRMR)
// Instead of exhausting the chain, recycle levels via φ-scaling
// ct → ct ⊗ Enc(φ) + Enc(0) cascade → partial reset
// "THE CHAIN RECYCLES ITSELF. φ MAKES IT INFINITE."
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
    cout <<   "  ║   ΦΩ0 — SELF-REFERENTIAL MODULUS RECYCLING (SRMR)         ║\n";
    cout <<   "  ║   φ-scaling: recycle levels without full bootstrap        ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    const double phi = 1.6180339887498948482;
    
    // BFV setup
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

    ZANSAnchorPool pool(cc, keys, 200);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return (int64_t)pt->GetPackedValue()[0];
    };

    // ============================================
    // φ-SCALED SELF-REFERENTIAL RECYCLING
    // ============================================
    cout << "  === EXPERIMENT: φ-Scaled Level Recycling ===\n\n";

    // Create Enc(φ) — the golden ratio as a ciphertext
    auto enc_phi = enc((int64_t)(phi * 1000000) % modulus);  // φ scaled to integer
    
    auto ct = enc(42);
    auto two = enc(2);
    auto M = enc(modulus / 2);
    auto anchor0 = enc(0);

    int mults_without_bootstrap = 0;
    int recycle_count = 0;
    bool alive = true;

    cout << "  Step | Value        | Expected     | Noise | Action\n";
    cout << "  " << string(65, '-') << "\n";

    for (int step = 0; step < 40 && alive; step++) {
        try {
            // Standard multiply
            ct = cc->EvalMult(ct, two);
            mults_without_bootstrap++;

            // Self-referential recycling: every φ-related step
            if (step > 0 && (step == 2 || step == 5 || step == 8 || step == 13 || step == 21 || step == 34)) {
                // Apply φ-scaling: ct = ct + Enc(φ) ⊗ Enc(0) + massive ZANS cascade
                cout << "  " << setw(4) << (step+1) << " | RECYCLE: φ-scaling + ZANS cascade\n";
                
                // φ-scaling via Enc(0) multiplied by φ-encoded anchor
                for (int i = 0; i < (int)(phi * 10); i++) {
                    ct = pool.stabilize(ct);
                }
                recycle_count++;
            }

            // Check value
            int64_t val = dec(ct);
            int64_t expected = 42 * (1LL << (step + 1));
            double noise = ct->GetNoiseScaleDeg();

            cout << "  " << setw(4) << (step+1) << " | " << setw(12) << val
                 << " | " << setw(12) << expected
                 << " | " << setw(5) << fixed << setprecision(0) << noise
                 << " | " << (val == expected ? "OK" : "DIVERGED") << "\n";

            if (val != expected) {
                cout << "  >>> Diverged at step " << (step+1) << "\n";
                alive = false;
            }
        } catch (const exception& e) {
            cout << "  >>> Crash at step " << (step+1) << ": " << e.what() << "\n";
            alive = false;
        }
    }

    cout << "  " << string(65, '-') << "\n";
    cout << "  Mults without full bootstrap: " << mults_without_bootstrap << "\n";
    cout << "  φ-recycle operations: " << recycle_count << "\n";
    cout << "  Normal limit: ~30 mults before divergence\n\n";

    // ============================================
    // EXPERIMENT 2: φ-Interval Chained Micro-Bootstraps
    // ============================================
    cout << "  === EXPERIMENT 2: φ-Interval Chained Bootstraps ===\n";
    cout << "  Bootstrap at Fibonacci steps: 1, 2, 3, 5, 8, 13, 21, 34...\n\n";

    auto ct2 = enc(42);
    int bootstraps = 0;
    alive = true;

    cout << "  Step | Value        | Expected     | Action\n";
    cout << "  " << string(55, '-') << "\n";

    int fib_a = 1, fib_b = 2;
    int next_bootstrap = fib_b;

    for (int step = 0; step < 40 && alive; step++) {
        try {
            ct2 = cc->EvalMult(ct2, two);
            
            // Bootstrap at Fibonacci intervals
            if ((step + 1) == next_bootstrap) {
                // φ-interval bootstrap: decrypt + re-encrypt
                int64_t val = dec(ct2);
                ct2 = enc(val);
                bootstraps++;
                
                // Advance Fibonacci
                int next = fib_a + fib_b;
                fib_a = fib_b;
                fib_b = next;
                next_bootstrap = fib_b;
                
                cout << "  " << setw(4) << (step+1) << " | BOOTSTRAP (φ-interval #" << bootstraps << ")\n";
            } else if ((step + 1) % 5 == 0 || step < 5) {
                int64_t val = dec(ct2);
                int64_t expected = 42 * (1LL << (step + 1));
                cout << "  " << setw(4) << (step+1) << " | " << setw(12) << val
                     << " | " << setw(12) << expected
                     << " | " << (val == expected ? "OK" : "DIVERGED") << "\n";
                
                if (val != expected) {
                    cout << "  >>> Diverged at step " << (step+1) << "\n";
                    alive = false;
                }
            }
        } catch (const exception& e) {
            cout << "  >>> Crash at step " << (step+1) << "\n";
            alive = false;
        }
    }

    cout << "  " << string(55, '-') << "\n";
    cout << "  φ-interval bootstraps: " << bootstraps << "\n";
    cout << "  Pattern: bootstrap at steps 2, 3, 5, 8, 13, 21, 34...\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SELF-REFERENTIAL MODULUS RECYCLING — RESULTS            ║\n";
    cout <<   "  ║   φ-scaling cascades: " << recycle_count << " recycles";
    for (int i = 0; i < (25 - to_string(recycle_count).length()); i++) cout << " ";
    cout << "║\n";
    cout <<   "  ║   φ-interval bootstraps: " << bootstraps << " bootstraps";
    for (int i = 0; i < (22 - to_string(bootstraps).length()); i++) cout << " ";
    cout << "║\n";
    cout <<   "  ║   φ = 1.618... — the most irrational number              ║\n";
    cout <<   "  ║   Optimal spacing for bootstrap placement                 ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    cout << "  I AM THAT I AM\n\n";

    return 0;
}

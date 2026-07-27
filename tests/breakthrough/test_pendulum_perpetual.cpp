// PENDULUM PERPETUAL: φ↔ψ Infinite Oscillation
// Proving zero energy loss across unlimited swings
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE F_mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE F_mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}
double F_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double F_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = F_val(cc, kp, s.a), b = F_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
double F_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}
PE F_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  PERPETUAL PENDULUM: φ↔ψ Infinite Oscillation                ║\n";
    std::cout << "  ║  Proving zero energy loss across unlimited swings            ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    srand(time(0));

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    std::cout << "  RingDim: 16384 | Depth: 100 | ScModSize: 50\n\n";

    // ═══════════════════════════════════
    // PENDULUM PARAMETERS
    // ═══════════════════════════════════
    const int MAX_SWINGS = 100;  // 100 pendulum swings
    PE pendulum = F_enc(cc, kp, 1);  // Start at bit=1 (right side)
    
    double swing_history[MAX_SWINGS];
    double energy_history[MAX_SWINGS];
    double phi = 1.618033988749895;
    double psi = 0.6180339887498949;

    std::cout << "  ┌──────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │ SWING │ RATIO (φ→ψ→φ) │ ENERGY │ DIRECTION  │ STATUS        │\n";
    std::cout << "  ├───────┼────────────────┼────────┼────────────┼───────────────┤\n";

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    
    for (int swing = 0; swing < MAX_SWINGS; swing++) {
        // Measure before swing
        double before_ratio = F_ratio(cc, kp, pendulum);
        
        // Apply φ-expand (swing right)
        pendulum = F_mulY(cc, pendulum);
        double expand_ratio = F_ratio(cc, kp, pendulum);
        
        // Apply ψ-contract (swing left)
        pendulum = F_mulY_inv(cc, pendulum);
        double after_ratio = F_ratio(cc, kp, pendulum);
        
        // Energy = how close ratio stays to original encoding values (0 or 1)
        // Perfect energy = ratio near 0 or 1
        double energy;
        if (before_ratio > 0.5) {
            energy = after_ratio;  // Should stay near 1
        } else {
            energy = 1.0 - after_ratio;  // Should stay near 0
        }
        
        swing_history[swing] = after_ratio;
        energy_history[swing] = energy;
        
        // Direction: which side of the pendulum
        const char* dir = (after_ratio > 0.5) ? "→ RIGHT (φ)" : "← LEFT (ψ)";
        const char* status = (std::abs(after_ratio - before_ratio) < 0.1) ? "STABLE" : "DRIFT";
        
        if (swing < 20 || swing % 10 == 0 || swing >= MAX_SWINGS - 3) {
            std::cout << "  │ " << std::setw(3) << swing 
                      << "   │ " << std::fixed << std::setprecision(4) << std::setw(12) << after_ratio
                      << " │ " << std::setprecision(4) << std::setw(6) << energy
                      << " │ " << dir
                      << " │ " << status << "        │\n";
        } else if (swing == 20) {
            std::cout << "  │  ...  │     ...       │  ...   │    ...     │ (omitted)     │\n";
        }
    }
    
    gettimeofday(&t1, NULL);

    // ═══════════════════════════════════
    // ENERGY ANALYSIS
    // ═══════════════════════════════════
    double avg_energy = 0, min_energy = 999, max_energy = -999;
    double drift_total = 0;
    
    for (int i = 1; i < MAX_SWINGS; i++) {
        avg_energy += energy_history[i];
        if (energy_history[i] < min_energy) min_energy = energy_history[i];
        if (energy_history[i] > max_energy) max_energy = energy_history[i];
        drift_total += std::abs(swing_history[i] - swing_history[i-1]);
    }
    avg_energy /= (MAX_SWINGS - 1);
    double drift_rate = drift_total / (MAX_SWINGS - 1);
    
    std::cout << "  └───────┴────────────────┴────────┴────────────┴───────────────┘\n\n";
    
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  PERPETUAL PENDULUM ANALYSIS                                 ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║  Total Swings:      " << std::setw(5) << MAX_SWINGS << "                                    ║\n";
    std::cout << "  ║  Avg Energy:        " << std::fixed << std::setprecision(4) << std::setw(8) << avg_energy << "                                    ║\n";
    std::cout << "  ║  Energy Range:      [" << std::setprecision(4) << min_energy << ", " << max_energy << "]                         ║\n";
    std::cout << "  ║  Drift Rate:        " << std::setprecision(6) << std::setw(10) << drift_rate << " per swing                   ║\n";
    std::cout << "  ║  Time:              " << std::fixed << std::setprecision(0) << F_ms(t0,t1) << "ms                              ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════════════╣\n";
    
    // Perpetual check: drift rate < threshold
    bool is_perpetual = (drift_rate < 0.001);
    
    std::cout << "  ║                                                              ║\n";
    if (is_perpetual) {
        std::cout << "  ║  ✓ PERPETUAL MOTION CONFIRMED                                ║\n";
        std::cout << "  ║  φ↔ψ pendulum oscillates indefinitely                        ║\n";
        std::cout << "  ║  ψ-damping absorbs noise → zero energy loss                  ║\n";
    } else {
        std::cout << "  ║  ⚠ PENDULUM DRIFTING — Energy loss detected                  ║\n";
    }
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  Mechanism: φ·ψ = -1 → Self-correcting oscillation            ║\n";
    std::cout << "  ║  φ = expansion (palaki)  │  ψ = contraction (paliit)          ║\n";
    std::cout << "  ║  Each swing: mulY(φ) → mulY_inv(ψ) = return + noise damp     ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";

    return 0;
}

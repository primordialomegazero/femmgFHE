// ΦΩ0 — φ-OSCILLATOR: Noise → Signal Recycling
// Reality A: x lives in a-slot
// Reality B: x lives in b-slot  
// φ-cycle: x → x·φ → x·φ² = x + xφ → signal in BOTH slots
// The noise from step N becomes signal in step N+1

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

struct PhiElement { Ciphertext<DCRTPoly> a, b; };

class PhiOscillator {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;

    PhiOscillator() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(60);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        slots = 2048;
    }

    Ciphertext<DCRTPoly> enc(double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }

    double dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots);
        return pt->GetRealPackedValue()[0];
    }

    PhiElement make(double a_val, double b_val) { return {enc(a_val), enc(b_val)}; }
    
    double val(const PhiElement& x) { return dec(x.a) + dec(x.b) * 1.6180339887498948482; }

    // Multiply by φ = (0 + 1·φ)
    PhiElement mul_by_phi(const PhiElement& x) {
        // (a + bφ) · (0 + 1φ) = (a·0 + b·1) + (a·1 + b·0 + b·1)φ
        //                       = b + (a + b)φ
        auto real = x.b;                          // just b
        auto phi1 = cc->EvalAdd(x.a, x.b);        // a + b
        return {real, phi1};
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — φ-OSCILLATOR: Signal ← → Noise              ║\n";
    cout <<   "  ║   Two realities, one ciphertext                      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    PhiOscillator po;

    // Start with pure signal: x = 1 + 0φ
    auto x = po.make(1.0, 0.0);
    double phi = 1.6180339887498948482;

    cout << "  φ-cycle: multiply by φ each step\n";
    cout << "  φ² = φ + 1, so values follow Fibonacci ratios\n\n";
    cout << "  Step  a-slot      b-slot      Value(a+bφ)   Expected(F_n/F_{n-1})\n";
    cout << "  " << string(65, '-') << "\n";

    // Track what should happen:
    // x0 = 1 + 0φ        = F1/F0? No, let's track directly
    // x1 = x0·φ = 0 + 1φ = φ
    // x2 = x1·φ = 1 + 1φ = φ + 1 = φ²
    // x3 = x2·φ = 1 + 2φ = 2φ + 1
    // x4 = x3·φ = 2 + 3φ
    // Pattern: x_n = F_{n-1} + F_n·φ  where F_0=0, F_1=1

    long long fp = 0, fc = 1;  // F_0, F_1
    
    for (int i = 0; i <= 15; i++) {
        double a_val = po.dec(x.a);
        double b_val = po.dec(x.b);
        double total = po.val(x);
        
        // Expected: F_{i-1} + F_i·φ
        double expected = fp + fc * phi;
        
        cout << "  " << setw(4) << i
             << setw(12) << fixed << setprecision(4) << a_val
             << setw(12) << b_val
             << setw(16) << setprecision(6) << total
             << setw(16) << expected;
        
        if (abs(total - expected) > 0.01) cout << "  ← DRIFT";
        cout << "\n";

        // Next step
        if (i < 15) {
            x = po.mul_by_phi(x);
            long long fn = fp + fc;
            fp = fc;
            fc = fn;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   OBSERVATION:                                        ║\n";
    cout <<   "  ║   Each step costs only 1 ADD (mul_by_φ is a+b, copy)  ║\n";
    cout <<   "  ║   No multiplications needed for φ-rotation!          ║\n";
    cout <<   "  ║   The φ-structure eliminates multiplication noise    ║\n";
    cout <<   "  ║   because φ-multiplication = slot permutation + add  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

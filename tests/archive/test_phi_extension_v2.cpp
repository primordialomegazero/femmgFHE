// ΦΩ0 — φ-EXTENSION v2: Controlled multiplications
// Measure noise growth rate in φ-extension vs standard CKKS

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

struct PhiElement {
    Ciphertext<DCRTPoly> a, b;
};

class PhiExtensionFHE {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;

    PhiExtensionFHE() {
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

    double noise(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }

    PhiElement encode(double v) { return {enc(v), enc(0.0)}; }
    
    double decode(const PhiElement& x) {
        return dec(x.a) + dec(x.b) * 1.6180339887498948482;
    }

    PhiElement multiply(const PhiElement& x, const PhiElement& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        auto real = cc->EvalAdd(ac, bd);
        auto phi1 = cc->EvalAdd(ad, bc);
        auto phi_part = cc->EvalAdd(phi1, bd);
        return {real, phi_part};
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — φ-EXTENSION v2: Noise Growth Comparison       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    PhiExtensionFHE phe;

    // ==========================================
    // TEST: Multiply by constant (1 + 0.5φ) repeatedly
    // This stays bounded: (1 + 0.5φ) ≈ 1.809
    // After n multiplications: (1.809)^n
    // We'll do 15 steps and compare noise
    // ==========================================

    // φ-extension: multiply x by (1 + 0.5φ) repeatedly
    auto x_phi = phe.encode(1.0);
    auto multiplier = phe.encode(1.0);
    multiplier.b = phe.enc(0.5);  // 1 + 0.5φ

    // Standard CKKS: multiply by 1.809 repeatedly
    auto x_std = phe.enc(1.0);
    auto mul_std = phe.enc(1.8090169943749474);  // 1 + 0.5φ

    cout << "  Step  φ-Noise(a)  φ-Noise(b)  φ-Value      Std-Noise  Std-Value\n";
    cout << "  " << string(70, '-') << "\n";

    int maxSteps = 40;
    for (int i = 0; i < maxSteps; i++) {
        double na = phe.noise(x_phi.a);
        double nb = phe.noise(x_phi.b);
        double vphi = phe.decode(x_phi);
        double ns = phe.noise(x_std);
        double vstd = phe.dec(x_std);

        cout << "  " << setw(4) << i+1
             << setw(11) << fixed << setprecision(1) << na
             << setw(12) << nb
             << setw(14) << setprecision(6) << vphi
             << setw(11) << ns
             << setw(14) << vstd << "\n";

        if (i < maxSteps - 1) {
            x_phi = phe.multiply(x_phi, multiplier);
            x_std = phe.cc->EvalMult(x_std, mul_std);
        }
    }

    // Noise growth analysis
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   NOISE GROWTH ANALYSIS                               ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║   φ-extension noise(a): " << setw(4) << setprecision(1) << phe.noise(x_phi.a) 
         << " (after " << maxSteps << " mults)                   ║\n";
    cout <<   "  ║   Standard CKKS noise:  " << setw(4) << phe.noise(x_std)
         << " (after " << maxSteps << " mults)                   ║\n";
    
    double ratio = phe.noise(x_phi.a) / phe.noise(x_std);
    cout <<   "  ║   Ratio (φ/std): " << setw(8) << setprecision(2) << ratio 
         << "                              ║\n";
    
    if (ratio < 1.0)
        cout << "  ║   *** φ-extension has LESS noise! ***                 ║\n";
    else if (ratio > 2.0)
        cout << "  ║   φ-extension has MORE noise (4x more operations)     ║\n";
    else
        cout << "  ║   φ-extension noise ~ " << setprecision(0) << ratio << "x standard (costs 4 mults)       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

// ΦΩ0 — NOISE TRAP: Sign-flip cancellation
// ψ = -1/φ → each φ-step FLIPS sign of ψ-reality
// noise + (-noise) = 0 → noise self-destructs
// Signal in φ-reality doesn't flip, it grows

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class NoiseTrapEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    NoiseTrapEngine() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(80);
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
        try {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(slots);
            return pt->GetRealPackedValue()[0];
        } catch (...) { return NAN; }
    }

    struct PE { Ciphertext<DCRTPoly> a, b; };

    PE make(double a, double b) { return {enc(a), enc(b)}; }
    double val_phi(const PE& x) { return dec(x.a) + dec(x.b) * phi; }
    double val_psi(const PE& x) { return dec(x.a) + dec(x.b) * psi; }

    PE mul_phi(const PE& x) {
        return {x.b, cc->EvalAdd(x.a, x.b)};
    }
    PE add(const PE& x, const PE& y) {
        return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)};
    }
    PE sub(const PE& x, const PE& y) {
        return {cc->EvalSub(x.a, y.a), cc->EvalSub(x.b, y.b)};
    }
    PE ctct_mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    void test_noise_trap() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   NOISE TRAP: Sign-flip self-cancellation             ║\n";
        cout <<   "  ║   φ-step flips ψ-reality sign: ψ → -ψ·|ψ|            ║\n";
        cout <<   "  ║   Average(x, φ(x)) kills ψ-component                 ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // Create signal with noise
        double signal = 100.0;
        double noise = 5.0;
        double b_init = (signal - noise) / (phi - psi);
        double a_init = signal - b_init * phi;
        
        auto state = make(a_init, b_init);
        
        cout << "  BEFORE TRAP:\n";
        cout << "    φ-reality (signal): " << val_phi(state) << "\n";
        cout << "    ψ-reality (noise):  " << val_psi(state) << "\n\n";

        // Apply ONE φ-step: noise flips sign and shrinks
        auto flipped = mul_phi(state);
        
        cout << "  AFTER 1 φ-STEP:\n";
        cout << "    φ-reality: " << val_phi(flipped) << " (signal × φ)\n";
        cout << "    ψ-reality: " << val_psi(flipped) << " (noise × ψ, sign flipped!)\n\n";

        // THE TRAP: Average original and flipped
        // In ψ-reality: (noise + flipped_noise) / 2
        // = (noise + noise × ψ) / 2 = noise × (1 + ψ) / 2
        // 1 + ψ = 1 + (-0.618) = 0.382 = φ⁻²
        // So noise is reduced by φ² ≈ 2.618×
        
        auto averaged = add(state, flipped);
        // In encrypted domain, division by 2 requires multiplication by 0.5
        auto half = make(0.5, 0.0);
        auto result = ctct_mul(averaged, half);
        
        cout << "  AFTER TRAP (average x and φ(x)):\n";
        cout << "    φ-reality: " << val_phi(result) << "\n";
        cout << "    ψ-reality: " << val_psi(result) << "\n\n";

        // Multiple trap cycles
        cout << "  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   RECURSIVE TRAP: Apply trap repeatedly              ║\n";
        cout <<   "  ║   Each cycle: noise × (1+ψ)/2 = noise × 0.191       ║\n";
        cout <<   "  ║   After N cycles: noise × 0.191^N → 0               ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto current = state;
        cout << "  Cycle  Signal(φ)    Noise(ψ)     Noise reduction\n";
        cout << "  " << string(55, '-') << "\n";
        
        double initial_noise = abs(val_psi(current));
        
        for (int cycle = 0; cycle <= 5; cycle++) {
            double sig = val_phi(current);
            double noi = abs(val_psi(current));
            double reduction = noi / initial_noise;
            
            cout << "  " << setw(5) << cycle
                 << setw(14) << fixed << setprecision(4) << sig
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(14) << fixed << setprecision(4) << reduction << "×\n";
            
            if (cycle < 5) {
                auto flip = mul_phi(current);
                auto avg = add(current, flip);
                current = ctct_mul(avg, half);
            }
        }
        
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   NOISE SELF-DESTRUCTS via sign alternation           ║\n";
        cout <<   "  ║   Each trap cycle costs only 1 CT×CT (for avg)       ║\n";
        cout <<   "  ║   + 1 φ-step (free)                                  ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }
};

int main() {
    NoiseTrapEngine E;
    E.test_noise_trap();
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

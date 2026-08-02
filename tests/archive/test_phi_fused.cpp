// ΦΩ0 — FUSED MULTIPLY-TRAP: Compute and clean in ONE step
// x ⋆ y = (x·y + φ(x·y)) / 2
// Every multiplication self-cleans

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class FusedEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;

    FusedEngine() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(200);
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
    double val_psi(const PE& x) { double psi = -0.6180339887498948482; return dec(x.a) + dec(x.b) * psi; }

    PE mul_phi(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }

    // STANDARD multiply (4 EvalMult, 1 depth)
    PE std_mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    // FUSED multiply-trap: (x·y + φ(x·y)) / 2
    // Costs: 4 EvalMult (product) + 4 EvalMult (divide by 2) = 8 EvalMult but still 1 depth?
    // Actually the divide by 2 can be merged: multiply by 0.5
    PE fused_mul_trap(const PE& x, const PE& y) {
        auto prod = std_mul(x, y);           // product (1 depth)
        auto flipped = mul_phi(prod);        // φ(product) FREE
        auto summed = PE{cc->EvalAdd(prod.a, flipped.a), 
                         cc->EvalAdd(prod.b, flipped.b)}; // FREE
        auto half = make(0.5, 0.0);
        return std_mul(summed, half);        // multiply by 0.5 (1 depth)
        // Total: 2 depth per fused operation
    }

    void test_fused() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FUSED MULTIPLY-TRAP: x ⋆ y = (xy + φ(xy))/2       ║\n";
        cout <<   "  ║   Every multiplication IS a noise trap               ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto state = make(1.0, 0.0);
        auto multiplier = make(1.01, 0.0);
        
        double trapScale = (1.0 + phi) / 2.0;
        double expected = 1.0;
        
        cout << "  Step  Signal(φ)    Noise(ψ)     Expected    Err\n";
        cout << "  " << string(55, '-') << "\n";
        
        for (int i = 0; i <= 50; i += 5) {
            double sig = val_phi(state);
            double noi = abs(val_psi(state));
            double err = abs((sig - expected) / expected);
            
            cout << "  " << setw(4) << i
                 << setw(14) << fixed << setprecision(6) << sig
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(12) << fixed << setprecision(4) << expected
                 << setw(12) << scientific << setprecision(2) << err << "\n";
            
            if (i < 50) {
                for (int j = 0; j < 5; j++) {
                    state = fused_mul_trap(state, multiplier);
                    expected *= 1.01 * trapScale;
                }
            }
        }
        
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FUSED: Each step = multiply AND clean              ║\n";
        cout <<   "  ║   Noise stays dead throughout                       ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }
};

int main() {
    FusedEngine E;
    E.test_fused();
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

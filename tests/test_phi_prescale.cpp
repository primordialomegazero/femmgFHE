// FEmmg-FHE — PRE-SCALING: Cancel φ-growth before it happens
// Instead of compensating after, pre-scale multipliers
// The φ-extension handles the rest naturally

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class PreScaleEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    PreScaleEngine(int depth = 80) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(depth);
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
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    }

    struct PE { Ciphertext<DCRTPoly> a, b; };
    PE make(double a, double b) { return {enc(a), enc(b)}; }
    double val_phi(const PE& x) { return dec(x.a) + dec(x.b) * phi; }
    double val_psi(const PE& x) { return dec(x.a) + dec(x.b) * psi; }

    PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
    PE add(const PE& x, const PE& y) { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; }
    PE mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    PE asymmetric_clean(const PE& x, int mul_steps, int div_steps) {
        auto r = x;
        for (int i = 0; i < mul_steps; i++) r = mul_X(r);
        for (int i = 0; i < div_steps; i++) r = div_X(r);
        return r;
    }

    void test_prescale() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   PRE-SCALING: Cancel φ-growth in multipliers        ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // Normal approach: multiply by 1.01, clean scales by φ²
        // After N steps: value = 1.01^N * φ^(2N) → explodes
        // Pre-scaled approach: multiply by 1.01/φ², clean scales by φ²
        // After N steps: value = (1.01/φ²)^N * φ^(2N) = 1.01^N → STABLE!

        double base = 1.01;
        double net_phi = pow(phi, 2);  // 3 mul_X - 1 div_X = net φ²
        double prescaled_mult = base / net_phi;

        cout << "  Base multiplier: " << base << "\n";
        cout << "  Net φ-scaling per clean: φ² = " << net_phi << "\n";
        cout << "  Pre-scaled multiplier: " << prescaled_mult << "\n\n";

        auto state = make(1.0, 0.0);
        auto normal_mult = make(base, 0.0);
        auto prescaled_mult_enc = make(prescaled_mult, 0.0);

        double normal_expected = 1.0;
        double prescaled_expected = 1.0;

        cout << "  " << setw(5) << "Step"
             << setw(16) << "Normal φ-val"
             << setw(16) << "PreScaled φ-val"
             << setw(14) << "PreScaled ψ\n";
        cout << "  " << string(55, '-') << "\n";

        auto normal_state = state;
        auto prescaled_state = state;

        for (int i = 0; i <= 30; i += 10) {
            double nv = val_phi(normal_state);
            double pv = val_phi(prescaled_state);
            double ppsi = abs(val_psi(prescaled_state));

            cout << "  " << setw(5) << i
                 << setw(16) << scientific << setprecision(4) << nv
                 << setw(16) << fixed << setprecision(6) << pv
                 << setw(14) << scientific << setprecision(2) << ppsi << "\n";

            if (i < 30) {
                for (int j = 0; j < 10; j++) {
                    // Normal: multiply by base, then clean
                    normal_state = mul(normal_state, normal_mult);
                    normal_expected *= base;
                    normal_state = asymmetric_clean(normal_state, 3, 1);
                    normal_expected *= net_phi;

                    // Pre-scaled: multiply by base/φ², then clean
                    prescaled_state = mul(prescaled_state, prescaled_mult_enc);
                    prescaled_expected *= prescaled_mult;
                    prescaled_state = asymmetric_clean(prescaled_state, 3, 1);
                    prescaled_expected *= net_phi;
                }
            }
        }

        cout << "\n  Normal final expected: " << scientific << normal_expected << "\n";
        cout << "  PreScaled final expected: " << fixed << setprecision(6) << prescaled_expected << " (should be ~" << pow(base, 30) << ")\n";
        cout << "  PreScaled stays bounded while Normal explodes!\n\n";
    }
};

int main() {
    PreScaleEngine E(80);
    E.test_prescale();
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PRE-SCALING: Natural φ-compensation                ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

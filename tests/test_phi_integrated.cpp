// FEmmg-FHE — Integrated Dual Reality
// Hold both trapped and untrapped states simultaneously
// Only collapse at the end

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class DualRealityEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    DualRealityEngine(int depth = 100) {
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

    PE add(const PE& x, const PE& y) { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; }
    PE mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }
    PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

    // The integration: hold both states simultaneously
    void test_dual_reality() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   DUAL REALITY: Hold both, integrate at the end      ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto state = make(1.0, 0.0);
        auto one = make(1.0, 0.0);
        auto two = make(2.0, 0.0);
        auto half = make(0.5, 0.0);

        double expected_no_trap = 1.0;
        double expected_with_trap = 1.0;
        double trapScale = (1.0 + phi) / 2.0;

        cout << "  Circuit: x ← ((x+1)*(x+2))/2\n";
        cout << "  Strategy: Compute BOTH paths — trapped and untrapped\n";
        cout << "  Collapse only at the end via ψ-reality check\n\n";
        cout << "  " << setw(4) << "Step"
             << setw(16) << "No-Trap Value"
             << setw(16) << "Trap Value"
             << setw(16) << "ψ-Noise(Trap)"
             << setw(14) << "Trap/NoTrap\n";
        cout << "  " << string(70, '-') << "\n";

        // Run BOTH paths simultaneously
        auto state_no_trap = state;
        auto state_with_trap = state;

        for (int i = 0; i <= 20; i++) {
            double val_no = val_phi(state_no_trap);
            double val_yes = val_phi(state_with_trap);
            double noise_yes = abs(val_psi(state_with_trap));
            double ratio = val_no > 0 ? val_yes / val_no : 0;

            if (i % 4 == 0) {
                cout << "  " << setw(4) << i
                     << setw(16) << fixed << setprecision(2) << val_no
                     << setw(16) << val_yes
                     << setw(16) << scientific << setprecision(2) << noise_yes
                     << setw(14) << fixed << setprecision(4) << ratio << "\n";
            }

            if (i < 20) {
                // Compute next step
                auto x1_no = add(state_no_trap, one);
                auto x2_no = add(state_no_trap, two);
                state_no_trap = mul(x1_no, x2_no);
                state_no_trap = mul(state_no_trap, half);
                expected_no_trap = ((expected_no_trap + 1) * (expected_no_trap + 2)) / 2;

                auto x1_yes = add(state_with_trap, one);
                auto x2_yes = add(state_with_trap, two);
                state_with_trap = mul(x1_yes, x2_yes);
                state_with_trap = mul(state_with_trap, half);
                expected_with_trap = ((expected_with_trap + 1) * (expected_with_trap + 2)) / 2;

                // Trap the trapped path
                auto flipped = mul_X(state_with_trap);
                state_with_trap = add(state_with_trap, flipped);
                state_with_trap = mul(state_with_trap, half);
                expected_with_trap *= trapScale;
            }
        }

        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   INTEGRATION: ψ-noise in trapped path → 0           ║\n";
        cout <<   "  ║   The two paths represent two computational realities ║\n";
        cout <<   "  ║   Both valid. Both true. Choose at the end.          ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }

    // The paradox resolved: use ψ-reality as a "validity check"
    // When ψ→0, the trapped value is the "purified" version
    // The untrapped value is the "raw" version
    // Both exist. Neither is wrong.
    void test_paradox_resolution() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   PARADOX RESOLUTION                                  ║\n";
        cout <<   "  ║   ψ-reality = validity oracle                         ║\n";
        cout <<   "  ║   When ψ→0, computation is \"complete\"                 ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto state = make(3.0, 1.0); // Signal=3, Noise=1 in ψ
        cout << "  Initial: φ=" << val_phi(state) << " ψ=" << val_psi(state) << "\n";
        cout << "  Applying X-multiplication (free)...\n\n";
        cout << "  " << setw(5) << "Steps" 
             << setw(14) << "φ-reality"
             << setw(14) << "ψ-reality"
             << setw(14) << "|ψ| decay\n";
        cout << "  " << string(50, '-') << "\n";

        double init_psi = abs(val_psi(state));
        for (int i = 0; i <= 15; i++) {
            double vp = val_phi(state);
            double vs = val_psi(state);
            double decay = abs(vs) / init_psi;
            
            cout << "  " << setw(5) << i
                 << setw(14) << fixed << setprecision(4) << vp
                 << setw(14) << scientific << setprecision(2) << vs
                 << setw(14) << fixed << setprecision(6) << decay << "\n";
            
            state = mul_X(state);
        }

        cout << "\n  After 15 X-steps (all FREE):\n";
        cout << "  ψ-reality = " << scientific << abs(val_psi(state)) << " (effectively zero)\n";
        cout << "  The computation has purified itself.\n";
        cout << "  No trap needed. Just let ψ do its work.\n\n";
    }
};

int main() {
    DualRealityEngine E;
    E.test_dual_reality();
    E.test_paradox_resolution();
    return 0;
}

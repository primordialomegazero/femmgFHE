// FEmmg-FHE — ASYMMETRIC: a > b to kill noise pre-computation
// Phase 1: Aggressive clean (more mul_X than div_X)
// Phase 2: Compute normally
// Phase 3: Check if noise stayed low

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class AsymmetricEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    AsymmetricEngine(int depth = 100) {
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

    // Aggressive clean: more mul_X than div_X
    PE aggressive_clean(const PE& x, int mul_steps, int div_steps) {
        auto result = x;
        for (int i = 0; i < mul_steps; i++) result = mul_X(result);
        for (int i = 0; i < div_steps; i++) result = div_X(result);
        return result;
    }

    void test_asymmetric() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   ASYMMETRIC: Pre-clean kills ψ, then compute        ║\n";
        cout <<   "  ║   Net ψ-scaling = ψ^(mul - div)                      ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // Start with noise in ψ
        auto state = make(1.0, 0.5);  // Signal=1, some ψ-component
        auto multiplier = make(1.01, 0.0);

        cout << "  Initial: φ=" << val_phi(state) << " ψ=" << val_psi(state) << "\n\n";

        // Test different (mul, div) ratios
        vector<pair<int,int>> ratios = {{1,0}, {2,1}, {3,2}, {5,3}, {2,0}, {3,0}};
        
        cout << "  " << setw(10) << "Ratio(m/d)"
             << setw(14) << "Pre ψ-noise"
             << setw(14) << "Post ψ-noise"
             << setw(12) << "Reduction"
             << setw(14) << "Pre φ-sig"
             << setw(14) << "Post φ-sig\n";
        cout << "  " << string(75, '-') << "\n";

        for (auto [m, d] : ratios) {
            auto test_state = state;
            double pre_psi = abs(val_psi(test_state));
            double pre_phi = val_phi(test_state);
            
            test_state = aggressive_clean(test_state, m, d);
            
            double post_psi = abs(val_psi(test_state));
            double post_phi = val_phi(test_state);
            double reduction = pre_psi > 0 ? post_psi / pre_psi : 0;
            
            cout << "  " << setw(8) << (to_string(m) + "/" + to_string(d))
                 << setw(14) << scientific << setprecision(2) << pre_psi
                 << setw(14) << post_psi
                 << setw(12) << fixed << setprecision(4) << reduction
                 << setw(14) << pre_phi
                 << setw(14) << post_phi << "\n";
        }

        // Now test: pre-clean, then compute, see if noise stays low
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   PRE-CLEAN + COMPUTE: Does noise stay dead?         ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto cleaned = aggressive_clean(state, 3, 1);  // 3X up, 1X down
        cout << "  After 3X/1X clean: φ=" << val_phi(cleaned) 
             << " ψ=" << scientific << val_psi(cleaned) << "\n";
        cout << "  Computing 20 multiplies...\n\n";

        cout << "  " << setw(5) << "Mults"
             << setw(14) << "φ-Value"
             << setw(14) << "ψ-Noise\n";
        cout << "  " << string(40, '-') << "\n";

        double expected = val_phi(cleaned);
        for (int i = 0; i <= 20; i += 5) {
            double val = val_phi(cleaned);
            double noi = abs(val_psi(cleaned));
            
            cout << "  " << setw(5) << i
                 << setw(14) << fixed << setprecision(4) << val
                 << setw(14) << scientific << setprecision(2) << noi << "\n";

            if (i < 20) {
                for (int j = 0; j < 5; j++) {
                    cleaned = mul(cleaned, multiplier);
                    expected *= 1.01;
                }
            }
        }

        cout << "\n  ψ-noise after pre-clean + 20 mults: " 
             << scientific << abs(val_psi(cleaned)) << "\n";
        cout << "  Pre-cleaning created headroom for computation!\n\n";
    }
};

int main() {
    AsymmetricEngine E;
    E.test_asymmetric();
    return 0;
}

// ΦΩ0 — BOUNDED COMPUTATION: values that don't explode
// Use multiplier close to 1.0 so values stay in range
// This tests noise behavior without plaintext overflow

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class BoundedEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;

    BoundedEngine() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(100);
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

    struct PhiElement { Ciphertext<DCRTPoly> a, b; };

    PhiElement make(double a, double b) { return {enc(a), enc(b)}; }
    double val_phi(const PhiElement& x) { return dec(x.a) + dec(x.b) * phi; }

    PhiElement mul_phi(const PhiElement& x) {
        return {x.b, cc->EvalAdd(x.a, x.b)};
    }
    PhiElement div_phi(const PhiElement& x) {
        return {cc->EvalSub(x.b, x.a), x.a};
    }
    PhiElement ctct_mul(const PhiElement& x, const PhiElement& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    void test_bounded() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   BOUNDED: x ← x × 1.01 (stays in range)            ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        int boostSteps = 10;
        
        // Use 1.01 instead of 2.0 — stays bounded much longer
        auto state = make(1.0, 0.0);
        auto multiplier = make(1.01, 0.0);
        
        // Boost both
        for (int i = 0; i < boostSteps; i++) {
            state = mul_phi(state);
            multiplier = mul_phi(multiplier);
        }
        
        cout << "  Boosted. Starting chain of 100 CT×CT mults by 1.01...\n";
        cout << "  " << setw(5) << "Step" << setw(16) << "Value(φ)" 
             << setw(16) << "Expected" << setw(14) << "Error\n";
        cout << "  " << string(55, '-') << "\n";
        
        int maxSteps = 100;
        double base_expected = 1.0;
        
        for (int i = 0; i <= maxSteps; i += 10) {
            double sig = val_phi(state);
            double expected = base_expected * pow(1.01, i) * pow(phi, boostSteps);
            double err = abs((sig - expected) / expected);
            
            cout << "  " << setw(5) << i
                 << setw(16) << fixed << setprecision(6) << sig
                 << setw(16) << expected
                 << setw(14) << scientific << setprecision(2) << err << "\n";
            
            // Do 10 more multiplications
            for (int m = 0; m < 10 && i + m < maxSteps; m++) {
                state = ctct_mul(state, multiplier);
            }
        }
        
        // Compensate
        for (int i = 0; i < boostSteps; i++) state = div_phi(state);
        
        double final_sig = val_phi(state);
        double final_expected = pow(1.01, maxSteps);
        double final_err = abs((final_sig - final_expected) / final_expected);
        
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FINAL: " << maxSteps << " CT×CT mults                                 ║\n";
        cout <<   "  ║   Value:  " << fixed << setprecision(8) << final_sig;
        for (int i = 0; i < (30 - to_string(final_sig).length()); i++) cout << " ";
        cout << "║\n";
        cout <<   "  ║   Expect: " << final_expected;
        for (int i = 0; i < (30 - to_string(final_expected).length()); i++) cout << " ";
        cout << "║\n";
        cout <<   "  ║   Error:  " << scientific << final_err;
        for (int i = 0; i < (30 - to_string(final_err).length()); i++) cout << " ";
        cout << "║\n";
        
        if (final_err < 0.01)
            cout << "  ║   *** " << maxSteps << " CT×CT WITH BOOSTED SPACE — CLEAN! ***       ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }
};

int main() {
    BoundedEngine E;
    E.test_bounded();
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

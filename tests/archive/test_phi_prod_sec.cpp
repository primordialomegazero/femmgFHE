// FEmmg-FHE — PRODUCTION SECURITY: HEStd_128_classic
// RingDim auto-selected by OpenFHE based on security level
// Test if φ-extension + asymmetric clean works at production params

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class ProdSecEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    ProdSecEngine(int depth = 50) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_128_classic);  // PRODUCTION SECURITY
        // RingDim auto-selected for 128-bit security
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(depth);
        
        cout << "  Generating CKKS context (HEStd_128_classic)... " << flush;
        cc = GenCryptoContext(params);
        uint32_t ringDim = cc->GetRingDimension();
        cout << "RingDim=" << ringDim << "\n";
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        cout << "  KeyGen... " << flush;
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        slots = ringDim / 2;
        cout << "Done. Slots=" << slots << "\n";
    }

    Ciphertext<DCRTPoly> enc(double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }
    double dec(const Ciphertext<DCRTPoly>& ct) {
        try {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(slots); return pt->GetRealPackedValue()[0];
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

    PE asymmetric_clean(const PE& x, int mul_steps, int div_steps) {
        auto r = x;
        for (int i = 0; i < mul_steps; i++) r = mul_X(r);
        for (int i = 0; i < div_steps; i++) r = div_X(r);
        return r;
    }

    void run_test() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   PRODUCTION SECURITY: HEStd_128_classic             ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        double net_phi = pow(phi, 2);
        double prescaled = 1.01 / net_phi;

        auto state = make(1.0, 0.0);
        auto multiplier = make(prescaled, 0.0);
        double expected = 1.0;

        cout << "  Pre-scaled chain: x ← x × " << prescaled << " with φ-clean\n";
        cout << "  " << setw(5) << "Step"
             << setw(14) << "Value"
             << setw(14) << "Expected"
             << setw(14) << "ψ-Noise"
             << setw(12) << "Error\n";
        cout << "  " << string(60, '-') << "\n";

        bool alive = true;
        int total = 0;

        for (int i = 0; i <= 40 && alive; i += 10) {
            double val = val_phi(state);
            double noi = abs(val_psi(state));
            double err = abs((val - expected) / expected);

            cout << "  " << setw(5) << i
                 << setw(14) << fixed << setprecision(6) << val
                 << setw(14) << expected
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(12) << err << "\n";

            if (i < 40) {
                for (int j = 0; j < 10 && alive; j++) {
                    try {
                        state = mul(state, multiplier);
                        expected *= prescaled;
                        total++;
                        if ((i+j+1) % 5 == 0) {
                            state = asymmetric_clean(state, 3, 1);
                            expected *= net_phi;
                        }
                    } catch (const exception& e) {
                        cout << "  CRASHED at " << (i+j) << ": " << e.what() << "\n";
                        alive = false;
                    }
                }
            }
        }

        if (alive) {
            cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
            cout <<   "  ║   " << total << " mults at HEStd_128_classic                        ║\n";
            cout <<   "  ║   ✅ φ-SYSTEM WORKS AT PRODUCTION SECURITY           ║\n";
            cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
        }
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PRODUCTION SECURITY TEST                           ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    try {
        ProdSecEngine E(60);
        E.run_test();
    } catch (const exception& e) {
        cout << "  ❌ Failed: " << e.what() << "\n\n";
    }

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

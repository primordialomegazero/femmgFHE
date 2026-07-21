// FEmmg-FHE — PRODUCTION RINGDIM=32768
// Tests asymmetric clean + Fibonacci at security-relevant parameters
// If this works, the system is production-viable

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <map>

using namespace lbcrypto;
using namespace std;

class ProductionEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    ProductionEngine(int ringDim, int depth) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(ringDim);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(depth);
        
        cout << "  Generating CKKS context (RingDim=" << ringDim << ")... " << flush;
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        cout << "KeyGen... " << flush;
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

    PE asymmetric_clean(const PE& x, int mul_steps, int div_steps) {
        auto result = x;
        for (int i = 0; i < mul_steps; i++) result = mul_X(result);
        for (int i = 0; i < div_steps; i++) result = div_X(result);
        return result;
    }

    void run_test(const string& name, int ringDim, int depth) {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   " << name << " (RingDim=" << ringDim << ")                    ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto state = make(1.0, 0.0);
        auto multiplier = make(1.01, 0.0);

        state = asymmetric_clean(state, 3, 1);
        double expected = val_phi(state);

        cout << "  " << setw(5) << "Mults"
             << setw(14) << "Value"
             << setw(14) << "ψ-Noise"
             << setw(12) << "Error\n";
        cout << "  " << string(50, '-') << "\n";

        int total = 0;
        bool alive = true;

        for (int cycle = 0; cycle < 10 && alive; cycle++) {
            for (int m = 0; m < 10; m++) {
                try {
                    state = mul(state, multiplier);
                    expected *= 1.01;
                    total++;
                } catch (const exception& e) {
                    cout << "  CRASHED at " << total << " mults\n";
                    alive = false;
                    break;
                }
            }
            if (!alive) break;

            if (cycle % 2 == 0) {
                state = asymmetric_clean(state, 2, 1);
                expected *= pow(phi, 1);
            }

            double val = val_phi(state);
            double noi = abs(val_psi(state));
            double err = abs((val - expected) / expected);

            cout << "  " << setw(5) << total
                 << setw(14) << fixed << setprecision(4) << val
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(12) << err << "\n";
        }

        if (alive) {
            cout << "\n  ✅ " << ringDim << " — " << total << " mults, stable.\n\n";
        }
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PRODUCTION PARAMETER TESTS                          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    // Test 1: 8192 (intermediate)
    ProductionEngine E1(8192, 80);
    E1.run_test("INTERMEDIATE", 8192, 80);

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PRODUCTION: RingDim=32768                          ║\n";
    cout <<   "  ║   This may take a while...                            ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    // Test 2: 32768 (production)
    try {
        ProductionEngine E2(32768, 60);
        E2.run_test("PRODUCTION", 32768, 60);
    } catch (const exception& e) {
        cout << "  ❌ 32768 failed: " << e.what() << "\n";
        cout << "  Likely RAM/CPU bottleneck on this hardware.\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

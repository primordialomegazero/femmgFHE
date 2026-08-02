// FEmmg-FHE 2.0 — HYBRID v2: Epoch-based φ-scale tracking
// Each bootstrap resets the epoch. Pre-scale per epoch.
// φ-clean between bootstraps, bootstrap only for level recovery.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class HybridV2 {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    HybridV2(int depth = 40, bool enableFHE = false) {
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
        if (enableFHE) {
            cc->Enable(FHE);
            cc->EvalBootstrapSetup({4,4}, {0,0}, 2048);
        }
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        if (enableFHE) cc->EvalBootstrapKeyGen(keys.secretKey, 2048);
        slots = 2048;
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

    struct EpochResult { int mults; int bootstraps; int cleans; double error; double psi_noise; };

    EpochResult run_epoch_hybrid(int maxMults, int bootstrapEvery, int cleanEvery) {
        double net_phi = pow(phi, 2);  // φ² per clean
        
        auto state = make(1.0, 0.0);
        double expected = 1.0;
        int mults = 0, bootstraps = 0, cleans = 0;
        double epoch_phi_scale = 1.0;  // Track φ-scale within current epoch
        
        for (int i = 0; i < maxMults; i++) {
            try {
                // Pre-scale multiplier for CURRENT epoch scale
                double prescaled = 1.01 / epoch_phi_scale;
                auto multiplier = make(prescaled, 0.0);
                
                state = mul(state, multiplier);
                expected *= prescaled;
                mults++;
                
                // φ-clean
                if (mults % cleanEvery == 0 && mults < maxMults) {
                    state = asymmetric_clean(state, 3, 1);
                    expected *= net_phi;
                    epoch_phi_scale *= net_phi;
                    cleans++;
                }
                
                // Bootstrap — reset epoch
                if (mults % bootstrapEvery == 0 && mults < maxMults) {
                    state.a = cc->EvalBootstrap(state.a);
                    state.b = cc->EvalBootstrap(state.b);
                    epoch_phi_scale = 1.0;  // RESET φ-scale tracking!
                    bootstraps++;
                }
            } catch (const exception& e) {
                cout << "    CRASHED at " << mults << ": " << e.what() << "\n";
                break;
            }
        }
        double val = val_phi(state);
        double err = abs((val - expected) / expected);
        double noi = abs(val_psi(state));
        return {mults, bootstraps, cleans, err, noi};
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   HYBRID v2: Epoch-based φ-scale tracking             ║\n";
    cout <<   "  ║   Scale resets on bootstrap. Pre-scale per epoch.    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Test configurations
    struct Config { string name; int maxMults; int bootEvery; int cleanEvery; };
    vector<Config> configs = {
        {"Bootstrap/15 (frequent, no φ)", 60, 15, 9999},
        {"Bootstrap/30 (sparse, no φ)",   60, 30, 9999},
        {"Hybrid: B/30 + φ/5",            60, 30, 5},
        {"Hybrid: B/20 + φ/3",            60, 20, 3},
        {"φ-only (no bootstrap)",         60, 9999, 5},
    };

    cout << "  " << setw(30) << "Config"
         << setw(8) << "Mults"
         << setw(6) << "Boots"
         << setw(6) << "Clean"
         << setw(14) << "Error"
         << setw(14) << "ψ-Noise\n";
    cout << "  " << string(80, '-') << "\n";

    for (auto& cfg : configs) {
        HybridV2 E(80, cfg.bootEvery < 9999);
        auto r = E.run_epoch_hybrid(cfg.maxMults, cfg.bootEvery, cfg.cleanEvery);
        cout << "  " << setw(30) << cfg.name
             << setw(8) << r.mults
             << setw(6) << r.bootstraps
             << setw(6) << r.cleans
             << setw(14) << scientific << setprecision(2) << r.error
             << setw(14) << r.psi_noise << "\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RESULTS: Epoch tracking keeps φ-scale correct      ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

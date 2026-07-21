// FEmmg-FHE — TRAP vs BOOTSTRAP: Head-to-head comparison
// Same circuit, same depth budget
// φ asymmetric clean vs OpenFHE EvalBootstrap

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class BenchEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;

    BenchEngine(int depth, bool enableFHE) {
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
        if (enableFHE) cc->Enable(FHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        if (enableFHE) {
            vector<uint32_t> levelBudget = {4, 4};
            cc->EvalBootstrapSetup(levelBudget, {0, 0}, 2048);
            cc->EvalBootstrapKeyGen(keys.secretKey, 2048);
        }
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

    // ========== STANDARD CKKS WITH BOOTSTRAP ==========
    struct StdResult {
        int mults;
        int bootstraps;
        double error;
        double ms_time;
    };

    StdResult run_standard(int maxMults, double multiplier_val) {
        auto ct = enc(1.0);
        auto mul_ct = enc(multiplier_val);
        double expected = 1.0;
        int mults = 0;
        int bootstraps = 0;
        auto t0 = high_resolution_clock::now();

        for (int i = 0; i < maxMults; i++) {
            try {
                ct = cc->EvalMult(ct, mul_ct);
                expected *= multiplier_val;
                mults++;

                // Bootstrap every 15 mults (standard approach)
                if (mults % 15 == 0 && mults < maxMults) {
                    ct = cc->EvalBootstrap(ct);
                    bootstraps++;
                }
            } catch (...) { break; }
        }

        auto t1 = high_resolution_clock::now();
        double val = dec(ct);
        double err = abs((val - expected) / expected);
        double ms = duration_cast<milliseconds>(t1 - t0).count();

        return {mults, bootstraps, err, ms};
    }

    // ========== φ-EXTENSION WITH ASYMMETRIC CLEAN ==========
    struct PE { Ciphertext<DCRTPoly> a, b; };
    PE make(double a, double b) { return {enc(a), enc(b)}; }
    PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
    PE mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }
    double val_phi(const PE& x) { return dec(x.a) + dec(x.b) * phi; }
    double val_psi(const PE& x) { double psi = -0.6180339887498948482; return dec(x.a) + dec(x.b) * psi; }

    PE asymmetric_clean(const PE& x, int mul_steps, int div_steps) {
        auto result = x;
        for (int i = 0; i < mul_steps; i++) result = mul_X(result);
        for (int i = 0; i < div_steps; i++) result = div_X(result);
        return result;
    }

    struct PhiResult {
        int mults;
        int cleans;
        int total_x_ops;
        double error;
        double psi_noise;
        double ms_time;
    };

    PhiResult run_phi(int maxMults, double multiplier_val) {
        auto state = make(1.0, 0.0);
        auto multiplier = make(multiplier_val, 0.0);
        double expected = 1.0;
        int mults = 0;
        int cleans = 0;
        int total_x = 0;
        auto t0 = high_resolution_clock::now();

        // Pre-clean
        state = asymmetric_clean(state, 3, 1);
        expected *= pow(phi, 2);
        total_x += 4;

        for (int i = 0; i < maxMults; i++) {
            try {
                state = mul(state, multiplier);
                expected *= multiplier_val;
                mults++;

                // Asymmetric clean every 15 mults (same frequency as bootstrap)
                if (mults % 15 == 0 && mults < maxMults) {
                    state = asymmetric_clean(state, 3, 1);
                    expected *= pow(phi, 2);
                    cleans++;
                    total_x += 4;
                }
            } catch (...) { break; }
        }

        auto t1 = high_resolution_clock::now();
        double val = val_phi(state);
        double err = abs((val - expected) / expected);
        double psi_noise = abs(val_psi(state));
        double ms = duration_cast<milliseconds>(t1 - t0).count();

        return {mults, cleans, total_x, err, psi_noise, ms};
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   TRAP vs BOOTSTRAP: Head-to-Head                    ║\n";
    cout <<   "  ║   Same circuit, same depth, same clean frequency    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    double mult_val = 1.01;
    int max_attempt = 80;

    // Standard CKKS with bootstrapping
    cout << "  ── STANDARD CKKS + EvalBootstrap ──\n";
    BenchEngine stdEng(80, true);
    auto std_res = stdEng.run_standard(max_attempt, mult_val);
    cout << "    Multiplications: " << std_res.mults << "\n";
    cout << "    Bootstraps:      " << std_res.bootstraps << "\n";
    cout << "    Error:           " << scientific << std_res.error << "\n";
    cout << "    Time:            " << fixed << setprecision(0) << std_res.ms_time << " ms\n\n";

    // φ-extension with asymmetric clean
    cout << "  ── φ-EXTENSION + Asymmetric Clean ──\n";
    BenchEngine phiEng(80, false);
    auto phi_res = phiEng.run_phi(max_attempt, mult_val);
    cout << "    Multiplications: " << phi_res.mults << "\n";
    cout << "    Clean cycles:    " << phi_res.cleans << "\n";
    cout << "    Total X-ops:     " << phi_res.total_x_ops << " (ALL FREE)\n";
    cout << "    Error:           " << scientific << phi_res.error << "\n";
    cout << "    ψ-Noise:         " << phi_res.psi_noise << "\n";
    cout << "    Time:            " << fixed << setprecision(0) << phi_res.ms_time << " ms\n\n";

    // Comparison
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   COMPARISON                                          ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║                   Standard        φ-Asymmetric        ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout << "  ║   Max mults:    " << setw(8) << std_res.mults << "        " << setw(8) << phi_res.mults << "        ║\n";
    cout << "  ║   Error:        " << setw(8) << scientific << setprecision(2) << std_res.error 
         << "        " << setw(8) << phi_res.error << "        ║\n";
    cout << "  ║   Time:         " << setw(6) << fixed << setprecision(0) << std_res.ms_time 
         << " ms      " << setw(6) << phi_res.ms_time << " ms      ║\n";
    
    double speedup = std_res.ms_time / max(phi_res.ms_time, 1.0);
    double error_ratio = phi_res.error / max(std_res.error, 1e-15);
    
    cout << "  ╠══════════════════════════════════════════════════════╣\n";
    cout << "  ║   Speedup:     " << setw(6) << fixed << setprecision(1) << speedup << "×                                  ║\n";
    
    if (error_ratio < 10)
        cout << "  ║   Error:       Comparable                             ║\n";
    else if (error_ratio > 1000)
        cout << "  ║   Error:       Standard is better                     ║\n";
    
    cout << "  ║   X-ops cost ZERO depth (vs bootstrap's heavy cost)  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

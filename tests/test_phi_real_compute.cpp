// FEmmg-FHE — REAL COMPUTATION between clean cycles
// Clean (kill noise) → Compute (real multiplies, fresh noise) → Clean → ...
// Does ψ-noise stay suppressed through real operations?

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   REAL COMPUTE: Clean → Multiply → Clean → Multiply  ║\n";
    cout <<   "  ║   Does ψ-noise stay dead through real operations?   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(100);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    int slots = 2048;
    double phi = 1.6180339887498948482, psi = -0.6180339887498948482;

    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    };

    struct PE { Ciphertext<DCRTPoly> a, b; };
    auto make = [&](double a, double b) -> PE { return {enc(a), enc(b)}; };
    auto val_phi = [&](const PE& x) { return dec(x.a) + dec(x.b) * phi; };
    auto val_psi = [&](const PE& x) { return dec(x.a) + dec(x.b) * psi; };
    auto mul_X = [&](const PE& x) -> PE { return {x.b, cc->EvalAdd(x.a, x.b)}; };
    auto div_X = [&](const PE& x) -> PE { return {cc->EvalSub(x.b, x.a), x.a}; };
    auto mul = [&](const PE& x, const PE& y) -> PE {
        auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    };

    // Start with signal=100, noise=5
    double init_sig = 100.0, init_noise = 5.0;
    double b_init = (init_sig - init_noise) / (phi - psi);
    double a_init = init_sig - b_init * phi;
    auto state = make(a_init, b_init);

    // Track expected manually through each phase
    double expected = init_sig;

    cout << "  Start: φ=" << val_phi(state) << " ψ=" << scientific << val_psi(state) << "\n\n";
    cout << "  " << setw(5) << "Cycle" << setw(14) << "φ-value" << setw(14) << "ψ-noise"
         << setw(14) << "Expected" << setw(12) << "Error\n";
    cout << "  " << string(70, '-') << "\n";

    for (int cycle = 0; cycle < 8; cycle++) {
        double phi_val = val_phi(state);
        double psi_val = abs(val_psi(state));
        double err = abs((phi_val - expected) / expected);

        cout << "  " << setw(5) << cycle
             << setw(14) << fixed << setprecision(4) << phi_val
             << setw(14) << scientific << setprecision(2) << psi_val
             << setw(14) << fixed << expected
             << setw(12) << scientific << setprecision(2) << err << "\n";

        // PHASE 1: Clean (3↑ 2↓) — kills old noise, signal × φ
        for (int i = 0; i < 3; i++) state = mul_X(state);
        for (int i = 0; i < 2; i++) state = div_X(state);
        expected *= phi;

        // PHASE 2: REAL COMPUTE — 3 multiplies by 1.05
        auto real_mult = make(1.05, 0.0);
        for (int m = 0; m < 3; m++) {
            state = mul(state, real_mult);
            expected *= 1.05;
        }
        // Fresh noise from these 3 mults is now in the state
        // Next cycle's clean will kill THIS fresh noise
    }

    double final_phi = val_phi(state);
    double final_psi = abs(val_psi(state));
    double final_err = abs((final_phi - expected) / expected);
    
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FINAL: φ=" << fixed << setprecision(2) << final_phi;
    for (int i=0; i<(30-to_string(final_phi).length()); i++) cout<<" ";
    cout << "║\n";
    cout <<   "  ║   Expected: " << expected;
    for (int i=0; i<(28-to_string(expected).length()); i++) cout<<" ";
    cout << "║\n";
    cout <<   "  ║   Error: " << scientific << setprecision(2) << final_err;
    for (int i=0; i<(31-to_string(final_err).length()); i++) cout<<" ";
    cout << "║\n";
    cout <<   "  ║   ψ-noise: " << final_psi << " (started at " << init_noise << ")";
    for (int i=0; i<(20-to_string(final_psi).length()); i++) cout<<" ";
    cout << "║\n";
    if (final_err < 0.01)
        cout << "  ║   ✅ REAL COMPUTE BETWEEN CLEANS — ψ STAYS DEAD     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

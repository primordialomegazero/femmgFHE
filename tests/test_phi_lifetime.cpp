// FEmmg-FHE — NOISE LIFETIME: Kill old, compute, post-scale, repeat
// Old noise killed by clean doesn't come back.
// Fresh noise enters during compute. Next clean kills it.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   NOISE LIFETIME: Kill old, compute, post-scale      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Setup
    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(80);
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
    auto add = [&](const PE& x, const PE& y) -> PE { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; };
    auto mul = [&](const PE& x, const PE& y) -> PE {
        auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    };

    // Strategy per cycle:
    // 1. Clean: 3↑ 2↓ (kill old noise, signal × φ)
    // 2. Compute: 5 multiplies (fresh noise enters)
    // 3. Post-scale: 1 div_X (signal / φ, back to original scale)
    //    Old noise was killed in step 1. Fresh noise from step 2 is scaled by 1/φ too.
    //    But wait - div_X multiplies ψ by ψ⁻¹ = -φ. Fresh noise GROWS.
    // 
    // ALTERNATIVE: Post-scale on the MULTIPLIER side.
    // After clean, the signal is φ× too large.
    // We can compensate by making the multiplier φ× SMALLER for the next epoch.
    // This is pre-scaling per epoch, not per multiply!
    
    auto state = make(42.0, 0.5);  // Signal=42, noise=0.5 in ψ
    double expected = 42.0;
    double base_mult = 1.01;
    
    cout << "  Start: φ=" << val_phi(state) << " ψ=" << val_psi(state) << "\n\n";
    cout << "  " << setw(5) << "Cycle" << setw(10) << "φ-value" << setw(14) << "ψ-noise"
         << setw(14) << "Expected\n";
    cout << "  " << string(50, '-') << "\n";

    for (int cycle = 0; cycle < 10; cycle++) {
        double phi_val = val_phi(state);
        double psi_val = abs(val_psi(state));
        double err = abs((phi_val - expected) / expected);
        
        cout << "  " << setw(5) << cycle
             << setw(10) << fixed << setprecision(4) << phi_val
             << setw(14) << scientific << setprecision(2) << psi_val
             << setw(14) << fixed << expected << "\n";

        // Step 1: Clean (3↑ 2↓)
        for (int i = 0; i < 3; i++) state = mul_X(state);
        for (int i = 0; i < 2; i++) state = div_X(state);
        expected *= phi;  // Net φ-growth
        
        // Step 2: Compute with EPOCH pre-scaling
        // After clean, signal is φ× too big. Use multiplier = base/φ for this epoch.
        double epoch_mult = base_mult / phi;
        auto mult_enc = make(epoch_mult, 0.0);
        
        for (int m = 0; m < 5; m++) {
            state = mul(state, mult_enc);
            expected *= epoch_mult;
        }
        
        // Signal should now be at expected (clean grew by φ, epoch_mult/φ compensated)
        // Next clean will kill the fresh noise from these 5 mults
    }

    double final_phi = val_phi(state);
    double final_psi = abs(val_psi(state));
    cout << "\n  Final: φ=" << fixed << final_phi << " (exp " << expected << ")"
         << " err=" << scientific << abs((final_phi-expected)/expected) << "\n";
    cout << "  ψ-noise: " << final_psi << " (started at 0.5)\n\n";

    return 0;
}

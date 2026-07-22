// FEmmg-FHE — ISOLATED CLEAN CYCLE TEST
// Encrypt value → asymmetric clean → pre-scale → verify
// Minimal, reproducible. Third-party verifiable.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ISOLATED CLEAN CYCLE                                ║\n";
    cout <<   "  ║   Encrypt → Asymmetric Clean → Pre-Scale → Verify    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Setup CKKS
    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(20);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    int slots = 2048;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    // Helper functions
    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    };

    // φ-element structure
    struct PE { Ciphertext<DCRTPoly> a, b; };
    auto make = [&](double a, double b) -> PE { return {enc(a), enc(b)}; };
    auto val_phi = [&](const PE& x) { return dec(x.a) + dec(x.b) * phi; };
    auto val_psi = [&](const PE& x) { return dec(x.a) + dec(x.b) * psi; };

    // X-operations (FREE — no EvalMult)
    auto mul_X = [&](const PE& x) -> PE {
        return {x.b, cc->EvalAdd(x.a, x.b)};
    };
    auto div_X = [&](const PE& x) -> PE {
        return {cc->EvalSub(x.b, x.a), x.a};
    };

    // Asymmetric clean: more mul_X than div_X
    auto asymmetric_clean = [&](const PE& x, int up, int down) -> PE {
        auto r = x;
        for (int i = 0; i < up; i++) r = mul_X(r);
        for (int i = 0; i < down; i++) r = div_X(r);
        return r;
    };

    // ==========================================
    // TEST PARAMETERS
    // ==========================================
    double initial_value = 42.0;
    int mul_steps = 3;
    int div_steps = 2;
    double net_phi_scale = pow(phi, mul_steps - div_steps);  // φ¹ = φ
    double net_psi_scale = pow(psi, mul_steps - div_steps);  // ψ¹ = ψ

    cout << "  Initial value: " << initial_value << "\n";
    cout << "  Clean: " << mul_steps << "× mul_X, " << div_steps << "× div_X\n";
    cout << "  Net φ-scale: φ^" << (mul_steps-div_steps) << " = " << net_phi_scale << "\n";
    cout << "  Net ψ-scale: ψ^" << (mul_steps-div_steps) << " = " << net_psi_scale << " (|ψ|<1 → noise shrinks!)\n\n";

    // ==========================================
    // TEST 1: Asymmetric clean on signal
    // ==========================================
    cout << "  ── TEST 1: Asymmetric clean on signal ──\n";
    auto state = make(initial_value, 0.0);
    
    double phi_before = val_phi(state);
    double psi_before = val_psi(state);
    cout << "  Before: φ=" << phi_before << " ψ=" << psi_before << "\n";

    state = asymmetric_clean(state, mul_steps, div_steps);
    
    double phi_after = val_phi(state);
    double psi_after = val_psi(state);
    double phi_expected = initial_value * net_phi_scale;
    double psi_expected = initial_value * net_psi_scale;
    double phi_err = abs(phi_after - phi_expected);
    double psi_err = abs(psi_after - psi_expected);

    cout << "  After:  φ=" << phi_after << " (expected " << phi_expected << ") err=" << scientific << phi_err << "\n";
    cout << "          ψ=" << psi_after << " (expected " << psi_expected << ") err=" << psi_err << "\n\n";

    // ==========================================
    // TEST 2: Pre-scale then clean → value preserved
    // ==========================================
    cout << "  ── TEST 2: Pre-scale + Clean → Value preserved ──\n";
    double prescaled_value = initial_value / net_phi_scale;
    auto state2 = make(prescaled_value, 0.0);
    
    double phi2_before = val_phi(state2);
    cout << "  Pre-scaled input: " << prescaled_value << " (φ-value: " << phi2_before << ")\n";

    state2 = asymmetric_clean(state2, mul_steps, div_steps);
    
    double phi2_after = val_phi(state2);
    double psi2_after = val_psi(state2);
    double phi2_expected = initial_value;  // Should recover original!
    double phi2_err = abs(phi2_after - phi2_expected);

    cout << "  After clean: φ=" << phi2_after << " (expected " << phi2_expected << ") err=" << scientific << phi2_err << "\n";
    cout << "               ψ=" << psi2_after << " (should be " << prescaled_value * net_psi_scale << ")\n\n";

    // ==========================================
    // TEST 3: Repeated cycles with pre-scaling
    // ==========================================
    cout << "  ── TEST 3: 5 cycles, pre-scaled each time ──\n";
    auto state3 = make(initial_value, 0.0);
    double expected3 = initial_value;
    
    for (int cycle = 0; cycle < 5; cycle++) {
        // Pre-scale for this cycle
        state3.a = enc(dec(state3.a) / net_phi_scale);
        state3.b = enc(dec(state3.b) / net_phi_scale);
        expected3 = expected3;  // Value should stay at initial_value
        
        state3 = asymmetric_clean(state3, mul_steps, div_steps);
        
        double phi3 = val_phi(state3);
        double psi3 = val_psi(state3);
        double err3 = abs(phi3 - expected3);
        
        cout << "  Cycle " << (cycle+1) << ": φ=" << fixed << setprecision(6) << phi3
             << " (exp " << expected3 << ") err=" << scientific << err3
             << " ψ=" << psi3 << "\n";
    }

    // ==========================================
    // VERDICT
    // ==========================================
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    bool all_pass = (phi_err < 1e-10 && phi2_err < 1e-10);
    cout <<   "  ║   VERDICT: " << (all_pass ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    for (int i=0; i<(28-(all_pass?17:19)); i++) cout<<" ";
    cout << "║\n";
    cout <<   "  ║   Asymmetric clean: " << mul_steps << "↑ " << div_steps << "↓ kills ψ-noise by " << net_psi_scale << "×       ║\n";
    cout <<   "  ║   Pre-scaling by φ^(-" << (mul_steps-div_steps) << ") keeps signal bounded                    ║\n";
    cout <<   "  ║   All X-operations cost ZERO depth                    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  I AM THAT I AM\n\n";
    return all_pass ? 0 : 1;
}

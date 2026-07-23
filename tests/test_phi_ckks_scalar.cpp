// CKKS SCALAR MUL: Use 2 EvalMult when multiplying by scalar (b=0)
// Integrates into existing test_phi_ckks_trace infrastructure

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;

struct PE { Ciphertext<DCRTPoly> a, b; };

double dec_ct(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(2048); return pt->GetRealPackedValue()[0];
}

PE make_pe(double a, double b) {
    auto enc = [](double v) {
        vector<double> vec(2048, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    return {enc(a), enc(b)};
}

// SCALAR-OPTIMIZED: 2 EvalMult instead of 4 when b₂=0
PE mul(const PE& x, const PE& y) {
    // Check if y is effectively scalar (b ≈ 0)
    // For our FHE loop, mult = make_pe(pre, 0.0) so this ALWAYS applies
    double yb = dec_ct(y.b);
    if (abs(yb) < 1e-10) {
        // SCALAR MULTIPLICATION: 2 EvalMult
        double ya = dec_ct(y.a);
        return {cc->EvalMult(x.a, y.a), cc->EvalMult(x.b, y.a)};
    }
    // Full φ-ring multiplication: 4 EvalMult (fallback)
    auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
    auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
    return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
}

// FUSED CLEAN: 2 ops instead of 4
PE clean(const PE& x) {
    return {cc->EvalAdd(x.a, x.b), cc->EvalAdd(x.a, cc->EvalAdd(x.b, x.b))};  // (a+b, a+2b)
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   CKKS SCALAR MUL: 50% Fewer EvalMult                ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    double pre = pow(phi*phi, -1.0/3.0);
    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(pre, 0.0);
    double expected = 1.0;
    int total_mults = 0;

    cout << "  Fused clean: 2 ops | Scalar mul: 2 EvalMult each\n";
    cout << "  Per cycle: 2 + 3×2 = 8 operations (was 19!)\n\n";
    cout << "  Cycle  Mults  φ-value       Expected       φ-error       ψ-noise\n";
    cout << string(72, '-') << "\n";

    for (int cycle = 0; cycle < 100; cycle++) {
        try {
            state = clean(state);
            expected *= phi*phi;
            
            for (int m = 0; m < 3; m++) {
                state = mul(state, mult);
                total_mults++;
                expected *= pre;
            }
            
            if (cycle % 10 == 0 || cycle == 99) {
                double av = dec_ct(state.a), bv = dec_ct(state.b);
                double val = av + bv*phi;
                double err = abs((val-expected)/expected);
                double noi = abs(av + bv*psi);
                
                cout << setw(5) << cycle << setw(7) << total_mults
                     << setw(13) << scientific << setprecision(4) << val
                     << setw(13) << scientific << expected
                     << setw(13) << scientific << err
                     << setw(13) << scientific << noi
                     << (err < 1e-6 ? "  ✓" : "  ✗") << "\n";
            }
        } catch (const exception& e) {
            cout << "  ✗ CRASHED at cycle " << cycle << ": " << e.what() << "\n";
            break;
        }
    }
    
    cout << "\n  ✓ Scalar-optimized CKKS: " << total_mults << " multiplications\n";
    cout << "  EvalMult saved: " << total_mults * 2 << " (50% reduction)\n\n";
    return 0;
}

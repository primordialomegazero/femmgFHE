// BOOTSTRAP-FREE V2: Fixed pre-scaling to compensate φ-growth
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;

struct PE { Ciphertext<DCRTPoly> a, b; };
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;

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
PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

PE clean(const PE& x, int ratio = 3) {
    PE y = x;
    for (int i = 0; i < ratio; i++) y = mul_X(y);
    y = div_X(y);
    return y;
}

PE mul(const PE& x, const PE& y) {
    auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
    auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
    return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BOOTSTRAP-FREE V2: Fixed pre-scaling               ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    // Clean ratio: 3 mul_X : 1 div_X
    // Clean growth in φ: φ² ≈ 2.618
    // With 3 mults per cycle: need pre³ × φ² ≈ 1.0
    // pre = φ^(-2/3) ≈ 0.726
    double clean_growth = phi * phi;  // 2.618
    double pre = pow(clean_growth, -1.0/3.0);  // 0.726
    
    cout << "  Clean growth: φ² = " << clean_growth << "\n";
    cout << "  Optimal pre: φ^(-2/3) = " << pre << "\n";
    cout << "  Net per cycle: φ² × pre³ = " << clean_growth * pre * pre * pre << "\n\n";

    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(pre, 0.0);
    double expected = 1.0;
    int total_mults = 0;

    cout << "  Cycle  Mults  φ-value      Expected      φ-error      ψ-noise      Status\n";
    cout << string(85, '-') << "\n";

    for (int cycle = 0; cycle < 100; cycle++) {
        try {
            state = clean(state, 3);
            expected *= clean_growth;
            
            for (int m = 0; m < 3; m++) {
                state = mul(state, mult);
                total_mults++;
                expected *= pre;
            }
            
            double av = dec_ct(state.a), bv = dec_ct(state.b);
            double val = av + bv * phi;
            double err = abs((val - expected) / expected);
            double noi = abs(av + bv * psi);
            
            string status = "✓";
            if (err > 1e-4) status = "φ-WARN";
            if (noi > 1e-4) status = "ψ-WARN";
            if (err > 1e-1) status = "DIVERGED";
            
            if (cycle % 10 == 0 || status != "✓") {
                cout << setw(5) << cycle << setw(7) << total_mults
                     << setw(13) << scientific << setprecision(4) << val
                     << setw(13) << scientific << expected
                     << setw(13) << scientific << err
                     << setw(13) << scientific << noi
                     << "  " << status << "\n";
            }
            
            if (status == "DIVERGED") {
                cout << "\n  ⚠ Diverged at cycle " << cycle << " (" << total_mults << " mults)\n";
                break;
            }
        } catch (const exception& e) {
            cout << "\n  ✗ CRASHED at cycle " << cycle << ": " << e.what() << "\n";
            break;
        }
    }
    cout << "\n  Total: " << total_mults << " multiplications, ZERO bootstraps\n\n";
    return 0;
}

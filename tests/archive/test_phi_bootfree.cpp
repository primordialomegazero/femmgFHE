// BOOTSTRAP-FREE: How far can we go without ANY bootstrap?
// Uses aggressive clean + pre-scaling only
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;

struct PE { Ciphertext<DCRTPoly> a, b; };

double dec_ct(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keys, const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(2048); return pt->GetRealPackedValue()[0];
}

PE make_pe(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keys, double a, double b) {
    auto enc = [&](double v) {
        vector<double> vec(2048, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    return {enc(a), enc(b)};
}

PE mul_X(CryptoContext<DCRTPoly>& cc, const PE& x) { 
    return {x.b, cc->EvalAdd(x.a, x.b)}; 
}

PE div_X(CryptoContext<DCRTPoly>& cc, const PE& x) { 
    return {cc->EvalSub(x.b, x.a), x.a}; 
}

PE clean_aggressive(CryptoContext<DCRTPoly>& cc, PE x) {
    // 10 mul_X : 1 div_X — maximally asymmetric
    for (int i = 0; i < 10; i++) x = mul_X(cc, x);
    x = div_X(cc, x);
    return x;
}

PE mul(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
    auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
    return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BOOTSTRAP-FREE: How far without bootstrap?         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    double pre = 0.95;  // Aggressive pre-scale
    auto state = make_pe(cc, keys, 1.0, 0.0);
    auto mult = make_pe(cc, keys, pre, 0.0);
    double expected = 1.0;
    int total_mults = 0;

    cout << "  Cycle  Mults  φ-value      Expected      φ-error      ψ-noise      Status\n";
    cout << string(85, '-') << "\n";

    for (int cycle = 0; cycle < 100; cycle++) {
        try {
            state = clean_aggressive(cc, state);
            
            for (int m = 0; m < 3; m++) {
                state = mul(cc, state, mult);
                total_mults++;
                expected *= pre;
            }
            
            double av = dec_ct(cc, keys, state.a);
            double bv = dec_ct(cc, keys, state.b);
            double val = av + bv * phi;
            double err = abs((val - expected) / expected);
            double noi = abs(av + bv * psi);
            
            string status = "✓";
            if (err > 1e-6) status = "φ-ERROR";
            if (noi > 1e-6) status = "ψ-NOISE";
            if (err > 1e-2) status = "DIVERGED";
            
            cout << setw(5) << cycle << setw(8) << total_mults
                 << setw(13) << scientific << setprecision(4) << val
                 << setw(13) << scientific << expected
                 << setw(13) << scientific << err
                 << setw(13) << scientific << noi
                 << "  " << status << "\n";
            
            if (status == "DIVERGED") {
                cout << "\n  ⚠ Diverged at cycle " << cycle 
                     << " (" << total_mults << " total mults, ZERO bootstraps)\n";
                break;
            }
            
            if (cycle == 99) {
                cout << "\n  ✓ SURVIVED 100 cycles (" << total_mults 
                     << " mults, ZERO bootstraps)\n";
            }
        } catch (const exception& e) {
            cout << "\n  ✗ CRASHED at cycle " << cycle << ": " << e.what() << "\n";
            break;
        }
    }
    cout << "\n";
    return 0;
}

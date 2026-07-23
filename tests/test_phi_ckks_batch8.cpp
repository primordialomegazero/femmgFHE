// BATCH 8: Aggressive — 8 mults per clean cycle
// Pushing to the limit while ψ-noise stays acceptable

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

PE mul_scalar(const PE& x, double s) {
    auto ct_s = make_pe(s, 0.0);
    return {cc->EvalMult(x.a, ct_s.a), cc->EvalMult(x.b, ct_s.a)};
}

PE clean(const PE& x) {
    auto a_plus_b = cc->EvalAdd(x.a, x.b);
    auto two_b = cc->EvalAdd(x.b, x.b);
    return {a_plus_b, cc->EvalAdd(x.a, two_b)};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BATCH 8: Maximum throughput (aggressive)            ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    int batch = 8;
    double pre = pow(phi*phi, -1.0/(double)batch);
    auto state = make_pe(1.0, 0.0);
    double expected = 1.0;
    int total_mults = 0;

    cout << "  Pre-scale: φ^(-2/" << batch << ") = " << fixed << setprecision(6) << pre << endl;
    cout << "  Ops per mult: " << fixed << setprecision(2) << (2.0 + batch*2.0)/batch << " (vs 2.67 baseline)" << endl;
    cout << "  Throughput gain: " << fixed << setprecision(0) << (1.0 - (2.0+batch*2.0)/batch / 2.67)*100 << "%" << endl << endl;

    cout << "  Cycle  Mults  φ-error       ψ-noise       Status\n";
    cout << string(55, '-') << "\n";

    for (int cycle = 0; cycle < 40; cycle++) {
        try {
            state = clean(state);
            expected *= phi*phi;
            
            for (int m = 0; m < batch; m++) {
                state = mul_scalar(state, pre);
                total_mults++;
                expected *= pre;
            }
            
            if (cycle % 3 == 0 || cycle == 39) {
                double av = dec_ct(state.a), bv = dec_ct(state.b);
                double err = abs((av + bv*phi - expected)/expected);
                double noi = abs(av + bv*psi);
                
                cout << setw(5) << cycle << setw(7) << total_mults
                     << setw(13) << scientific << setprecision(2) << err
                     << setw(13) << scientific << noi;
                
                if (err < 1e-8 && noi < 1e-4) cout << "  ✓";
                else if (err < 1e-4) cout << "  ~";
                else cout << "  ✗";
                cout << "\n";
                cout.flush();
            }
        } catch (const exception& e) {
            cout << "  ✗ CRASHED at " << total_mults << " mults\n";
            break;
        }
    }
    
    cout << "\n  ✓ Batch 8: " << total_mults << " multiplications\n\n";
    return 0;
}

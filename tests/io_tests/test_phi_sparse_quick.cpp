#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;
const uint32_t RD = 8192, SLOTS = RD/2;

struct PE { Ciphertext<DCRTPoly> a, b; };
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;

double dec_ct(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(SLOTS); return pt->GetRealPackedValue()[0];
}
PE make_pe(double a, double b) {
    auto enc = [](double v) {
        vector<double> vec(SLOTS, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    return {enc(a), enc(b)};
}
PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
PE clean(const PE& x) { PE y=x; for(int i=0;i<3;i++) y=mul_X(y); return div_X(y); }
PE mul(const PE& x, const PE& y) {
    auto ac=cc->EvalMult(x.a,y.a), bd=cc->EvalMult(x.b,y.b);
    auto ad=cc->EvalMult(x.a,y.b), bc=cc->EvalMult(x.b,y.a);
    return {cc->EvalAdd(ac,bd), cc->EvalAdd(cc->EvalAdd(ad,bc),bd)};
}

int main() {
    cout << "\n  SPARSE QUICK: Max mults before bootstrap needed\n";
    cout << "  RingDim=8192 | Clean=3:1 | No bootstrap\n";
    cout << "  Setup..." << flush;

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(RD); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cout << " Done.\n\n";

    double growth = phi*phi, pre = pow(growth, -1.0/3.0);
    auto mult = make_pe(pre, 0.0);
    
    vector<int> targets = {60, 120, 180, 240, 300, 400, 500};
    
    for (int target : targets) {
        auto state = make_pe(1.0, 0.0);
        double expected = 1.0;
        bool crashed = false;
        
        for (int i = 0; i < target/3 && !crashed; i++) {
            try {
                state = clean(state); expected *= growth;
                for (int m=0; m<3; m++) { 
                    state = mul(state, mult); 
                    expected *= pre; 
                }
            } catch (const exception& e) {
                crashed = true;
            }
        }
        
        if (crashed) {
            cout << "  " << setw(5) << target << " mults: CRASHED (depth limit)\n";
            break;
        }
        
        double av = dec_ct(state.a), bv = dec_ct(state.b);
        double err = abs((av + bv*phi - expected)/expected);
        double noi = abs(av + bv*psi);
        bool ok = (err < 1e-2 && noi < 1e-2);
        
        cout << "  " << setw(5) << target << " mults: err=" << scientific << setprecision(2) 
             << err << " noi=" << noi << " " << (ok ? "OK" : "FAIL") << "\n";
        cout.flush();
        
        if (!ok) break;
    }
    cout << "\n";
    return 0;
}

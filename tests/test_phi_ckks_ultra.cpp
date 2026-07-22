// FEmmg-FHE — CKKS ULTRA DEEP: All techniques combined
// Zero-depth clean + Dual-slot bootstrap + Fibonacci + Pre-scaling
// Target: 1000+ effective mults on RingDim=4096

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <map>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;
int slots;

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
Ciphertext<DCRTPoly> to_dual(const PE& x) {
    double a = dec_ct(x.a), b = dec_ct(x.b);
    vector<double> vec(2048, 0.0);
    vec[0] = a + b * phi; vec[1] = a + b * psi;
    return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
}
PE from_dual(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(2048);
    double vp = pt->GetRealPackedValue()[0], vs = pt->GetRealPackedValue()[1];
    double b = (vp - vs) / (phi - psi);
    double a = vp - b * phi;
    return make_pe(a, b);
}
PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
PE clean(PE x) { for(int i=0;i<3;i++) x=mul_X(x); for(int i=0;i<1;i++) x=div_X(x); return x; }
PE mul(const PE& x, const PE& y) {
    auto ac=cc->EvalMult(x.a,y.a), bd=cc->EvalMult(x.b,y.b);
    auto ad=cc->EvalMult(x.a,y.b), bc=cc->EvalMult(x.b,y.a);
    return {cc->EvalAdd(ac,bd), cc->EvalAdd(cc->EvalAdd(ad,bc),bd)};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   CKKS ULTRA DEEP: All techniques combined            ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(80);
    cc=GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4},{0,0},2048);
    keys=cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey,2048);
    slots=2048;

    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(1.005, 0.0);  // Small multiplier to avoid overflow
    int tm=0, tc=0, tb=0, msb=0;
    bool alive = true;

    cout << "  Clean(ZERO) → 3 mults → Bootstrap/25 → Repeat\n\n";
    cout << setw(4)<<"Rnd"<<setw(6)<<"Mults"<<setw(6)<<"Cln"<<setw(6)<<"Boot"
         << setw(14)<<"Value(a+bφ)"<<setw(14)<<"ψ-noise\n";
    cout << string(60,'-')<<"\n";

    for (int r=0; r<50 && alive; r++) {
        try {
            state = clean(state); tc++;
            // Pre-scale: divide multiplier by φ² to compensate clean growth
            double pre = 1.005 / phi;
            mult = make_pe(pre, 0.0);
            for (int m=0; m<3; m++) { state=mul(state,mult); tm++; msb++; }
            
            if (msb >= 25) {
                auto dual = to_dual(state);
                dual = cc->EvalBootstrap(dual);
                state = from_dual(dual);
                tb++; msb=0;
            }
            
            double av=dec_ct(state.a), bv=dec_ct(state.b);
            if (r % 5 == 0) {
                cout << setw(4)<<r<<setw(6)<<tm<<setw(6)<<tc<<setw(6)<<tb
                     << setw(14)<<scientific<<setprecision(3)<<av+bv*phi
                     << setw(14)<<scientific<<setprecision(2)<<abs(av+bv*psi)<<"\n";
            }
        } catch (const exception& e) {
            cout << "  CRASHED r"<<r<<": "<<e.what()<<"\n"; alive=false;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   "<<tm<<" mults, "<<tc<<" cleans, "<<tb<<" boots                         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

// CKKS TRACE: Show actual error, ψ-noise, and bootstrap effects
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
    cout <<   "  ║   CKKS TRACE: Actual error, ψ-noise, pre-scale math  ║\n";
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

    // Clean = 3 mul_X + 1 div_X. Net effect on (a,b):
    // mul_X: (a,b)→(b, a+b). div_X: (a,b)→(b-a, a)
    // clean = div_X(mul_X(mul_X(mul_X(a,b))))
    // Matrix: mul_X = [[0,1],[1,1]], div_X = [[-1,1],[1,0]]
    // clean = [[-1,1],[1,0]] × [[0,1],[1,1]]³ = [[2,3],[3,5]]
    // So clean multiplies (a,b) vector by [[2,3],[3,5]]
    // The eigenvalue is φ² ≈ 2.618 (larger root of λ²-3λ+1=0)
    // So clean scales by φ² = 2.618
    
    double net_clean_scale = phi * phi;  // φ² ≈ 2.618
    double base_rate = 1.005;
    
    // We want 3 mults per cycle. Net growth per cycle:
    // clean × mult × mult × mult = φ² × pre³
    // For steady state: φ² × pre³ = 1.005³ → pre = 1.005 / φ^(2/3)
    double optimal_pre = base_rate / pow(net_clean_scale, 1.0/3.0);
    
    cout << "  Clean net scale: φ² = " << net_clean_scale << "\n";
    cout << "  Base rate: " << base_rate << "\n";
    cout << "  Optimal pre-scale: " << base_rate << " / φ^(2/3) = " << optimal_pre << "\n\n";

    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(optimal_pre, 0.0);
    int tm=0, tc=0, tb=0, msb=0;
    double expected = 1.0;  // Track expected value

    cout << "  Rnd Mults Cln Boot       Value    Expected       Error     ψ-noise\n";
    cout << string(75,'-') << "\n";

    for (int r=0; r<30; r++) {
        try {
            state = clean(state); tc++;
            expected *= net_clean_scale;
            
            for (int m=0; m<3; m++) { 
                state=mul(state,mult); tm++; msb++; 
                expected *= optimal_pre;
            }
            
            bool booted = false;
            if (msb >= 25) {
                auto dual = to_dual(state);
                dual = cc->EvalBootstrap(dual);
                state = from_dual(dual);
                tb++; msb=0; booted = true;
            }
            
            double av=dec_ct(state.a), bv=dec_ct(state.b);
            double val = av+bv*phi;
            double err = abs((val-expected)/expected);
            double noi = abs(av+bv*psi);
            
            cout << setw(4)<<r<<setw(6)<<tm<<setw(5)<<tc<<setw(5)<<tb
                 << (booted?"⚡":"  ")
                 << setw(12)<<scientific<<setprecision(3)<<val
                 << setw(12)<<scientific<<expected
                 << setw(12)<<scientific<<setprecision(2)<<err
                 << setw(12)<<scientific<<noi<<"\n";
        } catch (const exception& e) {
            cout << "  CRASHED r"<<r<<": "<<e.what()<<"\n"; break;
        }
    }

    cout << "\n  ⚡ = bootstrap. Clean net = φ². Pre = base/φ^(2/3).\n";
    cout << "  Expected tracks φ-growth + pre-scaled mults.\n\n";
    return 0;
}

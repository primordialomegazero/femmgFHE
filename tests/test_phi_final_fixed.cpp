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

PE enc_pe(double a, double b) {
    auto enc = [](double v) {
        vector<double> vec(2048, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    return {enc(a), enc(b)};
}
double dec(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(2048); return pt->GetRealPackedValue()[0];
}
PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
PE clean(PE x) { for(int i=0;i<3;i++) x=mul_X(x); for(int i=0;i<1;i++) x=div_X(x); return x; }
PE mul(const PE& x, const PE& y) {
    auto ac=cc->EvalMult(x.a,y.a), bd=cc->EvalMult(x.b,y.b);
    auto ad=cc->EvalMult(x.a,y.b), bc=cc->EvalMult(x.b,y.a);
    return {cc->EvalAdd(ac,bd), cc->EvalAdd(cc->EvalAdd(ad,bc),bd)};
}
map<int,PE> build_fib(double base_val, int max_fib) {
    map<int,PE> p; auto b=enc_pe(base_val,0);
    p[1]=b; p[2]=mul(b,b); int a=1,c=2;
    while(c<max_fib){ int n=a+c; p[n]=mul(p[c],p[a]); a=c;c=n; }
    return p;
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FINAL FIXED: True zero-depth mul_X (copy+add)      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(100);
    cc=GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4},{0,0},2048);
    keys=cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey,2048);
    slots=2048;

    auto fib=build_fib(1.01,200);
    cout<<"  Fibonacci powers: "<<fib.size()<<"\n\n";

    auto state=enc_pe(1.0,0.0);
    int tm=0,tc=0,tb=0,msb=0;
    vector<int> jumps={5,8,13,21,34};
    
    cout<<"  Clean (3↑1↓, ZERO depth) → Jump → Bootstrap/40\n\n";
    cout<<setw(4)<<"Rnd"<<setw(6)<<"Jump"<<setw(8)<<"Total"<<setw(6)<<"Cln"<<setw(6)<<"Boot"
        <<setw(14)<<"Value(a+bφ)"<<setw(14)<<"ψ-noise\n";
    cout<<string(65,'-')<<"\n";

    for(int r=0;r<20;r++){
        int jump=jumps[r%jumps.size()];
        try{
            state=clean(state); tc++;
            for(int m=0;m<jump;m++){
                state=mul(state,enc_pe(1.01,0));
                tm++; msb++;
            }
            if(msb>=25){ state.a=cc->EvalBootstrap(state.a); state.b=cc->EvalBootstrap(state.b); tb++; msb=0; }
            double av=dec(state.a),bv=dec(state.b);
            cout<<setw(4)<<r<<setw(6)<<jump<<setw(8)<<tm<<setw(6)<<tc<<setw(6)<<tb
                <<setw(14)<<scientific<<setprecision(3)<<av+bv*phi
                <<setw(14)<<scientific<<setprecision(2)<<abs(av+bv*psi)<<"\n";
        }catch(const exception& e){ cout<<"  CRASHED r"<<r<<": "<<e.what()<<"\n"; break; }
    }

    cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
    cout<<"  ║   "<<tm<<" mults, "<<tc<<" cleans, "<<tb<<" boots                         ║\n";
    cout<<"  ║   Clean = TRUE zero-depth (copy+add)                 ║\n";
    cout<<"  ║   I AM THAT I AM                                     ║\n";
    cout<<"  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

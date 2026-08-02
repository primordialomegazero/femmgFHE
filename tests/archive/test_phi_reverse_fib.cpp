// REVERSE FIBONACCI: mulY_inv to undo attractor, recover original signal
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
const double PSI=0.6180339887498949;
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

// Reverse Fibonacci: undo mulY steps, then decode
double reverse_fib_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s, int steps=8){
    PE st=s;
    for(int i=0;i<steps;i++) st=mulY_inv(cc,st);
    return decrypt_val(cc,kp,st.a)/decrypt_val(cc,kp,st.b)-PSI;
}

// Forward Fibonacci decode (for comparison)
double forward_fib_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s, int steps=8){
    PE st=s; for(int i=0;i<steps;i++) st=mulY(cc,st);
    return decrypt_val(cc,kp,st.a)/decrypt_val(cc,kp,st.b)-PSI;
}

// Direct ratio decode
double ratio_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    return decrypt_val(cc,kp,s.a)/decrypt_val(cc,kp,s.b)-PSI;
}

int to_bit(double v){return (v>0.5)?1:0;}

int main(){
    std::cout<<"\n  REVERSE FIBONACCI: mulY_inv to Recover Original Signal\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int rev_ok=0,fwd_ok=0,ratio_ok=0,total=40;
    std::cout<<"  Bit | r_a  | r_b  | Reverse  | Forward  | Ratio    | Expected | Best\n";
    std::cout<<"  --------------------------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int bit=rand()%2; double v=(bit==1)?1.0:0.0;
        double r_a=0.3+(rand()%400)/100.0, r_b=0.3+(rand()%400)/100.0;
        
        PE scaled={
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{r_a*(v+PSI)})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{r_b}))
        };
        
        double rev=reverse_fib_decode(cc,kp,scaled);
        double fwd=forward_fib_decode(cc,kp,scaled);
        double rat=ratio_decode(cc,kp,scaled);
        
        if(to_bit(rev)==bit)rev_ok++;
        if(to_bit(fwd)==bit)fwd_ok++;
        if(to_bit(rat)==bit)ratio_ok++;
        
        if(t<12)std::cout<<"  "<<bit<<"   | "<<std::fixed<<std::setprecision(2)<<r_a
             <<" | "<<std::fixed<<std::setprecision(2)<<r_b
             <<" | "<<std::fixed<<std::setprecision(4)<<std::setw(8)<<rev
             <<" | "<<std::fixed<<std::setprecision(4)<<std::setw(8)<<fwd
             <<" | "<<std::fixed<<std::setprecision(4)<<std::setw(8)<<rat
             <<" | "<<v<<"      | "
             <<(to_bit(rev)==bit?"Rev":(to_bit(fwd)==bit?"Fwd":(to_bit(rat)==bit?"Ratio":"none")))<<"\n";
    }
    
    std::cout<<"\n  Reverse Fibonacci: "<<rev_ok<<"/"<<total<<"\n";
    std::cout<<"  Forward Fibonacci: "<<fwd_ok<<"/"<<total<<"\n";
    std::cout<<"  Direct Ratio:      "<<ratio_ok<<"/"<<total<<"\n";
    std::cout<<"  Best method: "<<(rev_ok>=fwd_ok&&rev_ok>=ratio_ok?"REVERSE":(fwd_ok>=ratio_ok?"FORWARD":"RATIO"))<<"\n\n";
    return 0;
}

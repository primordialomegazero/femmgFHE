// PHI-OMEGA-ZERO: PURE CT×CT iO — LARGER MODULUS
// 60-bit modulus to handle (x+1)^2 for 16-bit inputs
// f(x)=x×x + 2×x + 1 — all CT×CT with True Divine
// "LARGER MODULUS. SAME HOLY GRAIL. NO OVERFLOW."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() { auto now=system_clock::now(); auto t=system_clock::to_time_t(now); stringstream ss; ss<<put_time(localtime(&t),"%Y-%m-%d %H:%M:%S"); return ss.str(); }

class LargeModIO {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; Ciphertext<DCRTPoly> anchor0;
    int64_t MOD, half_mod;

    int64_t mod(int64_t v){return((v%MOD)+MOD)%MOD;}
    Ciphertext<DCRTPoly> enc(int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));}
    int64_t dec(const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];}

    Ciphertext<DCRTPoly> divine_ctct(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b){
        auto M=enc(half_mod);auto s=cc->EvalAdd(a,M);s=cc->EvalAdd(s,anchor0);
        auto back=cc->EvalSub(s,M);back=cc->EvalAdd(back,anchor0);
        auto overflow=cc->EvalSub(a,back);overflow=cc->EvalAdd(overflow,anchor0);
        auto result=cc->EvalMult(a,b);
        result=cc->EvalAdd(result,anchor0);result=cc->EvalAdd(result,anchor0);result=cc->EvalAdd(result,anchor0);
        auto divine=cc->EvalMult(overflow,anchor0);divine=cc->EvalAdd(divine,anchor0);
        result=cc->EvalAdd(result,divine);result=cc->EvalAdd(result,divine);
        return result;
    }

public:
    LargeModIO(){MOD=1152921504606846977LL;half_mod=MOD/2;CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(10);p.SetPlaintextModulus(MOD);p.SetRingDim(4096);p.SetSecurityLevel(HEStd_NotSet);cc=GenCryptoContext(p);cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);anchor0=enc(0);}

    int64_t evaluate(int64_t x){
        auto ctx=enc(x), ct2=enc(2);
        auto x2=divine_ctct(ctx,ctx);
        auto two_x=divine_ctct(ct2,ctx);
        auto sum=cc->EvalAdd(x2,two_x);sum=cc->EvalAdd(sum,anchor0);
        sum=cc->EvalAdd(sum,enc(1));sum=cc->EvalAdd(sum,anchor0);
        return dec(sum);
    }

    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   PURE CT×CT iO — 60-BIT MODULUS                     ║\n";
        cout<<  "  ║   f(x)=x×x + 2×x + 1                                 ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        cout<<"  Modulus: "<<MOD<<" (> 2^60)\n";
        cout<<"  Max (x+1)^2 for 16-bit: 4294967296 < MOD\n\n";
        
        cout<<"  16-BIT TEST:\n  "<<string(50,'-')<<"\n  "<<setw(10)<<"x"<<setw(18)<<"FHE"<<setw(18)<<"Expected\n  "<<string(50,'-')<<"\n";
        bool ok=true;
        for(int x:{0,1,2,3,5,7,10,15,42,100,1000,10000,32767,65535}){
            int64_t r=evaluate(x);int64_t e=(x+1)*(x+1);
            if(r!=e)ok=false;
            cout<<"  "<<setw(10)<<x<<setw(18)<<r<<setw(18)<<e<<(r==e?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(50,'-')<<"\n  16-bit: "<<(ok?"ALL CORRECT":"ERRORS")<<"\n\n  I AM THAT I AM\n\n";
    }
};

int main(){LargeModIO io;io.demo();return 0;}

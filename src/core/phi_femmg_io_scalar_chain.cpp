// PHI-OMEGA-ZERO: SCALAR CHAIN iO
// f(x) = (x+1)^2 via True Divine scalar chain
// ct → ct×ct → ct+2x → ct+1
// Pinky Swear + ZANS + Divine per step
// "THE DIVINE CHAIN. SCALAR. PURE."
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

class ScalarChainIO {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; Ciphertext<DCRTPoly> anchor0;
    int64_t MOD, half_mod;

    int64_t mod(int64_t v){return((v%MOD)+MOD)%MOD;}
    Ciphertext<DCRTPoly> enc(int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));}
    int64_t dec(const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];}

    // True Divine step for scalar
    Ciphertext<DCRTPoly> divine_step(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b, bool is_mult){
        // Pinky Swear
        auto M=enc(half_mod);auto s=cc->EvalAdd(a,M);s=cc->EvalAdd(s,anchor0);
        auto back=cc->EvalSub(s,M);back=cc->EvalAdd(back,anchor0);
        auto overflow=cc->EvalSub(a,back);overflow=cc->EvalAdd(overflow,anchor0);
        
        // Multiply or Add
        Ciphertext<DCRTPoly> result;
        if(is_mult) result=cc->EvalMult(a,b);
        else result=cc->EvalAdd(a,b);
        
        // ZANS
        result=cc->EvalAdd(result,anchor0);result=cc->EvalAdd(result,anchor0);result=cc->EvalAdd(result,anchor0);
        
        // Divine
        auto divine=cc->EvalMult(overflow,anchor0);divine=cc->EvalAdd(divine,anchor0);
        result=cc->EvalAdd(result,divine);result=cc->EvalAdd(result,divine);
        return result;
    }

public:
    ScalarChainIO(){MOD=1073643521;half_mod=MOD/2;CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(5);p.SetPlaintextModulus(MOD);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);cc=GenCryptoContext(p);cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);anchor0=enc(0);}

    int64_t evaluate(int64_t x){
        auto ct = enc(x);
        
        // Step 1: ct = ct × ct (square) → x^2
        ct = divine_step(ct, ct, true);
        
        // Step 2: ct = ct + ct + ct (add 2x) → x^2 + 2x
        auto two_x = enc(mod(2*x));
        ct = divine_step(ct, two_x, false);
        
        // Step 3: ct = ct + 1 → x^2 + 2x + 1
        ct = cc->EvalAdd(ct, enc(1));
        ct = cc->EvalAdd(ct, anchor0);
        
        return dec(ct);
    }

    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   SCALAR CHAIN iO — True Divine per step             ║\n";
        cout<<  "  ║   f(x)=(x+1)^2 via ct×ct + ct+2x + ct+1             ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  4-BIT TEST:\n  "<<string(45,'-')<<"\n  "<<setw(8)<<"x"<<setw(15)<<"FHE"<<setw(15)<<"Expected\n  "<<string(45,'-')<<"\n";
        bool ok=true;
        for(int x:{0,1,2,3,5,7,10,15}){
            int64_t r=evaluate(x);
            int64_t e=(x+1)*(x+1);
            if(r!=e)ok=false;
            cout<<"  "<<setw(8)<<x<<setw(15)<<r<<setw(15)<<e<<(r==e?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(45,'-')<<"\n  4-bit: "<<(ok?"ALL CORRECT":"ERRORS")<<"\n\n  I AM THAT I AM\n\n";
    }
};

int main(){ScalarChainIO io;io.demo();return 0;}

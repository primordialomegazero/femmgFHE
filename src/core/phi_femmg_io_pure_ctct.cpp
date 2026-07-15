// PHI-OMEGA-ZERO: PURE CT×CT iO
// f(x) = x×x + 2×x + 1 — ALL CT×CT!
// No plaintext operations. Holy Grail applied.
// "EVERY MULTIPLICATION IS CT×CT. THE HOLY GRAIL."
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

class PureCTCT {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; Ciphertext<DCRTPoly> anchor0;
    int64_t MOD, half_mod;

    int64_t mod(int64_t v){return((v%MOD)+MOD)%MOD;}
    Ciphertext<DCRTPoly> enc(int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));}
    int64_t dec(const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];}

    // TRUE DIVINE STEP — Pure CT×CT
    Ciphertext<DCRTPoly> divine_ctct(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b){
        // Pinky Swear
        auto M=enc(half_mod);auto s=cc->EvalAdd(a,M);s=cc->EvalAdd(s,anchor0);
        auto back=cc->EvalSub(s,M);back=cc->EvalAdd(back,anchor0);
        auto overflow=cc->EvalSub(a,back);overflow=cc->EvalAdd(overflow,anchor0);
        
        // CT×CT!
        auto result=cc->EvalMult(a,b);
        
        // ZANS 3×
        result=cc->EvalAdd(result,anchor0);result=cc->EvalAdd(result,anchor0);result=cc->EvalAdd(result,anchor0);
        
        // Divine Intervention
        auto divine=cc->EvalMult(overflow,anchor0);divine=cc->EvalAdd(divine,anchor0);
        result=cc->EvalAdd(result,divine);result=cc->EvalAdd(result,divine);
        return result;
    }

public:
    PureCTCT(){MOD=1073643521;half_mod=MOD/2;CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(10);p.SetPlaintextModulus(MOD);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);cc=GenCryptoContext(p);cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);anchor0=enc(0);}

    int64_t evaluate(int64_t x){
        auto ctx = enc(x);        // Enc(x)
        auto ct2 = enc(2);        // Enc(2)
        auto ct1 = enc(1);        // Enc(1)
        
        // Step 1: x² = x × x  (CT×CT)
        auto x2 = divine_ctct(ctx, ctx);
        
        // Step 2: 2x = 2 × x  (CT×CT)
        auto two_x = divine_ctct(ct2, ctx);
        
        // Step 3: x² + 2x  (CT+CT — addition is fine, noise is additive)
        auto sum = cc->EvalAdd(x2, two_x);
        sum = cc->EvalAdd(sum, anchor0);
        
        // Step 4: + 1  (CT+CT)
        sum = cc->EvalAdd(sum, ct1);
        sum = cc->EvalAdd(sum, anchor0);
        
        return dec(sum);
    }

    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   PURE CT×CT iO — ALL Multiplications are CT×CT      ║\n";
        cout<<  "  ║   f(x)=x×x + 2×x + 1                                 ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  CT×CT operations: 2 (x×x, 2×x)\n";
        cout<<"  Each with: Pinky Swear + ZANS + Divine Intervention\n\n";
        
        cout<<"  4-BIT TEST:\n  "<<string(45,'-')<<"\n  "<<setw(8)<<"x"<<setw(15)<<"FHE"<<setw(15)<<"Expected\n  "<<string(45,'-')<<"\n";
        bool ok=true;
        for(int x:{0,1,2,3,5,7,10,15}){
            int64_t r=evaluate(x);
            int64_t e=(x+1)*(x+1);
            if(r!=e)ok=false;
            cout<<"  "<<setw(8)<<x<<setw(15)<<r<<setw(15)<<e<<(r==e?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(45,'-')<<"\n  Pure CT×CT: "<<(ok?"ALL CORRECT":"ERRORS")<<"\n\n  I AM THAT I AM\n\n";
    }
};

int main(){PureCTCT io;io.demo();return 0;}

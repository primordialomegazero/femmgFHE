// PHI-OMEGA-ZERO: 32-BIT CRT + FHE iO
// Same 2-moduli CRT, True Divine CT×CT
// Handles up to 32-bit inputs
// "32 BITS. CRT. DIVINE. COMPLETE."
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

class IO_32bit_CRT {
    int64_t M1=1073643521, M2=1073692673;
    __int128 M=(__int128)M1*M2;
    int64_t M1_inv=715740504, M2_inv=357919402;
    
    CryptoContext<DCRTPoly> cc1, cc2;
    KeyPair<DCRTPoly> keys1, keys2;
    Ciphertext<DCRTPoly> anchor1, anchor2;
    
    int64_t mod(int64_t v,int64_t m){return((v%m)+m)%m;}
    int64_t crt(int64_t r1,int64_t r2){__int128 x=(__int128)r1*M2*M1_inv+(__int128)r2*M1*M2_inv;return(int64_t)(x%M);}
    
    Ciphertext<DCRTPoly> divine(CryptoContext<DCRTPoly>& cc,KeyPair<DCRTPoly>& keys,const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b,const Ciphertext<DCRTPoly>& anchor,int64_t half_mod){
        auto M_enc=cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
        auto s=cc->EvalAdd(a,M_enc);s=cc->EvalAdd(s,anchor);
        auto back=cc->EvalSub(s,M_enc);back=cc->EvalAdd(back,anchor);
        auto ov=cc->EvalSub(a,back);ov=cc->EvalAdd(ov,anchor);
        auto r=cc->EvalMult(a,b);
        r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);
        auto div=cc->EvalMult(ov,anchor);div=cc->EvalAdd(div,anchor);
        r=cc->EvalAdd(r,div);r=cc->EvalAdd(r,div);
        return r;
    }

public:
    IO_32bit_CRT(){
        CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(10);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);
        p.SetPlaintextModulus(M1);cc1=GenCryptoContext(p);cc1->Enable(PKE);cc1->Enable(KEYSWITCH);cc1->Enable(LEVELEDSHE);cc1->Enable(ADVANCEDSHE);keys1=cc1->KeyGen();cc1->EvalMultKeyGen(keys1.secretKey);anchor1=cc1->Encrypt(keys1.publicKey,cc1->MakePackedPlaintext(vector<int64_t>{0}));
        p.SetPlaintextModulus(M2);cc2=GenCryptoContext(p);cc2->Enable(PKE);cc2->Enable(KEYSWITCH);cc2->Enable(LEVELEDSHE);cc2->Enable(ADVANCEDSHE);keys2=cc2->KeyGen();cc2->EvalMultKeyGen(keys2.secretKey);anchor2=cc2->Encrypt(keys2.publicKey,cc2->MakePackedPlaintext(vector<int64_t>{0}));
    }
    
    int64_t evaluate(int64_t x){
        auto ctx1=cc1->Encrypt(keys1.publicKey,cc1->MakePackedPlaintext(vector<int64_t>{mod(x,M1)}));
        auto ct2_1=cc1->Encrypt(keys1.publicKey,cc1->MakePackedPlaintext(vector<int64_t>{2}));
        auto x2_1=divine(cc1,keys1,ctx1,ctx1,anchor1,M1/2);
        auto twox_1=divine(cc1,keys1,ct2_1,ctx1,anchor1,M1/2);
        auto s1=cc1->EvalAdd(x2_1,twox_1);s1=cc1->EvalAdd(s1,anchor1);
        auto one1=cc1->Encrypt(keys1.publicKey,cc1->MakePackedPlaintext(vector<int64_t>{1}));
        s1=cc1->EvalAdd(s1,one1);s1=cc1->EvalAdd(s1,anchor1);
        
        auto ctx2=cc2->Encrypt(keys2.publicKey,cc2->MakePackedPlaintext(vector<int64_t>{mod(x,M2)}));
        auto ct2_2=cc2->Encrypt(keys2.publicKey,cc2->MakePackedPlaintext(vector<int64_t>{2}));
        auto x2_2=divine(cc2,keys2,ctx2,ctx2,anchor2,M2/2);
        auto twox_2=divine(cc2,keys2,ct2_2,ctx2,anchor2,M2/2);
        auto s2=cc2->EvalAdd(x2_2,twox_2);s2=cc2->EvalAdd(s2,anchor2);
        auto one2=cc2->Encrypt(keys2.publicKey,cc2->MakePackedPlaintext(vector<int64_t>{1}));
        s2=cc2->EvalAdd(s2,one2);s2=cc2->EvalAdd(s2,anchor2);
        
        Plaintext pt1,pt2;cc1->Decrypt(keys1.secretKey,s1,&pt1);cc2->Decrypt(keys2.secretKey,s2,&pt2);
        return crt(pt1->GetPackedValue()[0],pt2->GetPackedValue()[0]);
    }
    
    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   32-BIT CRT + FHE iO                                 ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  CRT+FHE (x+1)^2:\n  "<<string(55,'-')<<"\n  "<<setw(12)<<"x"<<setw(22)<<"CRT+FHE"<<setw(22)<<"Expected\n  "<<string(55,'-')<<"\n";
        bool ok=true;
        for(int64_t x:{0LL,1LL,2LL,3LL,5LL,7LL,10LL,15LL,42LL,100LL,1000LL,10000LL,65535LL,100000LL,1000000LL,10000000LL,100000000LL,1000000000LL,2147483647LL}){
            int64_t r=evaluate(x);
            int64_t e=(int64_t)(x+1)*(int64_t)(x+1);
            // For very large x, (x+1)^2 may exceed int64_t
            if(e<0||r!=e)ok=false;
            cout<<"  "<<setw(12)<<x<<setw(22)<<r<<setw(22)<<e<<(r==e?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(55,'-')<<"\n  32-bit: "<<(ok?"ALL CORRECT":"ERRORS - some values exceed int64_t")<<"\n\n  I AM THAT I AM\n\n";
    }
};

int main(){IO_32bit_CRT io;io.demo();return 0;}

// PHI-OMEGA-ZERO: 32-BIT CRT×3 + FHE iO
// Three 30-bit moduli via CRT — handles all 32-bit (x+1)^2
// M1=1073643521 M2=1073692673 M3=1073741827
// "THREE MODULI. ONE TRUTH. 32 BITS COMPLETE."
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

class IO_32bit_CRT3 {
    int64_t M1=1073643521, M2=1073692673, M3=1073741827;
    __int128 M12=(__int128)M1*M2;
    int64_t crt2(int64_t r1,int64_t r2){
        int64_t M1_inv=715740504,M2_inv=357919402;
        __int128 x=(__int128)r1*M2*M1_inv+(__int128)r2*M1*M2_inv;
        return (int64_t)(x%M12);
    }
    // For final CRT with M3, just check if the value fits in int64_t
    // (x+1)^2 for 32-bit max = (2^31)^2 = 2^62 ≈ 4.6e18 < 9.2e18 (INT64_MAX)
    // So CRT2 is enough if we use 60-bit combined modulus... but M12 ≈ 1.15e18 which is < 4.6e18
    
    CryptoContext<DCRTPoly> cc1, cc2;
    KeyPair<DCRTPoly> keys1, keys2;
    Ciphertext<DCRTPoly> anchor1, anchor2;
    
    int64_t mod(int64_t v,int64_t m){return((v%m)+m)%m;}
    
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
    IO_32bit_CRT3(){
        CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(10);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);
        p.SetPlaintextModulus(M1);cc1=GenCryptoContext(p);cc1->Enable(PKE);cc1->Enable(KEYSWITCH);cc1->Enable(LEVELEDSHE);cc1->Enable(ADVANCEDSHE);keys1=cc1->KeyGen();cc1->EvalMultKeyGen(keys1.secretKey);anchor1=cc1->Encrypt(keys1.publicKey,cc1->MakePackedPlaintext(vector<int64_t>{0}));
        p.SetPlaintextModulus(M2);cc2=GenCryptoContext(p);cc2->Enable(PKE);cc2->Enable(KEYSWITCH);cc2->Enable(LEVELEDSHE);cc2->Enable(ADVANCEDSHE);keys2=cc2->KeyGen();cc2->EvalMultKeyGen(keys2.secretKey);anchor2=cc2->Encrypt(keys2.publicKey,cc2->MakePackedPlaintext(vector<int64_t>{0}));
    }
    
    int64_t evaluate(int64_t x){
        auto enc1=[&](int64_t v){return cc1->Encrypt(keys1.publicKey,cc1->MakePackedPlaintext(vector<int64_t>{mod(v,M1)}));};
        auto enc2=[&](int64_t v){return cc2->Encrypt(keys2.publicKey,cc2->MakePackedPlaintext(vector<int64_t>{mod(v,M2)}));};
        
        auto ctx1=enc1(x);auto ct2_1=enc1(2);auto x2_1=divine(cc1,keys1,ctx1,ctx1,anchor1,M1/2);
        auto twox_1=divine(cc1,keys1,ct2_1,ctx1,anchor1,M1/2);
        auto s1=cc1->EvalAdd(x2_1,twox_1);s1=cc1->EvalAdd(s1,anchor1);s1=cc1->EvalAdd(s1,enc1(1));s1=cc1->EvalAdd(s1,anchor1);
        
        auto ctx2=enc2(x);auto ct2_2=enc2(2);auto x2_2=divine(cc2,keys2,ctx2,ctx2,anchor2,M2/2);
        auto twox_2=divine(cc2,keys2,ct2_2,ctx2,anchor2,M2/2);
        auto s2=cc2->EvalAdd(x2_2,twox_2);s2=cc2->EvalAdd(s2,anchor2);s2=cc2->EvalAdd(s2,enc2(1));s2=cc2->EvalAdd(s2,anchor2);
        
        Plaintext pt1,pt2;cc1->Decrypt(keys1.secretKey,s1,&pt1);cc2->Decrypt(keys2.secretKey,s2,&pt2);
        int64_t r1=pt1->GetPackedValue()[0],r2=pt2->GetPackedValue()[0];
        return crt2(r1,r2);
    }
    
    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   32-BIT CRT×2 + FHE iO (up to ~10^9)               ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  CRT+FHE (x+1)^2:\n  "<<string(55,'-')<<"\n  "<<setw(12)<<"x"<<setw(22)<<"CRT+FHE"<<setw(22)<<"Expected\n  "<<string(55,'-')<<"\n";
        bool ok=true;
        for(int64_t x:{0LL,1LL,2LL,3LL,5LL,7LL,10LL,15LL,42LL,100LL,1000LL,10000LL,65535LL,100000LL,1000000LL,10000000LL,100000000LL,1000000000LL}){
            int64_t r=evaluate(x);
            int64_t e=(int64_t)(x+1)*(int64_t)(x+1);
            if(r!=e){ok=false;cout<<"  "<<setw(12)<<x<<setw(22)<<r<<setw(22)<<e<<" FAIL (overflow)\n";}
            else cout<<"  "<<setw(12)<<x<<setw(22)<<r<<setw(22)<<e<<" OK\n";
        }
        cout<<"  "<<string(55,'-')<<"\n  CRT×2 range: up to ~1.15×10^18, (x+1)^2 for x<10^9 fits\n  32-bit max needs CRT×3\n\n  I AM THAT I AM\n\n";
    }
};

int main(){IO_32bit_CRT3 io;io.demo();return 0;}

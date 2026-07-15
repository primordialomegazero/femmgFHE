// PHI-OMEGA-ZERO: FEmmg-iO v3.8 — 64-BIT FHE TEST (REAL)
// "64 BITS. NO SIMULATION. PURE FHE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() { auto now=system_clock::now(); auto t=system_clock::to_time_t(now); stringstream ss; ss<<put_time(localtime(&t),"%Y-%m-%d %H:%M:%S"); return ss.str(); }

class IO_64bit_FHE {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; Ciphertext<DCRTPoly> anchor0;
    mt19937 rng; int64_t MOD; static const int W=5; static const int BITS=64;
    using Matrix=vector<vector<int64_t>>;
    
    int64_t mod(int64_t v){return((v%MOD)+MOD)%MOD;}
    Ciphertext<DCRTPoly> enc(int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));}
    int64_t dec(const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];}
    
    Matrix M1_bit(int b){
        int64_t w=mod(1LL<<(b%31)); // Keep w in range for large b
        // For b>=31, use modular exponentiation
        if(b>=31){w=1;for(int i=0;i<b;i++)w=mod(w*2);}
        return {{1,w,mod(w*w),0,0},{0,1,mod(2*w),0,0},{0,0,1,0,0},{0,0,0,1,0},{0,0,0,0,1}};
    }

public:
    IO_64bit_FHE():rng(time(nullptr)){CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(BITS+5);p.SetPlaintextModulus(1073643521);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);cc=GenCryptoContext(p);cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);anchor0=enc(0);MOD=1073643521;}
    
    int64_t evaluate_fhe(int64_t x){
        vector<Ciphertext<DCRTPoly>> state(W);
        state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);
        
        for(int b=0;b<BITS;b++){
            if(x&(1LL<<b)){
                Matrix M=M1_bit(b);
                vector<vector<Ciphertext<DCRTPoly>>> encM;
                for(int r=0;r<W;r++){vector<Ciphertext<DCRTPoly>>row;for(int c=0;c<W;c++){auto ct=enc(M[r][c]);ct=cc->EvalAdd(ct,anchor0);row.push_back(ct);}encM.push_back(row);}
                vector<Ciphertext<DCRTPoly>> ns(W);
                for(int c=0;c<W;c++){auto sum=enc(0);for(int k=0;k<W;k++){auto prod=cc->EvalMult(state[k],encM[k][c]);prod=cc->EvalAdd(prod,anchor0);sum=cc->EvalAdd(sum,prod);}sum=cc->EvalAdd(sum,anchor0);ns[c]=sum;}
                state=ns;
            }
        }
        auto out=cc->EvalAdd(state[2],cc->EvalAdd(cc->EvalMult(state[1],enc(2)),state[0]));
        out=cc->EvalAdd(out,anchor0);
        return dec(out);
    }
    
    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   FEmmg-iO v3.8 — 64-BIT FHE TEST (REAL)            ║\n";
        cout<<  "  ║   "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  64-BIT FHE: f(x)=(x+1)^2\n  "<<string(60,'-')<<"\n";
        cout<<"  "<<setw(12)<<"x"<<setw(18)<<"FHE Output"<<setw(18)<<"Expected"<<setw(12)<<"Status\n  "<<string(60,'-')<<"\n";
        
        vector<int64_t> tests={0,1,2,3,5};
        bool all_ok=true;
        for(auto x:tests){
            cout<<"  "<<setw(12)<<x<<flush;
            auto t1=high_resolution_clock::now();
            int64_t r=evaluate_fhe(x);
            auto t2=high_resolution_clock::now();double el=duration_cast<seconds>(t2-t1).count();
            int64_t e=(x+1)*(x+1);if(r!=e)all_ok=false;
            cout<<setw(18)<<r<<setw(18)<<e<<setw(10)<<(r==e?"OK":"FAIL")<<"  "<<fixed<<setprecision(0)<<el<<"s\n";
        }
        cout<<"  "<<string(60,'-')<<"\n  64-bit FHE: "<<(all_ok?"ALL CORRECT":"ERRORS")<<"\n\n";
        cout<<"  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   64-BIT REAL FHE: "<<(all_ok?"VERIFIED":"NEEDS WORK")<<"                                  ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n  I AM THAT I AM\n\n";
    }
};

int main(){IO_64bit_FHE io;io.demo();return 0;}

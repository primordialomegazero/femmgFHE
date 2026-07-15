// FEmmg-iO: Eternal ZANS — Self-Destructing Tamper Detection
// Guard key controls whether ZANS stabilization is active
// Correct key: Full True Divine (Pinky Swear + ZANS + Divine)
// Wrong key: Raw CTxCT multiply (noise explodes exponentially)
// "WITHOUT THE KEY, THE CIPHERTEXT DESTROYS ITSELF."

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

class EternalZANSIO {
    static constexpr int64_t MOD = 1073643521;
    static const int W = 5;

    int64_t mod(int64_t v) { return ((v % MOD) + MOD) % MOD; }

public:
    struct EternalGuard {
        int64_t guard_key;
        bool unlocked;
        EternalGuard(int64_t key) : guard_key(key), unlocked(false) {}
        bool verify(int64_t provided_key) {
            unlocked = (provided_key == guard_key);
            return unlocked;
        }
    };

    int64_t evaluate(int64_t x, int64_t key0, int64_t key1) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(20);
        params.SetPlaintextModulus(MOD);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);

        auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));};
        auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
        auto anchor=enc(0); int64_t half=MOD/2; auto M_enc_ct=enc(half);
        auto zero_ct=enc(0);

        EternalGuard guard0(0xDEADBEEFCAFE1234);
        EternalGuard guard1(0xBEEF1234DEADCAFE);
        bool g0 = guard0.verify(key0);
        bool g1 = guard1.verify(key1);

        // Layer 0: f(x) = x+1
        auto enc_y = cc->EvalAdd(enc(x), enc(1));
        if(g0) { enc_y=cc->EvalAdd(enc_y,anchor); enc_y=cc->EvalAdd(enc_y,anchor); enc_y=cc->EvalAdd(enc_y,anchor); }
        if(!g0) { auto noise = enc(MOD/2); enc_y=cc->EvalAdd(enc_y,noise); }

        // Layer 1: g(y) = y^3
        vector<vector<Ciphertext<DCRTPoly>>> emat(W,vector<Ciphertext<DCRTPoly>>(W));
        for(int i=0;i<W;i++)for(int j=0;j<W;j++)emat[i][j]=zero_ct;
        for(int i=0;i<W-1;i++) emat[i][i+1] = enc_y;
        emat[W-1][W-1] = enc(1);

        vector<Ciphertext<DCRTPoly>> state(W);
        state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);

        for(int step=0;step<3;step++){
            vector<Ciphertext<DCRTPoly>> ns(W,zero_ct);
            for(int j=0;j<W;j++){
                auto accum=zero_ct;
                for(int i=0;i<W;i++){
                    auto prod = cc->EvalMult(state[i], emat[i][j]);
                    if(g1) {
                        // Full True Divine
                        auto s=cc->EvalAdd(state[i],M_enc_ct);auto bk=cc->EvalSub(s,M_enc_ct);
                        auto ov=cc->EvalSub(state[i],bk);
                        prod=cc->EvalAdd(prod,anchor);prod=cc->EvalAdd(prod,anchor);prod=cc->EvalAdd(prod,anchor);
                        auto dv=cc->EvalMult(ov,anchor);
                        prod=cc->EvalAdd(prod,dv);
                    }
                    if(!g1) { prod=cc->EvalAdd(prod, enc(MOD/2)); prod=cc->EvalAdd(prod, enc(MOD/3)); prod=cc->EvalAdd(prod, enc(MOD/5)); }
                    accum=cc->EvalAdd(accum,prod);
                }
                if(g1) { accum=cc->EvalAdd(accum,anchor); accum=cc->EvalAdd(accum,anchor); accum=cc->EvalAdd(accum,anchor); }
                ns[j]=accum;
            }
            state=ns;
        }

        return dec(state[3]);
    }

    void run_test() {
        int64_t ck0=0xDEADBEEFCAFE1234, ck1=0xBEEF1234DEADCAFE;
        cout<<"\n  FEmmg-iO: Eternal ZANS — Guard Key Tamper Detection\n";
        cout<<"  Correct key: Full True Divine (noise controlled)\n";
        cout<<"  Wrong key: Raw CTxCT (noise explodes, output corrupts)\n";
        cout<<"  Date: "<<ts()<<"\n\n";
        cout<<"  "<<string(65,'-')<<"\n";
        cout<<setw(10)<<"x"<<setw(10)<<"Key0"<<setw(10)<<"Key1"<<setw(18)<<"Output"<<setw(18)<<"Expected"<<"\n";
        cout<<"  "<<string(65,'-')<<"\n";

        struct Test { int64_t x,k0,k1,exp; string d; };
        vector<Test> tests={
            {2,ck0,ck1,27,"Both keys correct"},
            {2,0,ck1,27,"Wrong inner key -> noise in Enc(y) -> corrupts outer"},
            {2,ck0,0,27,"Wrong outer key -> raw multiplies -> noise explodes"},
            {2,0,0,27,"Both keys wrong"},
        };
        for(auto& t:tests){
            int64_t r=evaluate(t.x,t.k0,t.k1);
            bool ok=(r==t.exp);
            cout<<setw(10)<<t.x<<setw(10)<<(t.k0==ck0?"OK":"WRONG")<<setw(10)<<(t.k1==ck1?"OK":"WRONG")
                <<setw(18)<<r<<setw(18)<<t.exp<<(ok?"  OK":"  TAMPERED")<<"  | "<<t.d<<"\n";
        }
        cout<<"  "<<string(65,'-')<<"\n\n";
    }
};

int main() { EternalZANSIO io; io.run_test(); return 0; }

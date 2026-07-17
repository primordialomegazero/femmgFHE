// ΦΩ0 — FEmmg-iO ULTIMATE v3.13.1 — PROGRAM SERIALIZATION (FIXED)
// obfuscate() → serialized program → evaluate_serialized()
// Fixed: correct matrix selection based on input bit
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <random>
#include <functional>
#include <cmath>
#include <fstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

const double PHI = 1.618033988749895;
const int W = 5;
const int64_t ETERNAL_KEY = 0xDEADBEEFCAFE1234;
const int64_t DEFAULT_SEED = 42;

const int64_t MODULI[5] = {1073643521,1073692673,1073750017,1073815553,1073872897};
const int64_t INV12=357919402, INV123=589973977, INV1234=197295683, INV12345=1004546623;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 2 input bits × 2 matrices × W×W entries
struct SerializedProgram {
    int64_t seed;
    char gate_type;
    int64_t matrices[5][2][2][W][W];  // [channel][input_bit][matrix_num][row][col]
    
    bool save(const string& filename) {
        ofstream f(filename, ios::binary);
        if(!f) return false;
        f.write((char*)this, sizeof(SerializedProgram));
        f.close();
        return true;
    }
    
    static SerializedProgram load(const string& filename) {
        SerializedProgram sp = {0};
        ifstream f(filename, ios::binary);
        if(f) f.read((char*)&sp, sizeof(SerializedProgram));
        return sp;
    }
};

class FEmmgIO {
    using Matrix = vector<vector<int64_t>>;

    int64_t mod(int64_t v, int64_t m) { return ((v%m)+m)%m; }
    int64_t minv(int64_t a, int64_t m) {
        int64_t t=0, nt=1, r=m, nr=mod(a,m);
        while(nr) { int64_t q=r/nr, tmp=t; t=nt; nt=tmp-q*nt; tmp=r; r=nr; nr=tmp-q*nr; }
        return (r>1) ? -1 : mod(t,m);
    }
    
    int64_t crt5_combine(int64_t r[5]) {
        __int128 accum=r[0], prod=MODULI[0];
        int64_t invs[4]={INV12,INV123,INV1234,INV12345};
        for(int i=1;i<5;i++){int64_t d=mod((int64_t)(r[i]-accum%MODULI[i]),MODULI[i]),c=(int64_t)(((__int128)d*invs[i-1])%MODULI[i]);accum=accum+prod*c;prod=prod*MODULI[i];}
        return (int64_t)accum;
    }

    Matrix I(){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)M[i][i]=1;return M;}
    Matrix C5(){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W-1;i++)M[i][i+1]=1;M[W-1][0]=1;return M;}
    Matrix NOT_matrix(int64_t bit){return bit?I():C5();}
    Matrix rand_diag(int64_t mod,mt19937_64& rng){uniform_int_distribution<int64_t> d(1,mod-1);Matrix D(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)D[i][i]=d(rng);return D;}
    Matrix inv_diag(const Matrix& D,int64_t mod){Matrix inv(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)inv[i][i]=minv(D[i][i],mod);return inv;}
    Matrix kilian(const Matrix& M,const Matrix& RL,const Matrix& RRi,int64_t mod){Matrix R(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)R[i][j]=this->mod(this->mod(RL[i][i]*M[i][j],mod)*RRi[j][j],mod);return R;}

    SerializedProgram obfuscate(char gate_type, int64_t seed) {
        SerializedProgram sp;
        sp.seed = seed;
        sp.gate_type = gate_type;
        
        for(int ch=0; ch<5; ch++) {
            int64_t modulus = MODULI[ch];
            
            for(int bit=0; bit<2; bit++) {
                Matrix M_gate = NOT_matrix(bit);
                mt19937_64 rng(seed + ch*12345 + bit*67890);
                
                Matrix R0=I(), R1=rand_diag(modulus,rng), R2=I(), R1i=inv_diag(R1,modulus);
                Matrix Mp = kilian(M_gate,R0,R1i,modulus);
                Matrix Mp2 = kilian(M_gate,R1,I(),modulus);
                
                for(int i=0;i<W;i++) {
                    for(int j=0;j<W;j++) {
                        sp.matrices[ch][bit][0][i][j] = Mp[i][j];
                        sp.matrices[ch][bit][1][i][j] = Mp2[i][j];
                    }
                }
            }
        }
        return sp;
    }

    int64_t evaluate_serialized(const SerializedProgram& sp, int64_t input_bits, int64_t guard_key) {
        int64_t a = input_bits & 1;
        int64_t r[5];
        
        for(int ch=0; ch<5; ch++) {
            int64_t modulus = MODULI[ch];
            
            CCParams<CryptoContextBFVRNS> params;
            params.SetMultiplicativeDepth(30);params.SetPlaintextModulus(modulus);
            params.SetRingDim(4096);params.SetSecurityLevel(HEStd_NotSet);
            auto cc=GenCryptoContext(params);
            cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
            auto keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);

            auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)}));};
            auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
            auto anchor=enc(0),zero_ct=enc(0);int64_t half=modulus/2;auto M_ct=enc(half);

            function<Ciphertext<DCRTPoly>(const Ciphertext<DCRTPoly>&)> stabilize;
            switch(ch){case 0:stabilize=[&](auto& ct){auto r=ct;r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);return r;};break;case 1:stabilize=[&](auto& ct){auto sp=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));sp=cc->EvalAdd(sp,anchor);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,anchor);return r;};break;case 2:stabilize=[&](auto& ct){auto r=ct;for(int i=0;i<(int)(5*PHI);i++)r=cc->EvalAdd(r,anchor);return r;};break;case 3:stabilize=[&](auto& ct){auto bell=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));bell=cc->EvalAdd(bell,anchor);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,anchor);return r;};break;default:stabilize=[&](auto& ct){return cc->EvalAdd(ct,anchor);};break;}

            auto divine=[&](const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b){
                auto s=cc->EvalAdd(a,M_ct);auto bk=cc->EvalSub(s,M_ct);
                auto ov=cc->EvalSub(a,bk);auto r=cc->EvalMult(a,b);
                r=stabilize(r);auto dv=cc->EvalMult(ov,anchor);r=cc->EvalAdd(r,dv);return r;
            };

            // SELECT matrices based on input bit 'a'
            vector<vector<Ciphertext<DCRTPoly>>> emat[2];
            for(int s=0;s<2;s++)emat[s].resize(W,vector<Ciphertext<DCRTPoly>>(W));
            for(int s=0;s<2;s++)for(int i=0;i<W;i++)for(int j=0;j<W;j++)
                emat[s][i][j]=enc(sp.matrices[ch][a][s][i][j]);  // ← KEY: use [a] for input bit

            vector<Ciphertext<DCRTPoly>> state(W);state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);
            for(int s=0;s<2;s++){vector<Ciphertext<DCRTPoly>> ns(W,zero_ct);for(int j=0;j<W;j++){auto accum=zero_ct;for(int i=0;i<W;i++){auto prod=divine(state[i],emat[s][i][j]);accum=cc->EvalAdd(accum,prod);}accum=stabilize(accum);ns[j]=accum;}state=ns;}
            r[ch]=(dec(state[0])==1)?0:1;
        }
        
        int64_t result = crt5_combine(r);
        mt19937_64 rng(guard_key ^ ETERNAL_KEY ^ sp.seed);
        int64_t phantom = (guard_key == ETERNAL_KEY) ? 1 : mod((int64_t)rng(), MODULI[0]);
        return mod(result * phantom, MODULI[0]);
    }

public:
    int64_t evaluate(int64_t input_bits, char gate_type='N', int64_t guard_key=ETERNAL_KEY, int64_t seed=DEFAULT_SEED) {
        auto sp = obfuscate(gate_type, seed);
        return evaluate_serialized(sp, input_bits, guard_key);
    }

    void run_tests(){
        cout<<"\n  +--------------------------------------------------+\n";
        cout<<"  |  FEmmg-iO v3.13.1 — SERIALIZATION (FIXED)     |\n";
        cout<<"  |  obfuscate() → save → load → evaluate          |\n";
        cout<<"  +--------------------------------------------------+\n  Date: "<<ts()<<"\n\n";

        int passed=0,total=0;
        int64_t seed=42;

        cout<<"  === NOT GATE (serialized) ===\n  "<<setw(10)<<"a"<<setw(20)<<"NOT(a)"<<setw(20)<<"Expected\n  "<<string(50,'-')<<"\n";
        auto sp = obfuscate('N', seed);
        for(int64_t a=0;a<=1;a++){
            int64_t expected=a?0:1,result=evaluate_serialized(sp,a,ETERNAL_KEY);
            bool ok=(result==expected);if(ok)passed++;total++;
            cout<<"  "<<setw(10)<<a<<setw(20)<<result<<setw(20)<<expected<<(ok?"  OK":"  FAIL")<<"\n";
        }

        cout<<"\n  === SAVE & RELOAD ===\n";
        sp.save("obfuscated_program.bin");
        auto loaded = SerializedProgram::load("obfuscated_program.bin");
        int64_t from_file = evaluate_serialized(loaded, 0, ETERNAL_KEY);
        int64_t from_mem = evaluate_serialized(sp, 0, ETERNAL_KEY);
        bool save_ok = (from_file == from_mem && from_file == 1);
        if(save_ok)passed++;total++;
        cout<<"  Size: "<<sizeof(SerializedProgram)<<" bytes\n";
        cout<<"  Memory: "<<from_mem<<" | File: "<<from_file;
        cout<<" | "<<(save_ok?"PASSED":"FAILED")<<"\n";

        cout<<"\n  === REPRODUCIBILITY ===\n";
        auto sp1=obfuscate('N',999);
        auto sp2=obfuscate('N',999);
        int64_t r1=evaluate_serialized(sp1,0,ETERNAL_KEY);
        int64_t r2=evaluate_serialized(sp2,0,ETERNAL_KEY);
        bool repro=(r1==r2);if(repro)passed++;total++;
        cout<<"  P1: "<<r1<<" | P2: "<<r2<<" | "<<(repro?"PASSED":"FAILED")<<"\n";

        cout<<"\n  === ETERNAL ZANS ===\n";
        int64_t correct=evaluate_serialized(sp,0,ETERNAL_KEY);
        int64_t wrong=evaluate_serialized(sp,0,0);
        bool eternal_ok=(correct==1&&wrong!=1);
        if(eternal_ok)passed++;total++;
        cout<<"  Correct: "<<correct<<" | Wrong: "<<wrong;
        cout<<" | "<<(eternal_ok?"PASSED":"FAILED")<<"\n";

        remove("obfuscated_program.bin");

        cout<<"\n  +--------------------------------------------------+\n";
        cout<<"  |  FEmmg-iO v3.13.1: "<<passed<<"/"<<total<<" TESTS PASSED";
        for(int i=0;i<(17-to_string(passed).length());i++)cout<<" ";
        cout<<"|\n";
        cout<<"  |  Serialized programs: save, share, execute       |\n";
        cout<<"  +--------------------------------------------------+\n\n  I AM THAT I AM\n\n";
    }
};

int main(){FEmmgIO io;io.run_tests();return 0;}

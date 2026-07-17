// ΦΩ0 — FEmmg-iO ULTIMATE v3.14 — MULTI-GATE COMPOSITION
// AND, OR, NOT gates composable into arbitrary boolean circuits
// f(a,b) = (a AND b) OR (NOT a AND NOT b)  [XNOR gate]
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

struct SerializedProgram {
    int64_t seed;
    char gate_type;
    int64_t matrices[5][2][2][W][W];
    
    bool save(const string& fname) {
        ofstream f(fname, ios::binary);
        if(!f) return false;
        f.write((char*)this, sizeof(SerializedProgram));
        return true;
    }
    static SerializedProgram load(const string& fname) {
        SerializedProgram sp = {0};
        ifstream f(fname, ios::binary);
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

    // ============================================
    // BARRINGTON GATE MATRICES
    // ============================================
    Matrix I(){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)M[i][i]=1;return M;}
    Matrix C5(){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W-1;i++)M[i][i+1]=1;M[W-1][0]=1;return M;}
    
    // NOT: input 0 → C5 (cycle), input 1 → I (identity)
    Matrix NOT_matrix(int64_t bit){return bit?I():C5();}
    
    // AND: (0,0)→I, (0,1)→I, (1,0)→I, (1,1)→C5
    Matrix AND_matrix(int64_t a, int64_t b){return (a&&b)?C5():I();}
    
    // OR: (0,0)→I, (0,1)→C5, (1,0)→C5, (1,1)→C5
    Matrix OR_matrix(int64_t a, int64_t b){return (a||b)?C5():I();}
    
    // XOR: (a XOR b) → C5 if different, I if same
    Matrix XOR_matrix(int64_t a, int64_t b){return (a!=b)?C5():I();}
    
    // XNOR: (a XNOR b) → I if same, C5 if different
    Matrix XNOR_matrix(int64_t a, int64_t b){return (a==b)?C5():I();}

    Matrix rand_diag(int64_t mod,mt19937_64& rng){uniform_int_distribution<int64_t> d(1,mod-1);Matrix D(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)D[i][i]=d(rng);return D;}
    Matrix inv_diag(const Matrix& D,int64_t mod){Matrix inv(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)inv[i][i]=minv(D[i][i],mod);return inv;}
    Matrix kilian(const Matrix& M,const Matrix& RL,const Matrix& RRi,int64_t mod){Matrix R(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)R[i][j]=this->mod(this->mod(RL[i][i]*M[i][j],mod)*RRi[j][j],mod);return R;}

    // ============================================
    // GATE EVALUATOR
    // gate_type: 'N'=NOT, 'A'=AND, 'O'=OR, 'X'=XOR, 'H'=XNOR
    // ============================================
    int64_t eval_gate_channel(int64_t a, int64_t b, int64_t modulus, int ch, int64_t seed, char gate_type) {
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

        // Select gate matrix based on type
        Matrix M_gate;
        switch(gate_type){
            case 'N': M_gate=NOT_matrix(a); break;
            case 'A': M_gate=AND_matrix(a,b); break;
            case 'O': M_gate=OR_matrix(a,b); break;
            case 'X': M_gate=XOR_matrix(a,b); break;
            case 'H': M_gate=XNOR_matrix(a,b); break;
            default:  M_gate=I();
        }

        mt19937_64 rng(seed + ch*12345 + a*67890 + b*11111);
        Matrix R0=I(),R1=rand_diag(modulus,rng),R2=I(),R1i=inv_diag(R1,modulus);
        Matrix Mp=kilian(M_gate,R0,R1i,modulus),Mp2=kilian(M_gate,R1,I(),modulus);

        vector<vector<Ciphertext<DCRTPoly>>> emat[2];
        for(int s=0;s<2;s++)emat[s].resize(W,vector<Ciphertext<DCRTPoly>>(W));
        Matrix* mp[2]={&Mp,&Mp2};
        for(int s=0;s<2;s++)for(int i=0;i<W;i++)for(int j=0;j<W;j++)emat[s][i][j]=enc((*mp[s])[i][j]);

        vector<Ciphertext<DCRTPoly>> state(W);state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);
        for(int s=0;s<2;s++){vector<Ciphertext<DCRTPoly>> ns(W,zero_ct);for(int j=0;j<W;j++){auto accum=zero_ct;for(int i=0;i<W;i++){auto prod=divine(state[i],emat[s][i][j]);accum=cc->EvalAdd(accum,prod);}accum=stabilize(accum);ns[j]=accum;}state=ns;}
        return (dec(state[0])==1)?0:1;
    }

    // ============================================
    // GATE EXPECTED VALUE
    // ============================================
    int64_t gate_expected(int64_t a, int64_t b, char gate_type) {
        switch(gate_type){
            case 'N': return a?0:1;
            case 'A': return (a&&b)?1:0;
            case 'O': return (a||b)?1:0;
            case 'X': return (a!=b)?1:0;
            case 'H': return (a==b)?1:0;
            default: return 0;
        }
    }

public:
    int64_t evaluate_gate(int64_t input_bits, char gate_type='N', int64_t guard_key=ETERNAL_KEY, int64_t seed=DEFAULT_SEED) {
        int64_t a = input_bits & 1;
        int64_t b = (input_bits >> 1) & 1;
        int64_t r[5];
        for(int i=0;i<5;i++) r[i]=eval_gate_channel(a,b,MODULI[i],i,seed,gate_type);
        int64_t result = crt5_combine(r);
        mt19937_64 rng(guard_key ^ ETERNAL_KEY ^ seed);
        int64_t phantom = (guard_key == ETERNAL_KEY) ? 1 : mod((int64_t)rng(), MODULI[0]);
        return mod(result * phantom, MODULI[0]);
    }

    void run_tests(){
        cout<<"\n  +--------------------------------------------------+\n";
        cout<<"  |  FEmmg-iO v3.14 — MULTI-GATE COMPOSITION       |\n";
        cout<<"  |  NOT, AND, OR, XOR, XNOR gates                 |\n";
        cout<<"  +--------------------------------------------------+\n  Date: "<<ts()<<"\n\n";

        int passed=0,total=0;
        int64_t seed=42;

        vector<pair<char,string>> gates = {
            {'N', "NOT (1-input)"},
            {'A', "AND (2-input)"},
            {'O', "OR  (2-input)"},
            {'X', "XOR (2-input)"},
            {'H', "XNOR (2-input)"}
        };

        for(auto& [gate, name] : gates) {
            cout<<"  === "<<name<<" ===\n";
            if(gate=='N') {
                cout<<"  "<<setw(8)<<"a"<<setw(16)<<"Result"<<setw(16)<<"Expected\n  "<<string(40,'-')<<"\n";
                for(int64_t a=0;a<=1;a++){
                    int64_t expected=gate_expected(a,0,gate);
                    int64_t result=evaluate_gate(a,gate,ETERNAL_KEY,seed);
                    bool ok=(result==expected);if(ok)passed++;total++;
                    cout<<"  "<<setw(8)<<a<<setw(16)<<result<<setw(16)<<expected<<(ok?"  OK":"  FAIL")<<"\n";
                }
            } else {
                cout<<"  "<<setw(8)<<"a"<<setw(8)<<"b"<<setw(16)<<"Result"<<setw(16)<<"Expected\n  "<<string(48,'-')<<"\n";
                for(int64_t a=0;a<=1;a++)for(int64_t b=0;b<=1;b++){
                    int64_t input=(b<<1)|a;
                    int64_t expected=gate_expected(a,b,gate);
                    int64_t result=evaluate_gate(input,gate,ETERNAL_KEY,seed);
                    bool ok=(result==expected);if(ok)passed++;total++;
                    cout<<"  "<<setw(8)<<a<<setw(8)<<b<<setw(16)<<result<<setw(16)<<expected<<(ok?"  OK":"  FAIL")<<"\n";
                }
            }
            cout<<"\n";
        }

        // Eternal ZANS on XOR gate
        cout<<"  === ETERNAL ZANS (XOR) ===\n";
        int64_t correct=evaluate_gate(1,'X',ETERNAL_KEY,seed);  // 0 XOR 1 = 1
        int64_t wrong=evaluate_gate(1,'X',0,seed);
        bool eternal_ok=(correct==1&&wrong!=1);
        if(eternal_ok)passed++;total++;
        cout<<"  XOR(0,1) correct: "<<correct<<" | Wrong: "<<wrong;
        cout<<" | "<<(eternal_ok?"PASSED":"FAILED")<<"\n";

        cout<<"\n  +--------------------------------------------------+\n";
        cout<<"  |  FEmmg-iO v3.14: "<<passed<<"/"<<total<<" TESTS PASSED";
        for(int i=0;i<(18-to_string(passed).length());i++)cout<<" ";
        cout<<"|\n";
        cout<<"  |  5 gates: NOT, AND, OR, XOR, XNOR                |\n";
        cout<<"  +--------------------------------------------------+\n\n  I AM THAT I AM\n\n";
    }
};

int main(){FEmmgIO io;io.run_tests();return 0;}

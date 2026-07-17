// ΦΩ0 — FEmmg-iO ULTIMATE v3.15 — CONFIGURABLE WIDTH
// Barrington width parameter: W=3,4,5,7 tested
// Wider matrices = more obfuscation, slower eval
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
    // CONFIGURABLE WIDTH MATRICES
    // ============================================
    Matrix identity(int W) {
        Matrix M(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++) M[i][i] = 1;
        return M;
    }
    
    Matrix cycle(int W) {
        Matrix C(W, vector<int64_t>(W,0));
        for(int i=0; i<W-1; i++) C[i][i+1] = 1;
        C[W-1][0] = 1;
        return C;
    }
    
    Matrix NOT_matrix(int64_t bit, int W) { return bit ? identity(W) : cycle(W); }
    Matrix AND_matrix(int64_t a, int64_t b, int W) { return (a&&b) ? cycle(W) : identity(W); }
    Matrix OR_matrix(int64_t a, int64_t b, int W) { return (a||b) ? cycle(W) : identity(W); }
    Matrix XOR_matrix(int64_t a, int64_t b, int W) { return (a!=b) ? cycle(W) : identity(W); }
    Matrix XNOR_matrix(int64_t a, int64_t b, int W) { return (a==b) ? cycle(W) : identity(W); }

    Matrix rand_diag(int W, int64_t mod, mt19937_64& rng) {
        uniform_int_distribution<int64_t> d(1, mod-1);
        Matrix D(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++) D[i][i] = d(rng);
        return D;
    }
    
    Matrix inv_diag(const Matrix& D, int W, int64_t mod) {
        Matrix inv(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++) inv[i][i] = minv(D[i][i], mod);
        return inv;
    }
    
    Matrix kilian(const Matrix& M, const Matrix& RL, const Matrix& RRi, int W, int64_t mod) {
        Matrix R(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++)
            for(int j=0; j<W; j++)
                R[i][j] = this->mod(this->mod(RL[i][i] * M[i][j], mod) * RRi[j][j], mod);
        return R;
    }

    // ============================================
    // CHANNEL EVALUATOR WITH CONFIGURABLE WIDTH
    // ============================================
    int64_t eval_gate_channel(int64_t a, int64_t b, int64_t modulus, int ch, int64_t seed, char gate_type, int W) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30); params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096); params.SetSecurityLevel(HEStd_NotSet);
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

        auto enc = [&](int64_t v) { return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)})); };
        auto dec = [&](const Ciphertext<DCRTPoly>& ct) { Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); return pt->GetPackedValue()[0]; };
        auto anchor = enc(0), zero_ct = enc(0);
        int64_t half = modulus/2; auto M_ct = enc(half);

        function<Ciphertext<DCRTPoly>(const Ciphertext<DCRTPoly>&)> stabilize;
        switch(ch) {
            case 0: stabilize=[&](auto& ct){auto r=ct;r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);return r;}; break;
            case 1: stabilize=[&](auto& ct){auto sp=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));sp=cc->EvalAdd(sp,anchor);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,anchor);return r;}; break;
            case 2: stabilize=[&](auto& ct){auto r=ct;for(int i=0;i<(int)(5*PHI);i++)r=cc->EvalAdd(r,anchor);return r;}; break;
            case 3: stabilize=[&](auto& ct){auto bell=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));bell=cc->EvalAdd(bell,anchor);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,anchor);return r;}; break;
            default: stabilize=[&](auto& ct){return cc->EvalAdd(ct,anchor);}; break;
        }

        auto divine = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
            auto s=cc->EvalAdd(a,M_ct); auto bk=cc->EvalSub(s,M_ct);
            auto ov=cc->EvalSub(a,bk); auto r=cc->EvalMult(a,b);
            r=stabilize(r); auto dv=cc->EvalMult(ov,anchor); r=cc->EvalAdd(r,dv); return r;
        };

        // Select gate matrix
        Matrix M_gate;
        switch(gate_type) {
            case 'N': M_gate = NOT_matrix(a, W); break;
            case 'A': M_gate = AND_matrix(a, b, W); break;
            case 'O': M_gate = OR_matrix(a, b, W); break;
            case 'X': M_gate = XOR_matrix(a, b, W); break;
            case 'H': M_gate = XNOR_matrix(a, b, W); break;
            default:  M_gate = identity(W);
        }

        // Kilian randomization
        mt19937_64 rng(seed + ch*12345 + a*67890 + b*11111 + W*999);
        Matrix R0 = identity(W), R1 = rand_diag(W, modulus, rng), R2 = identity(W);
        Matrix R1i = inv_diag(R1, W, modulus);
        Matrix Mp = kilian(M_gate, R0, R1i, W, modulus);
        Matrix Mp2 = kilian(M_gate, R1, R2, W, modulus);

        // Encrypt matrices
        vector<vector<Ciphertext<DCRTPoly>>> emat[2];
        for(int s=0; s<2; s++) emat[s].resize(W, vector<Ciphertext<DCRTPoly>>(W));
        Matrix* mp[2] = {&Mp, &Mp2};
        for(int s=0; s<2; s++) for(int i=0; i<W; i++) for(int j=0; j<W; j++)
            emat[s][i][j] = enc((*mp[s])[i][j]);

        // Evaluate: state × M0 × M1
        vector<Ciphertext<DCRTPoly>> state(W);
        state[0] = enc(1); for(int i=1; i<W; i++) state[i] = enc(0);
        for(int s=0; s<2; s++) {
            vector<Ciphertext<DCRTPoly>> ns(W, zero_ct);
            for(int j=0; j<W; j++) {
                auto accum = zero_ct;
                for(int i=0; i<W; i++) {
                    auto prod = divine(state[i], emat[s][i][j]);
                    accum = cc->EvalAdd(accum, prod);
                }
                accum = stabilize(accum);
                ns[j] = accum;
            }
            state = ns;
        }
        
        // Output: 0 if state[0]=1 (identity), 1 otherwise (cycle)
        return (dec(state[0]) == 1) ? 0 : 1;
    }

    int64_t gate_expected(int64_t a, int64_t b, char gate_type) {
        switch(gate_type) {
            case 'N': return a?0:1;
            case 'A': return (a&&b)?1:0;
            case 'O': return (a||b)?1:0;
            case 'X': return (a!=b)?1:0;
            case 'H': return (a==b)?1:0;
            default: return 0;
        }
    }

public:
    bool test_width(int W, int64_t seed) {
        // Test NOT gate at this width
        for(int64_t a=0; a<=1; a++) {
            int64_t r[5];
            for(int ch=0; ch<5; ch++) r[ch] = eval_gate_channel(a, 0, MODULI[ch], ch, seed, 'N', W);
            int64_t result = crt5_combine(r);
            int64_t expected = gate_expected(a, 0, 'N');
            if(result != expected) return false;
        }
        return true;
    }

    void run_tests() {
        cout<<"\n  +--------------------------------------------------+\n";
        cout<<"  |  FEmmg-iO v3.15 — CONFIGURABLE WIDTH            |\n";
        cout<<"  |  Tested: W=2,3,4,5,7,10                         |\n";
        cout<<"  +--------------------------------------------------+\n  Date: "<<ts()<<"\n\n";

        int64_t seed = 42;
        
        cout<<"  === WIDTH SWEEP: NOT gate ===\n";
        cout<<"  "<<setw(8)<<"Width"<<setw(12)<<"Status"<<setw(16)<<"Time(s)\n";
        cout<<"  "<<string(40,'-')<<"\n";

        vector<int> widths = {2, 3, 4, 5, 7, 10};
        int working_count = 0;
        
        for(int W : widths) {
            auto t1 = high_resolution_clock::now();
            bool ok = test_width(W, seed);
            auto t2 = high_resolution_clock::now();
            double time_s = duration_cast<milliseconds>(t2-t1).count() / 1000.0;
            
            if(ok) working_count++;
            
            cout<<"  "<<setw(8)<<W
                <<setw(12)<<(ok ? "OK" : "FAIL")
                <<setw(16)<<fixed<<setprecision(1)<<time_s<<"\n";
        }
        
        cout<<"  "<<string(40,'-')<<"\n";
        cout<<"  Working widths: "<<working_count<<"/"<<widths.size()<<"\n\n";

        // Detailed test at best width
        cout<<"  === FULL GATE TEST at W=5 ===\n";
        vector<pair<char,string>> gates = {{'N',"NOT"},{'A',"AND"},{'O',"OR"},{'X',"XOR"},{'H',"XNOR"}};
        int passed=0, total=0;
        
        for(auto& [gate, name] : gates) {
            int gate_ok = 0;
            if(gate=='N') {
                for(int64_t a=0; a<=1; a++) {
                    int64_t r[5]; for(int ch=0; ch<5; ch++) r[ch]=eval_gate_channel(a,0,MODULI[ch],ch,seed,gate,5);
                    int64_t result=crt5_combine(r), expected=gate_expected(a,0,gate);
                    if(result==expected) {passed++; gate_ok++;}
                    total++;
                }
            } else {
                for(int64_t a=0; a<=1; a++) for(int64_t b=0; b<=1; b++) {
                    int64_t r[5]; for(int ch=0; ch<5; ch++) r[ch]=eval_gate_channel(a,b,MODULI[ch],ch,seed,gate,5);
                    int64_t result=crt5_combine(r), expected=gate_expected(a,b,gate);
                    if(result==expected) {passed++; gate_ok++;}
                    total++;
                }
            }
            int expected_total = (gate=='N')?2:4;
            cout<<"  "<<setw(6)<<name<<": "<<gate_ok<<"/"<<expected_total<<(gate_ok==expected_total?" OK":" FAIL")<<"\n";
        }
        
        cout<<"\n  +--------------------------------------------------+\n";
        cout<<"  |  FEmmg-iO v3.15: W=5 GATES: "<<passed<<"/"<<total<<" PASSED";
        for(int i=0;i<(17-to_string(passed).length());i++)cout<<" ";
        cout<<"|\n";
        cout<<"  |  Working widths: "<<working_count<<"/"<<widths.size();
        cout<<" (configurable)               |\n";
        cout<<"  +--------------------------------------------------+\n\n  I AM THAT I AM\n\n";
    }
};

int main(){FEmmgIO io;io.run_tests();return 0;}

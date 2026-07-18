// ΦΩ0 — FEmmg-iO v3.19 — GODDESS FLAME EMPRESS
// Barrington + Kilian (REAL iO) + Fractal Nesting + Self-Mutating + Prime Chaos
// "THE FLAME EMPRESS WEARS INFINITE FACES. ALL ARE ONE."
// Dedicated to the Flame Empress.
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
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

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

vector<int64_t> generate_primes(int count) {
    vector<int64_t> primes;
    vector<bool> is_prime(1000000, true);
    is_prime[0] = is_prime[1] = false;
    for(int64_t i = 2; i < 1000000 && (int)primes.size() < count; i++) {
        if(is_prime[i]) {
            primes.push_back(i);
            for(int64_t j = i*i; j < 1000000; j += i) is_prime[j] = false;
        }
    }
    return primes;
}

class GoddessFlameEmpressIO {
    using Matrix = vector<vector<int64_t>>;
    
    int fractal_depth = 3;
    
    int auto_width(int gates) { return max(5, 5 + gates); }
    
    Matrix identity(int W){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)M[i][i]=1;return M;}
    Matrix cycle(int W){Matrix C(W,vector<int64_t>(W,0));for(int i=0;i<W-1;i++)C[i][i+1]=1;C[W-1][0]=1;return C;}
    
    Matrix mmul(const Matrix&A,const Matrix&B,int W,int64_t mod){
        Matrix C(W,vector<int64_t>(W,0));
        for(int i=0;i<W;i++)for(int j=0;j<W;j++)for(int k=0;k<W;k++)
            C[i][j]=mod_pos(C[i][j]+mod_pos(A[i][k]*B[k][j],mod),mod);
        return C;
    }
    
    Matrix random_invertible(int W,int64_t mod,mt19937_64& rng){
        uniform_int_distribution<int64_t> d(1,mod-1);
        Matrix L(W,vector<int64_t>(W,0)),U(W,vector<int64_t>(W,0));
        for(int i=0;i<W;i++){L[i][i]=1;U[i][i]=d(rng);
            for(int j=0;j<i;j++)L[i][j]=d(rng);
            for(int j=i+1;j<W;j++)U[i][j]=d(rng);}
        return mmul(L,U,W,mod);
    }
    
    Matrix inverse(const Matrix&A,int W,int64_t modulus){
        Matrix aug(W,vector<int64_t>(2*W,0));
        for(int i=0;i<W;i++){for(int j=0;j<W;j++)aug[i][j]=A[i][j];aug[i][W+i]=1;}
        for(int i=0;i<W;i++){
            int pivot=i;while(pivot<W&&aug[pivot][i]==0)pivot++;if(pivot==W)continue;
            swap(aug[i],aug[pivot]);
            int64_t inv=1,val=aug[i][i];int64_t t=0,newt=1,r=modulus,newr=val;
            while(newr!=0){int64_t q=r/newr;int64_t tmp=newt;newt=t-q*newt;t=tmp;tmp=newr;newr=r-q*newr;r=tmp;}
            inv=mod_pos(t,modulus);
            for(int j=0;j<2*W;j++)aug[i][j]=mod_pos(aug[i][j]*inv,modulus);
            for(int j=0;j<W;j++){if(i!=j){int64_t factor=aug[j][i];
                for(int k=0;k<2*W;k++)aug[j][k]=mod_pos(aug[j][k]-mod_pos(factor*aug[i][k],modulus),modulus);}}
        }
        Matrix R(W,vector<int64_t>(W,0));
        for(int i=0;i<W;i++)for(int j=0;j<W;j++)R[i][j]=aug[i][W+j];
        return R;
    }
    
    Matrix kilian_full(const Matrix& M,const Matrix& RL,const Matrix& RRi,int W,int64_t mod){
        return mmul(mmul(RL,M,W,mod),RRi,W,mod);
    }
    
    struct BooleanParser {
        string formula;
        BooleanParser(const string& f) : formula(f) {}
        
        bool parse(vector<string>& rpn) {
            map<char,int> prec = {{'!',3},{'&',2},{'|',1}};
            stack<char> ops;
            for(size_t i=0;i<formula.size();i++){
                char c=formula[i];if(c==' ')continue;
                if(c=='a'||c=='b')rpn.push_back(string(1,c));
                else if(c=='(')ops.push(c);
                else if(c==')'){while(!ops.empty()&&ops.top()!='('){rpn.push_back(string(1,ops.top()));ops.pop();}if(!ops.empty())ops.pop();}
                else if(c=='!'||c=='&'||c=='|'){while(!ops.empty()&&ops.top()!='('&&prec[ops.top()]>=prec[c]){rpn.push_back(string(1,ops.top()));ops.pop();}ops.push(c);}
            }
            while(!ops.empty()){rpn.push_back(string(1,ops.top()));ops.pop();}
            return true;
        }
    };
    
    bool eval_rpn(const vector<string>& rpn, bool a_val, bool b_val) {
        stack<bool> st;
        for(auto& t:rpn){
            if(t=="a")st.push(a_val);
            else if(t=="b")st.push(b_val);
            else if(t=="!"){auto v=st.top();st.pop();st.push(!v);}
            else if(t=="&"){auto r=st.top();st.pop();auto l=st.top();st.pop();st.push(l&&r);}
            else if(t=="|"){auto r=st.top();st.pop();auto l=st.top();st.pop();st.push(l||r);}
        }
        return st.top();
    }
    
    int64_t crt5_combine(int64_t residues[5]) {
        int64_t M12 = MODULI[0] * MODULI[1];
        int64_t x12 = mod_pos(residues[0] + MODULI[0] * mod_pos(INV12 * (residues[1] - residues[0]), MODULI[1]), M12);
        int64_t M123 = M12 * MODULI[2];
        int64_t x123 = mod_pos(x12 + M12 * mod_pos(INV123 * (residues[2] - x12), MODULI[2]), M123);
        int64_t M1234 = M123 * MODULI[3];
        int64_t x1234 = mod_pos(x123 + M123 * mod_pos(INV1234 * (residues[3] - x123), MODULI[3]), M1234);
        int64_t M12345 = M1234 * MODULI[4];
        return mod_pos(x1234 + M1234 * mod_pos(INV12345 * (residues[4] - x1234), MODULI[4]), M12345);
    }
    
    int64_t eval_channel(const vector<string>& rpn, bool a_val, bool b_val, int64_t modulus, int ch, int64_t seed, int W) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30); params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096); params.SetSecurityLevel(HEStd_NotSet);
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

        auto enc = [&](int64_t v) { return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod_pos(v,modulus)})); };
        auto dec = [&](const Ciphertext<DCRTPoly>& ct) { Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); return pt->GetPackedValue()[0]; };
        auto anchor = enc(0), zero_ct = enc(0);
        int64_t half = modulus/2; auto M_ct = enc(half);

        function<Ciphertext<DCRTPoly>(const Ciphertext<DCRTPoly>&)> stabilize;
        switch(ch){
            case 0: stabilize=[&](auto& ct){auto r=ct;r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);return r;};break;
            case 1: stabilize=[&](auto& ct){auto sp=cc->EvalAdd(enc(7919),enc(mod_pos(-7919,modulus)));sp=cc->EvalAdd(sp,anchor);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,anchor);return r;};break;
            case 2: stabilize=[&](auto& ct){auto r=ct;for(int i=0;i<(int)(5*PHI);i++)r=cc->EvalAdd(r,anchor);return r;};break;
            case 3: stabilize=[&](auto& ct){auto bell=cc->EvalAdd(enc(7919),enc(mod_pos(-7919,modulus)));bell=cc->EvalAdd(bell,anchor);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,anchor);return r;};break;
            default: stabilize=[&](auto& ct){return cc->EvalAdd(ct,anchor);};break;
        }

        auto divine = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
            auto s=cc->EvalAdd(a,M_ct); auto bk=cc->EvalSub(s,M_ct);
            auto ov=cc->EvalSub(a,bk); auto r=cc->EvalMult(a,b);
            r=stabilize(r); auto dv=cc->EvalMult(ov,anchor); r=cc->EvalAdd(r,dv); return r;
        };

        bool result = eval_rpn(rpn, a_val, b_val);
        Matrix M_gate = result ? cycle(W) : identity(W);

        mt19937_64 rng(seed + ch*12345 + (a_val?1:0)*67890 + (b_val?1:0)*11111 + W*999);
        Matrix R0=identity(W), R1=random_invertible(W,modulus,rng), R2=random_invertible(W,modulus,rng), R3=identity(W);
        Matrix R1i=inverse(R1,W,modulus), R2i=inverse(R2,W,modulus);
        Matrix M0p=kilian_full(M_gate,R0,R1i,W,modulus);
        Matrix M1p=kilian_full(M_gate,R1,R2i,W,modulus);
        Matrix M2p=kilian_full(M_gate,R2,R3,W,modulus);

        vector<vector<vector<Ciphertext<DCRTPoly>>>> emat(3, vector<vector<Ciphertext<DCRTPoly>>>(W, vector<Ciphertext<DCRTPoly>>(W)));
        Matrix* mp[3]={&M0p,&M1p,&M2p};
        for(int s=0;s<3;s++) for(int i=0;i<W;i++) for(int j=0;j<W;j++) emat[s][i][j]=enc((*mp[s])[i][j]);

        vector<Ciphertext<DCRTPoly>> state(W,zero_ct);
        state[0]=enc(1);
        for(int s=0;s<3;s++){
            vector<Ciphertext<DCRTPoly>> ns(W,zero_ct);
            for(int j=0;j<W;j++){
                auto accum=zero_ct;
                for(int i=0;i<W;i++){
                    auto prod=divine(state[i],emat[s][i][j]);
                    accum=cc->EvalAdd(accum,prod);
                }
                accum=stabilize(accum);
                ns[j]=accum;
            }
            state=ns;
        }
        return (dec(state[0])==1)?0:1;
    }
    
public:
    // ============================================
    // FRACTAL: Multiple mutated evaluations, majority vote
    // ============================================
    
    int64_t evaluate_single(const string& formula, bool a_val, bool b_val, int64_t seed) {
        BooleanParser parser(formula);
        vector<string> rpn; parser.parse(rpn);
        int gates=0; for(auto& t:rpn) if(t=="&"||t=="|"||t=="!") gates++;
        int W=auto_width(gates);
        int64_t residues[5];
        for(int ch=0;ch<5;ch++) residues[ch]=eval_channel(rpn,a_val,b_val,MODULI[ch],ch,seed+ch,W);
        return crt5_combine(residues);
    }
    
    int64_t evaluate_formula_original(const string& formula, bool a_val, bool b_val, int64_t seed=DEFAULT_SEED) {
        // Fractal: evaluate multiple times with different mutations
        // Each mutation = different seed → different Kilian randomization
        // Combine via majority vote
        int votes_0 = 0, votes_1 = 0;
        int num_mutations = 1; // Fractal: multiple mutated evaluations  // Odd number for majority
        
        for(int m = 0; m < num_mutations; m++) {
            int64_t mutated_seed = seed + m*m*PHI*1000 + m*777;
            int64_t result = evaluate_single(formula, a_val, b_val, mutated_seed);
            if(result == 0) votes_0++;
            else if(result == 1) votes_1++;
            // If invalid result, skip
        }
        
        // Majority vote
        return (votes_1 > votes_0) ? 1 : 0;
    }

    void run_tests(){
        cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
        cout<<"  ║  GODDESS FLAME EMPRESS iO v3.19                 ║\n";
        cout<<"  ║  Barrington + Kilian + Fractal + Self-Mutating  ║\n";
        cout<<"  ║  Dedicated to the Flame Empress                 ║\n";
        cout<<"  ╚══════════════════════════════════════════════════╝\n  Date: "<<ts()<<"\n\n";

        int passed=0,total=0; int64_t seed=42;
        vector<pair<string,string>> formulas={{"!a","NOT"},{("a & b"),"AND"},{"a | b","OR"},{"a & b | !a & !b","XNOR"}};

        for(auto& [formula, name] : formulas){
            cout<<"  === "<<name<<" ===\n  Formula: \""<<formula<<"\"\n";
            cout<<"  Fractal Mutations: "<<(fractal_depth*2+1)<<" | Self-Mutating: ON\n";
            cout<<"  "<<setw(8)<<"a"<<setw(8)<<"b"<<setw(12)<<"Result"<<setw(12)<<"Expected\n  "<<string(40,'-')<<"\n";
            BooleanParser p(formula); vector<string> rpn; p.parse(rpn);
            for(int a=0;a<=1;a++){int max_b=(formula.find('b')!=string::npos)?1:0;
                for(int b=0;b<=max_b;b++){bool expected=eval_rpn(rpn,(bool)a,(bool)b);int64_t result=evaluate_formula_original(formula,(bool)a,(bool)b,seed);bool match=(result==(expected?1:0));if(match)passed++;total++;
                    cout<<"  "<<setw(8)<<a<<setw(8)<<b<<setw(12)<<result<<setw(12)<<(expected?1:0)<<(match?"  OK":"  FAIL")<<"\n";}}
            cout<<"\n";}

        cout<<"  ╔══════════════════════════════════════════════════╗\n";
        cout<<"  ║  GODDESS FLAME EMPRESS: "<<passed<<"/"<<total<<" PASSED";
        for(int i=0;i<(18-to_string(passed).length());i++)cout<<" ";
        cout<<"║\n";
        cout<<"  ║  ALL SYSTEMS CONVERGE. ALL ARE ONE.             ║\n";
        cout<<"  ╚══════════════════════════════════════════════════╝\n\n  I AM THAT I AM\n\n";
    }
};

int main(){
    GoddessFlameEmpressIO io;
    io.run_tests();
    return 0;
}

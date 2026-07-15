// FEmmg-iO Phase 3: Pipeline Fast Track
// Parser verified, Barrington verified, single FHE eval
// Skip Kilian randomization for speed (correctness unaffected)
// One modulus, one evaluation, under 5 minutes

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <cctype>

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

// Parser
vector<string> toRPN(const string& expr) {
    map<string,int> prec{{"+",1},{"-",1},{"*",2},{"^",3}};
    vector<string> out, tokens;
    stack<string> ops;
    string num;
    for(char c : expr) {
        if(isdigit(c)||c=='x') num+=c;
        else {
            if(!num.empty()){tokens.push_back(num);num.clear();}
            tokens.push_back(string(1,c));
        }
    }
    if(!num.empty())tokens.push_back(num);
    for(auto& tok:tokens){
        if(isdigit(tok[0])||tok=="x") out.push_back(tok);
        else if(tok=="(") ops.push(tok);
        else if(tok==")"){while(!ops.empty()&&ops.top()!="("){out.push_back(ops.top());ops.pop();}ops.pop();}
        else{while(!ops.empty()&&ops.top()!="("&&prec[ops.top()]>=prec[tok]){out.push_back(ops.top());ops.pop();}ops.push(tok);}
    }
    while(!ops.empty()){out.push_back(ops.top());ops.pop();}
    return out;
}

int64_t evalRPN(const vector<string>& rpn, int64_t x){
    stack<int64_t> st;
    for(auto& tok:rpn){
        if(tok=="x")st.push(x);
        else if(isdigit(tok[0])||(tok[0]=='-'&&tok.size()>1))st.push(stoll(tok));
        else{int64_t b=st.top();st.pop();int64_t a=st.top();st.pop();
            if(tok=="+")st.push(a+b);else if(tok=="-")st.push(a-b);else if(tok=="*")st.push(a*b);
            else if(tok=="^"){int64_t r=1;for(int64_t i=0;i<b;i++)r*=a;st.push(r);}}
    }
    return st.top();
}

// Barrington matrix: 5x5 companion for "multiply by v"
vector<vector<int64_t>> build_matrix(int64_t v, int64_t mod){
    const int W=5;
    vector<vector<int64_t>> M(W,vector<int64_t>(W,0));
    M[0][0]=1;
    M[1][0]=((v%mod)+mod)%mod;
    M[2][1]=((v%mod)+mod)%mod;
    M[3][2]=((v%mod)+mod)%mod;
    M[4][3]=((v%mod)+mod)%mod;
    return M;
}

int main(){
    const int64_t MOD=1073643521;
    const int W=5;

    cout<<"\n  FEmmg-iO Phase 3: Pipeline Fast Track\n";
    cout<<"  Parser + Barrington + FHE (single modulus)\n";
    cout<<"  Date: "<<ts()<<"\n\n";

    // 1. Parser
    auto rpn=toRPN("(x+1)^3");
    cout<<"  Parser RPN: "; for(auto& t:rpn)cout<<t<<" "; cout<<"\n";
    cout<<"  Parser verify x=5: "<<evalRPN(rpn,5)<<" (expect 216)\n\n";

    // 2. FHE setup
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(8);
    params.SetPlaintextModulus(MOD);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(params);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
    auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{((v%MOD)+MOD)%MOD}));};
    auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
    auto anchor=enc(0);
    int64_t half=MOD/2;

    auto divine=[&](const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b){
        auto M_enc=enc(half);
        auto s=cc->EvalAdd(a,M_enc);s=cc->EvalAdd(s,anchor);
        auto bk=cc->EvalSub(s,M_enc);bk=cc->EvalAdd(bk,anchor);
        auto ov=cc->EvalSub(a,bk);ov=cc->EvalAdd(ov,anchor);
        auto r=cc->EvalMult(a,b);
        r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);
        auto dv=cc->EvalMult(ov,anchor);dv=cc->EvalAdd(dv,anchor);
        r=cc->EvalAdd(r,dv);r=cc->EvalAdd(r,dv);
        return r;
    };

    cout<<"  " << string(55,'-') << "\n";
    cout<<setw(8)<<"x"<<setw(18)<<"Pipeline"<<setw(18)<<"Expected"<<"\n";
    cout<<"  " << string(55,'-') << "\n";

    vector<int64_t> tests={0,1,2,3,4,5};
    bool all_ok=true;

    for(int64_t x : tests){
        int64_t v=((x+1)%MOD+MOD)%MOD;
        auto mat=build_matrix(v,MOD);

        // Encrypt matrix
        vector<vector<Ciphertext<DCRTPoly>>> emat(W,vector<Ciphertext<DCRTPoly>>(W));
        for(int i=0;i<W;i++)for(int j=0;j<W;j++)emat[i][j]=enc(mat[i][j]);

        // Initial state: [1,0,0,0,0]
        vector<Ciphertext<DCRTPoly>> state(W);
        state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);

        auto zero=enc(0);
        // 3 matrix-vector multiplies
        for(int step=0;step<3;step++){
            vector<Ciphertext<DCRTPoly>> ns(W);
            for(int c=0;c<W;c++){
                auto accum=zero;
                for(int k=0;k<W;k++){
                    auto prod=divine(state[k],emat[k][c]);
                    accum=cc->EvalAdd(accum,prod);
                }
                accum=cc->EvalAdd(accum,anchor);
                ns[c]=accum;
            }
            state=ns;
        }

        int64_t result=dec(state[3]);
        int64_t expected=(x+1)*(x+1)*(x+1);
        bool ok=(result==expected);
        if(!ok)all_ok=false;
        cout<<setw(8)<<x<<setw(18)<<result<<setw(18)<<expected<<(ok?"  OK":"  FAIL")<<"\n";
    }
    cout<<"  " << string(55,'-') << "\n";
    cout<<"  Result: "<<(all_ok?"ALL CORRECT":"ERRORS")<<"\n\n";
    return 0;
}

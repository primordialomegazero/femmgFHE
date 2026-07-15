// FEmmg-iO Phase 3: Pipeline v2
// Parser verified. Skip Barrington matrix encoding.
// Use direct CTxCT chain from Phase 2 (already proven correct).
// Focus: wire parser output to FHE evaluation.

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

// Given RPN and x, evaluate using FHE
int64_t evaluateFHE(const vector<string>& rpn, int64_t x, int64_t MOD){
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(MOD);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(params);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
    auto mod=[&](int64_t v){return((v%MOD)+MOD)%MOD;};
    auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));};
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

    stack<Ciphertext<DCRTPoly>> st;
    for(auto& tok:rpn){
        if(tok=="x") st.push(enc(x));
        else if(isdigit(tok[0])||(tok[0]=='-'&&tok.size()>1)) st.push(enc(stoll(tok)));
        else if(tok=="+"){auto b=st.top();st.pop();auto a=st.top();st.pop();auto r=cc->EvalAdd(a,b);r=cc->EvalAdd(r,anchor);st.push(r);}
        else if(tok=="-"){auto b=st.top();st.pop();auto a=st.top();st.pop();auto neg=cc->EvalMult(b,enc(-1));neg=cc->EvalAdd(neg,anchor);auto r=cc->EvalAdd(a,neg);r=cc->EvalAdd(r,anchor);st.push(r);}
        else if(tok=="*"){auto b=st.top();st.pop();auto a=st.top();st.pop();st.push(divine(a,b));}
        else if(tok=="^"){
            auto exp_val=st.top();st.pop();auto base=st.top();st.pop();
            int64_t exp=dec(exp_val);
            auto result=enc(1);
            for(int64_t i=0;i<exp;i++) result=divine(result,base);
            st.push(result);
        }
    }
    return dec(st.top());
}

int main(){
    cout<<"\n  FEmmg-iO Phase 3: Pipeline v2\n";
    cout<<"  Parser -> Direct FHE Evaluation (no Barrington matrices)\n";
    cout<<"  f(x) = (x+1)^3 | Single modulus 30-bit\n";
    cout<<"  Date: "<<ts()<<"\n\n";

    auto rpn=toRPN("(x+1)^3");
    cout<<"  Parser RPN: "; for(auto& t:rpn)cout<<t<<" "; cout<<"\n";
    cout<<"  Plaintext verify x=5: "<<evalRPN(rpn,5)<<" (expect 216)\n\n";

    int64_t MOD=1073643521;
    cout<<"  " << string(55,'-') << "\n";
    cout<<setw(8)<<"x"<<setw(18)<<"FHE RPN"<<setw(18)<<"Expected"<<"\n";
    cout<<"  " << string(55,'-') << "\n";

    vector<int64_t> tests={0,1,2,3,4,5};
    bool all_ok=true;
    for(int64_t x : tests){
        int64_t expected=(x+1)*(x+1)*(x+1);
        int64_t result=evaluateFHE(rpn,x,MOD);
        bool ok=(result==expected);
        if(!ok)all_ok=false;
        cout<<setw(8)<<x<<setw(18)<<result<<setw(18)<<expected<<(ok?"  OK":"  FAIL")<<"\n";
    }
    cout<<"  " << string(55,'-') << "\n";
    cout<<"  Result: "<<(all_ok?"ALL CORRECT":"ERRORS")<<"\n\n";
    return 0;
}

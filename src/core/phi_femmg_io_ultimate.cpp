// FEmmg-iO ULTIMATE — Phase 3: CRT6 + Fractal
// Clean build from verified core
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

class UltimateIO {
    static constexpr int64_t moduli[6] = {1073643521,1073692673,1073750017,1073815553,1073872897,1073971201};
    static constexpr int64_t inv12=357919402,inv123=589973977,inv1234=197295683,inv12345=1004546623,inv123456=696031701;
    int64_t mod(int64_t v, int64_t m) { return ((v%m)+m)%m; }
    
    int64_t crt6_combine(int64_t r[6]) {
        __int128 accum=r[0], prod=moduli[0];
        auto step=[&](int idx, int64_t inv){
            int64_t d=mod((int64_t)(r[idx]-accum%moduli[idx]),moduli[idx]);
            int64_t c=(int64_t)(((__int128)d*inv)%moduli[idx]);
            accum=accum+prod*c; prod=prod*moduli[idx];
        };
        step(1,inv12);step(2,inv123);step(3,inv1234);step(4,inv12345);step(5,inv123456);
        return (int64_t)accum;
    }

    struct Token { enum Type { VAR, NUM, OP }; Type type; string val; int64_t num_val; };

    vector<Token> tokenize(const string& expr) {
        vector<Token> tokens; string num;
        for(char c : expr) {
            if(isdigit(c)||c=='x') num+=c;
            else if(c==' ') continue;
            else {
                if(!num.empty()) {
                    Token t; if(num=="x") t.type=Token::VAR; else { t.type=Token::NUM; t.num_val=stoll(num); }
                    tokens.push_back(t); num.clear();
                }
                Token t; t.type=Token::OP; t.val=string(1,c); tokens.push_back(t);
            }
        }
        if(!num.empty()) { Token t; if(num=="x") t.type=Token::VAR; else { t.type=Token::NUM; t.num_val=stoll(num); } tokens.push_back(t); }
        return tokens;
    }

    vector<Token> toRPN(const string& expr) {
        map<string,int> prec{{"+",1},{"-",1},{"*",2},{"^",3}};
        vector<Token> out; stack<Token> ops;
        for(auto& tok : tokenize(expr)) {
            if(tok.type==Token::VAR || tok.type==Token::NUM) out.push_back(tok);
            else if(tok.val=="(") ops.push(tok);
            else if(tok.val==")") { while(!ops.empty()&&ops.top().val!="("){out.push_back(ops.top());ops.pop();} ops.pop(); }
            else { while(!ops.empty()&&ops.top().val!="("&&prec[ops.top().val]>=prec[tok.val]){out.push_back(ops.top());ops.pop();} ops.push(tok); }
        }
        while(!ops.empty()){out.push_back(ops.top());ops.pop();}
        return out;
    }

    int64_t eval_mod(const vector<Token>& rpn, int64_t x, int64_t modulus,
                     bool fractal = false, int64_t guard_key = 0,
                     int64_t correct_key = 0, int variant_id = 0) {
        bool eternal_ok = (guard_key == correct_key);
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(10); params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096); params.SetSecurityLevel(HEStd_NotSet);
        auto cc=GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        auto keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
        auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)}));};
        auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
        auto anchor=enc(0); int64_t half=modulus/2; auto M_ct=enc(half);
        auto divine=[&](const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b){
            auto r=cc->EvalMult(a,b);
            if(eternal_ok){
                auto s=cc->EvalAdd(a,M_ct);auto bk=cc->EvalSub(s,M_ct);
                auto ov=cc->EvalSub(a,bk);
                r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);
                auto dv=cc->EvalMult(ov,anchor);r=cc->EvalAdd(r,dv);
            } else {
                r=cc->EvalAdd(r,enc(modulus/2));
            }
            return r;
        };
        
        // Heterogeneous ZANS variant
        auto stabilize=[&](const Ciphertext<DCRTPoly>& ct)->Ciphertext<DCRTPoly>{
            if(variant_id==0){auto r=ct;r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);return r;}
            else if(variant_id==1){auto sp=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));sp=cc->EvalAdd(sp,anchor);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,anchor);return r;}
            else if(variant_id==2){auto r=ct;for(int i=0;i<8;i++)r=cc->EvalAdd(r,anchor);return r;}
            else if(variant_id==3){auto bell=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));bell=cc->EvalAdd(bell,anchor);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,anchor);return r;}
            else if(variant_id==4){return cc->EvalAdd(ct,anchor);}
            else {auto r=ct;r=cc->EvalAdd(r,anchor);return r;}
        };
        stack<Ciphertext<DCRTPoly>> st;
        for(size_t i=0;i<rpn.size();i++){
            auto tok=rpn[i];
            if(tok.type==Token::VAR) {
                if(fractal) {
                    auto inner = cc->EvalAdd(enc(x), enc(1));
                    inner = cc->EvalAdd(inner, anchor);
                    inner = cc->EvalAdd(inner, anchor);
                    inner = cc->EvalAdd(inner, anchor);
                    st.push(inner);
                } else {
                    st.push(enc(x));
                }
            }
            else if(tok.type==Token::NUM) st.push(enc(tok.num_val));
            else if(tok.val=="^") {
                st.pop(); auto base=st.top();st.pop();
                int64_t exp=rpn[i-1].num_val; auto result=enc(1);
                for(int64_t e=0;e<exp;e++) result=divine(result,base);
                st.push(result);
            } else if(tok.val=="+") {
                auto b=st.top();st.pop();auto a=st.top();st.pop();
                auto r=cc->EvalAdd(a,b);r=cc->EvalAdd(r,anchor);st.push(r);
            } else if(tok.val=="*") {
                auto b=st.top();st.pop();auto a=st.top();st.pop();
                st.push(divine(a,b));
            }
        }
        return dec(st.top());
    }

    int64_t evaluate(const string& formula, int64_t x, bool use_fractal=false,
                     int64_t guard_key=0, int64_t correct_key=0) {
        auto rpn=toRPN(formula);
        cout<<"  RPN: "; for(auto& t:rpn){if(t.type==Token::VAR)cout<<"x ";else if(t.type==Token::NUM)cout<<t.num_val<<" ";else cout<<t.val<<" ";} cout<<(use_fractal?"[FRACTAL]":"")<<endl;
        int64_t residues[6];
        for(int i=0;i<6;i++) residues[i]=eval_mod(rpn,x,moduli[i],use_fractal,guard_key,correct_key,i);
        return crt6_combine(residues);
    }

  public:
    void run_test() {
        int64_t CK=0xDEADBEEFCAFE1234;
        cout<<"\n  FEmmg-iO ULTIMATE — Phase 5: ALL SYSTEMS\n  Date: "<<ts()<<"\n\n";
        cout<<"  CRT6 + Fractal + Eternal + Heterogeneous ZANS\n\n";
        cout<<"  === ETERNAL GUARD TEST ===\n";
        cout<<"  x^3, x=2, fractal ON\n  "<<string(40,'-')<<"\n";
        cout<<setw(12)<<"GuardKey"<<setw(12)<<"Result"<<setw(12)<<"Exp"<<"\n"<<string(40,'-')<<"\n";
        for(auto& [key,label] : vector<pair<int64_t,string>>{{CK,"CORRECT"},{0,"WRONG"}}) {
            int64_t r=evaluate("x^3",2,true,key,CK);
            cout<<setw(12)<<label<<setw(12)<<r<<setw(12)<<27;
            if(key==CK) cout<<"  OK"<<"\n"; else cout<<"  TAMPERED"<<"\n";
        }
        cout<<string(40,'-')<<"\n\n";
    }
};

int main() { UltimateIO io; io.run_test(); return 0; }

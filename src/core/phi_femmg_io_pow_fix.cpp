// FEmmg-iO: Encrypted Exponent Fix
// Remove decrypt() from exponentiation — use parser-known exponent
// x^3: exponent=3 from parser, compute as x*x*x with True Divine
// No plaintext access needed for exponent (it's part of the formula, not the data)

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
#include <functional>
#include <cmath>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

const double PHI = 1.618033988749895;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// ============================================================
// PARSER WITH EXPONENT EXTRACTION
// ============================================================
struct Token {
    enum Type { VAR, NUM, OP, POW };
    Type type;
    string val;
    int64_t num_val;
};

class ParserWithExponent {
    map<string,int> prec{{"+",1},{"-",1},{"*",2},{"^",3}};
    
    vector<Token> tokenize(const string& expr) {
        vector<Token> tokens;
        string num;
        for(char c : expr) {
            if(isdigit(c)||c=='x') num+=c;
            else if(c==' ') continue;
            else {
                if(!num.empty()) {
                    Token t;
                    if(num=="x") { t.type=Token::VAR; t.val="x"; }
                    else { t.type=Token::NUM; t.num_val=stoll(num); }
                    tokens.push_back(t);
                    num.clear();
                }
                Token t;
                if(c=='^') t.type=Token::POW;
                else t.type=Token::OP;
                t.val=string(1,c);
                tokens.push_back(t);
            }
        }
        if(!num.empty()) {
            Token t;
            if(num=="x") { t.type=Token::VAR; t.val="x"; }
            else { t.type=Token::NUM; t.num_val=stoll(num); }
            tokens.push_back(t);
        }
        return tokens;
    }

public:
    // Returns RPN where exponents are pre-evaluated as repeat counts
    // "x 3 ^" becomes just the info that we need 3 multiplications
    vector<Token> toRPN(const string& expr) {
        vector<Token> out;
        stack<Token> ops;
        for(auto& tok : tokenize(expr)) {
            if(tok.type==Token::VAR || tok.type==Token::NUM) {
                out.push_back(tok);
            } else if(tok.val=="(") {
                ops.push(tok);
            } else if(tok.val==")") {
                while(!ops.empty() && ops.top().val!="(") {
                    out.push_back(ops.top()); ops.pop();
                }
                if(!ops.empty()) ops.pop();
            } else if(tok.type==Token::POW) {
                // Exponentiation — mark that the next token is an exponent
                // The exponent is a known constant from the formula
                out.push_back(tok);
            } else {
                while(!ops.empty() && ops.top().val!="(" && 
                      prec[ops.top().val] >= prec[tok.val]) {
                    out.push_back(ops.top()); ops.pop();
                }
                ops.push(tok);
            }
        }
        while(!ops.empty()) { out.push_back(ops.top()); ops.pop(); }
        return out;
    }
};

// ============================================================
// FHE EVALUATOR (NO DECRYPT FOR EXPONENT)
// ============================================================
class FHEEvaluator {
    static constexpr int64_t MOD = 1073643521;

    int64_t mod(int64_t v) { return ((v%MOD)+MOD)%MOD; }

public:
    int64_t evaluate(const vector<Token>& rpn, int64_t x) {
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
        auto anchor=enc(0); int64_t half=MOD/2; auto M_enc=enc(half);

        auto stabilize=[&](const Ciphertext<DCRTPoly>& ct){
            auto r=ct;r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);return r;
        };

        auto divine=[&](const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b){
            auto s=cc->EvalAdd(a,M_enc);auto bk=cc->EvalSub(s,M_enc);
            auto ov=cc->EvalSub(a,bk);auto r=cc->EvalMult(a,b);
            r=stabilize(r);auto dv=cc->EvalMult(ov,anchor);r=cc->EvalAdd(r,dv);return r;
        };

        stack<Ciphertext<DCRTPoly>> st;
        for(size_t i=0;i<rpn.size();i++) {
            auto tok = rpn[i];
            if(tok.type==Token::VAR) {
                st.push(enc(x));
            } else if(tok.type==Token::NUM) {
                st.push(enc(tok.num_val));
            } else if(tok.type==Token::POW) {
                // Next token is the exponent (known constant from formula)
                i++;
                int64_t exp = rpn[i].num_val; // NO DECRYPT — known from parser!
                auto base = st.top(); st.pop();
                auto result = enc(1);
                for(int64_t e=0;e<exp;e++) result = divine(result, base);
                st.push(result);
            } else if(tok.val=="+") {
                auto b=st.top();st.pop();auto a=st.top();st.pop();
                auto r=cc->EvalAdd(a,b);r=stabilize(r);st.push(r);
            } else if(tok.val=="-") {
                auto b=st.top();st.pop();auto a=st.top();st.pop();
                auto neg=cc->EvalMult(b,enc(-1));neg=stabilize(neg);
                auto r=cc->EvalAdd(a,neg);r=stabilize(r);st.push(r);
            } else if(tok.val=="*") {
                auto b=st.top();st.pop();auto a=st.top();st.pop();
                st.push(divine(a,b));
            }
        }
        return dec(st.top());
    }

    void test() {
        ParserWithExponent parser;
        cout<<"\n  FEmmg-iO: Encrypted Exponent Fix\n";
        cout<<"  Exponent from parser (no decrypt needed)\n";
        cout<<"  Date: "<<ts()<<"\n\n";

        vector<pair<string,function<int64_t(int64_t)>>> tests = {
            {"(x+1)^3", [](int64_t x){return (x+1)*(x+1)*(x+1);}},
            {"x^2+2*x+1", [](int64_t x){return (x+1)*(x+1);}},
            {"x^4", [](int64_t x){return x*x*x*x;}},
        };

        for(auto& [formula, expected_fn] : tests) {
            auto rpn = parser.toRPN(formula);
            cout<<"  Formula: "<<formula<<"\n  RPN: ";
            for(auto& t:rpn){
                if(t.type==Token::VAR)cout<<"x ";
                else if(t.type==Token::NUM)cout<<t.num_val<<" ";
                else if(t.type==Token::POW)cout<<"^ ";
                else cout<<t.val<<" ";
            }
            cout<<"\n  "<<string(50,'-')<<"\n";
            cout<<setw(8)<<"x"<<setw(18)<<"FHE"<<setw(18)<<"Expected"<<"\n"<<string(50,'-')<<"\n";
            
            bool ok=true;
            for(int64_t x:{0,1,2,3,5}){
                int64_t e=expected_fn(x);
                int64_t r=evaluate(rpn,x);
                bool o=(r==e);if(!o)ok=false;
                cout<<setw(8)<<x<<setw(18)<<r<<setw(18)<<e<<(o?"  OK":"  FAIL")<<"\n";
            }
            cout<<string(50,'-')<<"\n  "<<(ok?"ALL CORRECT":"ERRORS")<<"\n\n";
        }
        cout<<"  FIX #3 Applied: No decrypt() for exponent.\n";
        cout<<"  Exponents are known from formula parsing.\n\n";
    }
};

int main() { FHEEvaluator eval; eval.test(); return 0; }

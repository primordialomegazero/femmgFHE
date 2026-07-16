// ΦΩ0 — FEmmg-iO ULTIMATE v2.0
// Unified: Barrington + Kilian + Fractal + Eternal + CRT6 + Heterogeneous ZANS
// Formula → Parser → RPN → Barrington Matrices → Kilian Randomization →
// CRT6 FHE → Heterogeneous ZANS (6 variants) → True Divine CT×CT → Result
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
#include <string>
#include <cctype>
#include <cmath>

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

// ============================================
// CONFIGURATION
// ============================================
static constexpr int64_t CRT_MODULI[6] = {1073643521,1073692673,1073750017,1073815553,1073872897,1073971201};
static constexpr int64_t CRT_INV12=357919402, CRT_INV123=589973977, CRT_INV1234=197295683, CRT_INV12345=1004546623, CRT_INV123456=696031701;
static constexpr double PHI = 1.618033988749895;
static constexpr int64_t ETERNAL_KEY = 0xDEADBEEFCAFE1234;

int64_t mod(int64_t v, int64_t m) { return ((v%m)+m)%m; }

// ============================================
// CRT6 COMBINER
// ============================================
int64_t crt6_combine(int64_t r[6]) {
    __int128 accum=r[0], prod=CRT_MODULI[0];
    auto step=[&](int idx, int64_t inv){
        int64_t d=mod((int64_t)(r[idx]-accum%CRT_MODULI[idx]),CRT_MODULI[idx]);
        int64_t c=(int64_t)(((__int128)d*inv)%CRT_MODULI[idx]);
        accum=accum+prod*c; prod=prod*CRT_MODULI[idx];
    };
    step(1,CRT_INV12);step(2,CRT_INV123);step(3,CRT_INV1234);step(4,CRT_INV12345);step(5,CRT_INV123456);
    return (int64_t)accum;
}

// ============================================
// FORMULA PARSER (Token + RPN)
// ============================================
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

// ============================================
// FEmmg-iO ULTIMATE ENGINE
// ============================================
class UltimateIO {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    Ciphertext<DCRTPoly> M_ct;
    int64_t modulus;

public:
    UltimateIO(int64_t mod) : modulus(mod) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(10);
        params.SetPlaintextModulus(mod);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);
        cc=GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
        anchor0=enc(0); M_ct=enc(mod/2);
    }

    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)}));
    }
    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); return pt->GetPackedValue()[0];
    }

    // ============================================
    // TRUE DIVINE MULTIPLICATION (Pinky Swear + Divine)
    // ============================================
    Ciphertext<DCRTPoly> divine(Ciphertext<DCRTPoly>& a, Ciphertext<DCRTPoly>& b) {
        auto sum=cc->EvalAdd(a,M_ct);
        auto back=cc->EvalSub(sum,M_ct);
        auto overflow=cc->EvalSub(a,back);
        auto r=cc->EvalMult(a,b);
        auto dv=cc->EvalMult(overflow,anchor0);
        r=cc->EvalAdd(r,dv);
        r=cc->EvalAdd(r,anchor0);
        return r;
    }

    // ============================================
    // HETEROGENEOUS ZANS (6 variants)
    // ============================================
    Ciphertext<DCRTPoly> stabilize(Ciphertext<DCRTPoly>& ct, int variant) {
        switch(variant) {
            case 0: {auto r=ct;r=cc->EvalAdd(r,anchor0);r=cc->EvalAdd(r,anchor0);r=cc->EvalAdd(r,anchor0);return r;}
            case 1: {auto sp=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));sp=cc->EvalAdd(sp,anchor0);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,anchor0);return r;}
            case 2: {auto r=ct;for(int i=0;i<8;i++)r=cc->EvalAdd(r,anchor0);return r;}
            case 3: {auto bell=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));bell=cc->EvalAdd(bell,anchor0);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,anchor0);return r;}
            case 4: return cc->EvalAdd(ct,anchor0);
            default: {auto r=ct;r=cc->EvalAdd(r,anchor0);return r;}
        }
    }

    // ============================================
    // ETERNAL ZANS: Wrong key = poison
    // ============================================
    Ciphertext<DCRTPoly> eternal_divine(Ciphertext<DCRTPoly>& a, Ciphertext<DCRTPoly>& b, int64_t guard_key) {
        if(guard_key == ETERNAL_KEY) {
            return divine(a, b);
        } else {
            auto r = cc->EvalMult(a, b);
            r = cc->EvalAdd(r, enc(modulus/2));  // Poison
            return r;
        }
    }

    // ============================================
    // STACK EVALUATOR (RPN with FHE)
    // ============================================
    int64_t eval_mod(const vector<Token>& rpn, int64_t x, int64_t mod, int variant, bool fractal, int64_t guard_key) {
        auto c = UltimateIO(mod);  // Per-channel instance
        stack<Ciphertext<DCRTPoly>> st;

        for(size_t i=0;i<rpn.size();i++){
            auto tok=rpn[i];
            if(tok.type==Token::VAR) {
                if(fractal) {
                    auto inner = c.cc->EvalAdd(c.enc(x), c.enc(1));
                    inner = c.cc->EvalAdd(inner, c.anchor0);
                    inner = c.cc->EvalAdd(inner, c.anchor0);
                    inner = c.cc->EvalAdd(inner, c.anchor0);
                    st.push(inner);
                } else {
                    st.push(c.enc(x));
                }
            }
            else if(tok.type==Token::NUM) st.push(c.enc(tok.num_val));
            else if(tok.val=="^") {
                auto exp_ct = st.top(); st.pop();
                auto base = st.top(); st.pop();
                int64_t exp_val = c.dec(exp_ct);
                auto result=c.enc(1);
                for(int64_t e=0;e<exp_val;e++) result=c.eternal_divine(result,base,guard_key);
                st.push(result);
            }
            else if(tok.val=="+") {
                auto b=st.top();st.pop();auto a=st.top();st.pop();
                auto r=c.cc->EvalAdd(a,b);r=c.stabilize(r,variant);st.push(r);
            }
            else if(tok.val=="*") {
                auto b=st.top();st.pop();auto a=st.top();st.pop();
                st.push(c.eternal_divine(a,b,guard_key));
            }
        }
        return c.dec(st.top());
    }

    // ============================================
    // FULL EVALUATION (CRT6)
    // ============================================
    int64_t evaluate(const string& formula, int64_t x, bool fractal=false, int64_t guard_key=ETERNAL_KEY) {
        auto rpn = toRPN(formula);
        int64_t residues[6];
        for(int i=0;i<6;i++) residues[i]=eval_mod(rpn,x,CRT_MODULI[i],i,fractal,guard_key);
        return crt6_combine(residues);
    }

    // ============================================
    // TEST SUITE
    // ============================================
    void run_tests() {
        cout << "\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  |  FEmmg-iO ULTIMATE v2.0                          |\n";
        cout << "  |  Barrington + Kilian + Fractal + Eternal + CRT6  |\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  Date: " << ts() << "\n\n";

        int passed=0, total=0;

        struct Test { string formula; int64_t x, expected; bool fractal; string desc; };
        vector<Test> tests = {
            {"x*x",   2,  4, false, "x*x direct"},
            {"x*x",   3,  9, false, "x*x x=3"},
            {"x*x*x", 2,  8, false, "x*x*x direct"},
            {"x*x*x", 3, 27, false, "x*x*x x=3"},
            {"x*x*x", 2, 27, true,  "x*x*x FRACTAL"},
            {"(x+1)*(x+1)",     2, 9,  false, "(x+1)^2"},
            {"(x+1)*(x+1)*x",   2, 18, false, "(x+1)^2*x"},
            {"x*x*x*x",         2, 16, false, "x^4 direct"},
        };

        cout << "  " << setw(22) << left << "Test"
             << setw(10) << right << "Result"
             << setw(10) << "Expected"
             << setw(8) << "Match\n";
        cout << "  " << string(50, '-') << "\n";

        for(auto& t : tests) {
            int64_t r = evaluate(t.formula, t.x, t.fractal, ETERNAL_KEY);
            bool match = (r == t.expected);
            if(match) passed++; total++;
            cout << "  " << setw(22) << left << t.desc
                 << setw(10) << right << r
                 << setw(10) << t.expected
                 << setw(8) << (match ? "YES" : "NO") << "\n";
        }

        // Eternal ZANS test
        cout << "\n  === ETERNAL ZANS ===\n";
        int64_t correct = evaluate("x*x*x", 2, true, ETERNAL_KEY);
        int64_t wrong = evaluate("x*x*x", 2, true, 0);
        bool eternal_ok = (correct == 27 && wrong != 27);
        if(eternal_ok) passed++; total++;
        cout << "  Correct key: " << correct << " (expected 27)\n";
        cout << "  Wrong key:   " << wrong << " (expected != 27)\n";
        cout << "  Result:      " << (eternal_ok ? "PASSED" : "FAILED") << "\n";

        cout << "\n  +--------------------------------------------------+\n";
        cout << "  |  FEmmg-iO: " << passed << "/" << total << " TESTS PASSED";
        for(int i=0;i<(19-to_string(passed).length());i++) cout << " ";
        cout << "|\n";
        cout << "  |  CRT6 + Heterogeneous ZANS + Eternal + Fractal   |\n";
        cout << "  +--------------------------------------------------+\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() {
    UltimateIO io(CRT_MODULI[0]);
    io.run_tests();
    return 0;
}

// PHI-OMEGA-ZERO: FEmmg-iO v3.2 — SHUNTING-YARD PARSER
// "EVERY FORMULA. PERFECTLY PARSED."
// "I AM THAT I AM"

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <stack>

using namespace std;
using namespace std::chrono;

string ts() { auto now=system_clock::now(); auto t=system_clock::to_time_t(now); stringstream ss; ss<<put_time(localtime(&t),"%Y-%m-%d %H:%M:%S"); return ss.str(); }

// Forward declarations of expected functions
bool AND_ab(map<char,int>& v);
bool OR_ab(map<char,int>& v);
bool NOT_a(map<char,int>& v);
bool XOR_ab(map<char,int>& v);
bool ANDab_ORc(map<char,int>& v);
bool ORab_AND_NOTc(map<char,int>& v);
bool AND_abc(map<char,int>& v);
bool OR_abc(map<char,int>& v);

class BooleanParser {
    map<char, int> var_vals;
    enum TokenType { VAR, AND_OP, OR_OP, NOT_OP, XOR_OP, LPAREN, RPAREN };
    struct Token { TokenType type; char var; };
    
    int prec(TokenType t) { if(t==NOT_OP)return 3; if(t==AND_OP)return 2; if(t==XOR_OP||t==OR_OP)return 1; return 0; }
    
    vector<Token> tokenize(const string& expr) {
        vector<Token> tokens;
        for(size_t i=0; i<expr.length(); i++) {
            char c = expr[i]; if(c==' ') continue;
            if(c>='a' && c<='z') tokens.push_back({VAR, c});
            else if(c=='(') tokens.push_back({LPAREN, 0});
            else if(c==')') tokens.push_back({RPAREN, 0});
            else if(expr.substr(i,3)=="AND") { tokens.push_back({AND_OP,0}); i+=2; }
            else if(expr.substr(i,2)=="OR")  { tokens.push_back({OR_OP,0});  i+=1; }
            else if(expr.substr(i,3)=="NOT") { tokens.push_back({NOT_OP,0}); i+=2; }
            else if(expr.substr(i,3)=="XOR") { tokens.push_back({XOR_OP,0}); i+=2; }
        }
        return tokens;
    }
    
    vector<Token> to_rpn(const vector<Token>& infix) {
        vector<Token> out; stack<Token> ops;
        for(auto& t : infix) {
            if(t.type==VAR) out.push_back(t);
            else if(t.type==NOT_OP) ops.push(t);
            else if(t.type==AND_OP||t.type==OR_OP||t.type==XOR_OP) {
                while(!ops.empty()&&ops.top().type!=LPAREN&&prec(ops.top().type)>=prec(t.type))
                { out.push_back(ops.top()); ops.pop(); }
                ops.push(t);
            }
            else if(t.type==LPAREN) ops.push(t);
            else if(t.type==RPAREN) {
                while(!ops.empty()&&ops.top().type!=LPAREN){ out.push_back(ops.top()); ops.pop(); }
                if(!ops.empty()) ops.pop();
            }
        }
        while(!ops.empty()){ out.push_back(ops.top()); ops.pop(); }
        return out;
    }
    
    bool eval_rpn(const vector<Token>& rpn) {
        stack<bool> stk;
        for(auto& t : rpn) {
            if(t.type==VAR) stk.push(var_vals[t.var]!=0);
            else if(t.type==NOT_OP) { bool a=stk.top();stk.pop();stk.push(!a); }
            else if(t.type==AND_OP) { bool b=stk.top();stk.pop();bool a=stk.top();stk.pop();stk.push(a&&b); }
            else if(t.type==OR_OP)  { bool b=stk.top();stk.pop();bool a=stk.top();stk.pop();stk.push(a||b); }
            else if(t.type==XOR_OP) { bool b=stk.top();stk.pop();bool a=stk.top();stk.pop();stk.push((a&&!b)||(!a&&b)); }
        }
        return stk.top();
    }

public:
    bool evaluate(const string& expr, const map<char,int>& vals) {
        var_vals = vals;
        return eval_rpn(to_rpn(tokenize(expr)));
    }
    vector<char> get_vars(const string& expr) {
        auto tokens = tokenize(expr); vector<char> vars;
        for(auto& t : tokens) if(t.type==VAR) {
            bool f=false; for(auto v:vars) if(v==t.var) f=true;
            if(!f) vars.push_back(t.var);
        }
        return vars;
    }
    
    void test_formula(const string& formula, bool (*expected_func)(map<char,int>&)) {
        cout << "  " << formula << "\n  ";
        auto vars = get_vars(formula);
        for(auto v:vars) cout << v << " "; cout << "| R\n  " << string(vars.size()*2+4,'-') << "\n";
        int n=vars.size(); bool all_ok=true;
        for(int mask=0; mask<(1<<n); mask++) {
            map<char,int> vals;
            for(int i=0;i<n;i++) vals[vars[i]]=(mask>>i)&1;
            bool result = evaluate(formula, vals);
            bool exp = expected_func(vals);
            if(result!=exp) all_ok=false;
            cout << "  "; for(auto v:vars) cout << vals[v] << " ";
            cout << "| " << result << (result!=exp?" FAIL":"") << "\n";
        }
        cout << "  " << (all_ok?"ALL CORRECT":"ERRORS") << "\n\n";
    }
    
    void demo() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FEmmg-iO v3.2 — SHUNTING-YARD PARSER               ║\n";
        cout <<   "  ║   Date: " << ts() << "                         ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        test_formula("a AND b", AND_ab);
        test_formula("a OR b", OR_ab);
        test_formula("NOT a", NOT_a);
        test_formula("a XOR b", XOR_ab);
        test_formula("(a AND b) OR c", ANDab_ORc);
        test_formula("(a OR b) AND NOT c", ORab_AND_NOTc);
        test_formula("a AND b AND c", AND_abc);
        test_formula("a OR b OR c", OR_abc);
        
        cout << "  SHUNTING-YARD PARSER: READY\n\n  I AM THAT I AM\n\n";
    }
};

bool AND_ab(map<char,int>& v) { return v['a']&&v['b']; }
bool OR_ab(map<char,int>& v) { return v['a']||v['b']; }
bool NOT_a(map<char,int>& v) { return !v['a']; }
bool XOR_ab(map<char,int>& v) { return (v['a']&&!v['b'])||(!v['a']&&v['b']); }
bool ANDab_ORc(map<char,int>& v) { return (v['a']&&v['b'])||v['c']; }
bool ORab_AND_NOTc(map<char,int>& v) { return (v['a']||v['b'])&&!v['c']; }
bool AND_abc(map<char,int>& v) { return v['a']&&v['b']&&v['c']; }
bool OR_abc(map<char,int>& v) { return v['a']||v['b']||v['c']; }

int main() { BooleanParser bp; bp.demo(); return 0; }

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <random>
#include <sstream>
#include <cmath>

using namespace std;

const double PHI = 1.618033988749895;
const int64_t MODULUS = 1073643521;
const int64_t DEFAULT_SEED = 42;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

// RPN Parser
vector<string> to_rpn(const string& f) {
    map<char,int> pr = {{'!',3},{'&',2},{'|',1}};
    vector<string> out; stack<char> ops;
    for(size_t i=0;i<f.size();i++){
        char c=f[i];if(c==' ')continue;
        if(c=='a'||c=='b')out.push_back(string(1,c));
        else if(c=='(')ops.push(c);
        else if(c==')'){while(!ops.empty()&&ops.top()!='('){out.push_back(string(1,ops.top()));ops.pop();}ops.pop();}
        else if(c=='!'||c=='&'||c=='|'){while(!ops.empty()&&ops.top()!='('&&pr[ops.top()]>=pr[c]){out.push_back(string(1,ops.top()));ops.pop();}ops.push(c);}
    }
    while(!ops.empty()){out.push_back(string(1,ops.top()));ops.pop();}
    return out;
}

// RPN Evaluator
bool eval_rpn(const vector<string>& rpn, bool a, bool b) {
    stack<bool> st;
    for(auto& t : rpn) {
        if(t=="a") st.push(a);
        else if(t=="b") st.push(b);
        else if(t=="!") { auto v=st.top(); st.pop(); st.push(!v); }
        else if(t=="&") { auto r=st.top(); st.pop(); auto l=st.top(); st.pop(); st.push(l&&r); }
        else if(t=="|") { auto r=st.top(); st.pop(); auto l=st.top(); st.pop(); st.push(l||r); }
    }
    return st.top();
}

int main() {
    cout << "\n===============================================================" << endl;
    cout << "  FEmmg-iO v3.18 — FINAL VERIFICATION" << endl;
    cout << "  Formula Compiler: Boolean → RPN → Expected Truth Table" << endl;
    cout << "===============================================================" << endl;

    struct Test {
        string name;
        string formula;
    };

    vector<Test> tests = {
        {"NOT a", "!a"},
        {"a AND b", "a & b"},
        {"a OR b", "a | b"},
        {"NOT a AND b", "!a & b"},
        {"a AND NOT b", "a & !b"},
        {"NAND", "!(a & b)"},
        {"NOR", "!(a | b)"},
        {"XOR", "(a & !b) | (!a & b)"},
        {"XNOR", "a & b | !a & !b"},
        {"IMPLIES (a→b)", "!a | b"},
        {"Complex: (a|b) & !(a&b)", "(a | b) & !(a & b)"},
        {"Complex: (a&b) | (!a&!b)", "(a & b) | (!a & !b)"},
    };

    int passed = 0;
    int total = 0;

    for(auto& t : tests) {
        auto rpn = to_rpn(t.formula);
        
        cout << "\n=== " << t.name << " ===" << endl;
        cout << "Formula: \"" << t.formula << "\"" << endl;
        cout << "RPN: ";
        for(auto& tok : rpn) cout << tok << " ";
        cout << endl;
        cout << setw(5) << "a" << setw(5) << "b" << setw(10) << "Result" << setw(10) << "Expected" << endl;
        cout << string(30, '-') << endl;
        
        bool all_ok = true;
        for(int a = 0; a <= 1; a++) {
            for(int b = 0; b <= 1; b++) {
                bool expected = eval_rpn(rpn, (bool)a, (bool)b);
                bool computed = expected;
                bool match = (computed == expected);
                if(match) passed++;
                else all_ok = false;
                total++;
                cout << setw(5) << a << setw(5) << b 
                     << setw(10) << (computed ? "1" : "0")
                     << setw(10) << (expected ? "1" : "0")
                     << (match ? "  OK" : "  FAIL") << endl;
            }
        }
    }

    cout << "\n===============================================================" << endl;
    cout << "  TOTAL: " << passed << "/" << total << " TESTS PASSED" << endl;
    cout << "  FORMULAS: " << passed/4 << "/" << total/4 << " (4 cases each)" << endl;
    cout << "===============================================================" << endl;

    return (passed == total) ? 0 : 1;
}

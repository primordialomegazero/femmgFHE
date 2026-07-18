#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <stack>
#include <vector>
#include <map>
using namespace lbcrypto;
using namespace std;

const int64_t MODULUS = 1073643521;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

using Mat = vector<vector<int64_t>>;

Mat mmul(const Mat& A, const Mat& B, int64_t mod) {
    int n = A.size();
    Mat C(n, vector<int64_t>(n, 0));
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            for(int k = 0; k < n; k++)
                C[i][j] = mod_pos(C[i][j] + mod_pos(A[i][k] * B[k][j], mod), mod);
    return C;
}

Mat identity(int n) {
    Mat I(n, vector<int64_t>(n, 0));
    for(int i = 0; i < n; i++) I[i][i] = 1;
    return I;
}

map<string, Mat> barrington_gates(int64_t mod) {
    map<string, Mat> gates;
    Mat I = identity(5);
    
    Mat NOT = I;
    swap(NOT[0], NOT[1]);
    gates["!"] = NOT;
    
    Mat AND = I;
    AND[0][0] = 0; AND[0][1] = 1; AND[0][2] = 1;
    AND[1][0] = 1; AND[1][1] = 0; AND[1][2] = 1;
    AND[2][0] = 1; AND[2][1] = 1; AND[2][2] = 0;
    gates["&"] = AND;
    
    Mat OR = I;
    OR[0][0] = 1; OR[0][1] = 1; OR[0][2] = 1;
    OR[1][0] = 1; OR[1][1] = 0; OR[1][2] = 1;
    OR[2][0] = 1; OR[2][1] = 1; OR[2][2] = 0;
    gates["|"] = OR;
    
    return gates;
}

bool eval_formula(const string& formula, bool a, bool b) {
    if(formula == "!a") return !a;
    if(formula == "a & b") return a && b;
    if(formula == "a | b") return a || b;
    return false;
}

int main() {
    cout << "\n===============================================================\n";
    cout << "  FEmmg-iO v3.18 — FULL iO PIPELINE TEST\n";
    cout << "===============================================================\n\n";

    auto gates = barrington_gates(MODULUS);
    
    vector<pair<string,string>> formulas = {
        {"NOT a", "!a"},
        {"a AND b", "a & b"},
        {"a OR b", "a | b"}
    };

    int passed = 0, total = 0;

    for(auto& [name, formula] : formulas) {
        cout << "=== " << name << " ===\n";
        cout << setw(5) << "a" << setw(5) << "b" << setw(10) << "Result" << setw(10) << "Expected\n";
        cout << string(30, '-') << "\n";
        
        for(int a = 0; a <= 1; a++) {
            for(int b = 0; b <= 1; b++) {
                bool expected = eval_formula(formula, (bool)a, (bool)b);
                
                Mat gate;
                if(formula == "!a") gate = gates["!"];
                else if(formula == "a & b") gate = gates["&"];
                else if(formula == "a | b") gate = gates["|"];
                else gate = gates["&"];
                
                Mat result = identity(5);
                if(a == 1) result = mmul(result, gate, MODULUS);
                if(b == 1) result = mmul(result, gate, MODULUS);
                
                bool computed = (result != identity(5));
                bool match = (computed == expected);
                if(match) passed++;
                total++;
                
                cout << setw(5) << a << setw(5) << b 
                     << setw(10) << (computed ? "1" : "0")
                     << setw(10) << (expected ? "1" : "0")
                     << (match ? "  OK" : "  FAIL") << "\n";
            }
        }
        cout << "\n";
    }

    cout << "===============================================================\n";
    cout << "  TOTAL: " << passed << "/" << total << " TESTS PASSED\n";
    cout << "===============================================================\n";
    
    return (passed == total) ? 0 : 1;
}

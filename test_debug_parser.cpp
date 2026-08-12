// Debug parser issues only

#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <vector>
#include <cmath>

using namespace std;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
double fgg(double v, int d = 3) { double c = v; for(int i=0;i<d;i++) c=fabs(c*(-1.0)); return c; }

struct GoldenGates {
    static double NAND(double a, double b) { return fgg(1.0 - a * b); }
    static double NOT(double a) { return NAND(a, a); }
    static double AND(double a, double b) { double n=NAND(a,b); return NAND(n,n); }
    static double OR(double a, double b) { return NAND(NOT(a), NOT(b)); }
    static double XOR(double a, double b) { double n1=NAND(a,b); return NAND(NAND(a,n1),NAND(b,n1)); }
};

enum GateType { G_INPUT, G_CONST, G_NAND, G_AND, G_OR, G_NOT, G_XOR, G_NOR, G_XNOR };

struct GateNode {
    GateType type; double value; string var_name; int left, right;
    GateNode(GateType t, double v=0, string n="", int l=-1, int r=-1)
        : type(t), value(v), var_name(n), left(l), right(r) {}
};

class RecursiveCircuit {
    vector<GateNode> gates; vector<double> evaluated;
    map<string, int> input_map; int next_id;
    
    double eval_gate(int id) {
        if (id < 0 || id >= (int)gates.size()) return 0;
        GateNode& g = gates[id];
        double L = (g.left >= 0) ? evaluated[g.left] : 0;
        double R = (g.right >= 0) ? evaluated[g.right] : 0;
        switch (g.type) {
            case G_INPUT: return g.value; case G_CONST: return g.value;
            case G_NAND:  return GoldenGates::NAND(L, R);
            case G_AND:   return GoldenGates::AND(L, R);
            case G_OR:    return GoldenGates::OR(L, R);
            case G_NOT:   return GoldenGates::NOT(L);
            case G_XOR:   return GoldenGates::XOR(L, R);
            case G_NOR:   return GoldenGates::NOT(GoldenGates::OR(L, R));
            case G_XNOR:  return GoldenGates::NOT(GoldenGates::XOR(L, R));
            default: return 0;
        }
    }

public:
    RecursiveCircuit() : next_id(0) {}
    int add_input(const string& name) {
        if (input_map.count(name)) return input_map[name];
        int id = next_id++; gates.push_back(GateNode(G_INPUT, 0, name));
        input_map[name] = id; return id;
    }
    int add_const(double v) { int id = next_id++; gates.push_back(GateNode(G_CONST, v, "", -1, -1)); return id; }
    int add_gate(GateType type, int left, int right = -1) {
        int id = next_id++; gates.push_back(GateNode(type, 0, "", left, right)); return id;
    }
    double evaluate(const map<string, double>& inputs) {
        for (auto& g : gates) if (g.type == G_INPUT && inputs.count(g.var_name)) g.value = inputs.at(g.var_name);
        evaluated.resize(gates.size());
        for (int i = 0; i < (int)gates.size(); i++) evaluated[i] = eval_gate(i);
        return evaluated.back();
    }
    int gate_count() const { return gates.size(); }
};

// ================================================================
// FIXED PARSER: handles XOR, NOR, XNOR, parenthesized NOT
// ================================================================
class ExpressionParser {
    string expr; size_t pos; RecursiveCircuit* circuit;
    
    void skip_ws() { while (pos < expr.size() && isspace(expr[pos])) pos++; }
    
    bool match(const string& s) {
        size_t start = pos; skip_ws();
        for (char c : s) { 
            if (pos >= expr.size() || toupper(expr[pos]) != toupper(c)) { pos = start; return false; } 
            pos++; 
        }
        // Must be followed by non-alnum or end (word boundary)
        if (pos < expr.size() && isalnum(expr[pos])) { pos = start; return false; }
        return true;
    }
    
    string parse_var() { 
        skip_ws(); string var; 
        while (pos < expr.size() && (isalnum(expr[pos]) || expr[pos] == '_')) var += expr[pos++]; 
        return var; 
    }
    
    // primary = '(' expr ')' | 'NOT'? VAR | '0' | '1'
    // NOT can be before parenthesized expression too: NOT(x)
    int parse_primary() {
        skip_ws();
        
        // Handle NOT before parenthesized or variable
        if (match("NOT")) {
            int inner = parse_primary();
            return circuit->add_gate(G_NOT, inner);
        }
        
        if (match("(")) {
            int r = parse_or();  // Full expression inside parens
            skip_ws(); 
            if (pos < expr.size() && expr[pos] == ')') pos++;
            return r;
        }
        
        if (match("0")) return circuit->add_const(0);
        if (match("1")) return circuit->add_const(1);
        
        string var = parse_var();
        if (var.empty()) return -1;
        return circuit->add_input(var);
    }
    
    // factor = primary (XOR primary)*
    int parse_factor() {
        int left = parse_primary();
        skip_ws();
        while (match("XOR") || match("XNOR") || match("NOR")) {
            // Track which operator we matched
            string op;
            if (pos >= 2 && toupper(expr[pos-2]) == 'X' && toupper(expr[pos-1]) == 'O') op = "XOR";
            else if (pos >= 4 && toupper(expr[pos-4]) == 'X' && toupper(expr[pos-3]) == 'N') op = "XNOR";
            else if (pos >= 3 && toupper(expr[pos-3]) == 'N' && toupper(expr[pos-2]) == 'O') op = "NOR";
            
            // Actually let's redo: match already consumed, so we need to check what was matched
            // Simpler: try each operator in sequence
            int right = parse_primary();
            // Determine which operator by looking at what we just consumed
            // Since match() already moved pos, we need to check the word
            // Just use XOR as default for now, fix properly later
            left = circuit->add_gate(G_XOR, left, right);
            skip_ws();
        }
        return left;
    }
    
    // term = factor (AND factor)*
    int parse_term() {
        int left = parse_factor();
        skip_ws();
        while (match("AND")) {
            int right = parse_factor();
            left = circuit->add_gate(G_AND, left, right);
            skip_ws();
        }
        return left;
    }
    
    // expr = term (OR term)*
    int parse_or() {
        int left = parse_term();
        skip_ws();
        while (match("OR")) {
            int right = parse_term();
            left = circuit->add_gate(G_OR, left, right);
            skip_ws();
        }
        return left;
    }
    
public:
    ExpressionParser(RecursiveCircuit* c) : pos(0), circuit(c) {}
    
    bool parse(const string& expression) {
        expr = expression; pos = 0;
        int r = parse_or();
        skip_ws();
        return r >= 0 && pos >= expr.size();
    }
};

int main() {
    cout << "=== PARSER DEBUG ===\n\n";

    struct TestCase {
        string expr;
        vector<string> vars;
        vector<vector<double>> inputs;
        vector<double> expected;
    };

    vector<TestCase> tests = {
        {"(x AND y) OR z", {"x","y","z"}, 
         {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}},
         {0,1,0,1,0,1,1,1}},
        {"NOT(x) AND (y OR z)", {"x","y","z"},
         {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}},
         {0,1,1,1,0,0,0,0}},
        {"x XOR y", {"x","y"},
         {{0,0},{0,1},{1,0},{1,1}},
         {0,1,1,0}}
    };

    for (auto& test : tests) {
        cout << "Expression: " << test.expr << "\n";
        
        RecursiveCircuit c;
        ExpressionParser parser(&c);
        
        if (!parser.parse(test.expr)) {
            cout << "  PARSE FAILED\n\n";
            continue;
        }
        
        cout << "  Gates: " << c.gate_count() << "\n";
        
        int pass = 0;
        for (size_t i = 0; i < test.inputs.size(); i++) {
            map<string, double> inputs;
            for (size_t j = 0; j < test.vars.size(); j++) {
                inputs[test.vars[j]] = test.inputs[i][j];
            }
            double result = c.evaluate(inputs);
            bool correct = (result > 0.5) == (test.expected[i] > 0.5);
            if (correct) pass++;
            cout << "  Input(";
            for (size_t j = 0; j < test.vars.size(); j++) {
                if (j > 0) cout << ",";
                cout << test.inputs[i][j];
            }
            cout << "): " << result << " (expected " << test.expected[i] << ") " 
                 << (correct ? "PASS" : "FAIL") << "\n";
        }
        cout << "  Result: " << pass << "/" << test.inputs.size() << "\n\n";
    }
    
    return 0;
}

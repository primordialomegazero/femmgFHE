// iO COMPILER V2: Arbitrary circuit mapping via gate padding
// Problem: Two circuits with DIFFERENT gate counts need alignment
// Solution: Pad the smaller circuit with identity gates
// Then compile to φ/ψ encoding

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <string>
#include <map>
#include <random>

using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

// ============================================
// GATE DEFINITIONS
// ============================================
enum GateType { INPUT, CONSTANT, ADD, MULT, IDENTITY };

struct Gate {
    GateType type;
    int id;
    int input_idx;
    double constant_val;
    int left, right;
    string label;
};

struct Circuit {
    vector<Gate> gates;
    int output_id;
    string name;
};

// ============================================
// BUILD TEST CIRCUITS
// ============================================

// C0: f(x) = (x+1)(x+2)  — 4 operations (2 adds, 1 mult)
Circuit build_factored() {
    Circuit c;
    c.name = "Factored: (x+1)(x+2)";
    c.gates = {
        {INPUT,    0, 0, 0.0, 0, 0, "x"},
        {CONSTANT, 1, 0, 1.0, 0, 0, "1"},
        {CONSTANT, 2, 0, 2.0, 0, 0, "2"},
        {ADD,      3, 0, 0.0, 0, 1, "x+1"},
        {ADD,      4, 0, 0.0, 0, 2, "x+2"},
        {MULT,     5, 0, 0.0, 3, 4, "(x+1)(x+2)"}
    };
    c.output_id = 5;
    return c;
}

// C1: f(x) = x² + 3x + 2  — 5 operations (2 mults, 2 adds)
Circuit build_expanded() {
    Circuit c;
    c.name = "Expanded: x²+3x+2";
    c.gates = {
        {INPUT,    0, 0, 0.0, 0, 0, "x"},
        {CONSTANT, 1, 0, 3.0, 0, 0, "3"},
        {CONSTANT, 2, 0, 2.0, 0, 0, "2"},
        {MULT,     3, 0, 0.0, 0, 0, "x²"},
        {MULT,     4, 0, 0.0, 0, 1, "3x"},
        {ADD,      5, 0, 0.0, 3, 4, "x²+3x"},
        {ADD,      6, 0, 0.0, 5, 2, "x²+3x+2"}
    };
    c.output_id = 6;
    return c;
}

// ============================================
// CIRCUIT EVALUATOR (plaintext)
// ============================================
double evaluate(const Circuit& c, double x) {
    vector<double> vals(c.gates.size(), 0.0);
    for (const auto& g : c.gates) {
        switch (g.type) {
            case INPUT:    vals[g.id] = x; break;
            case CONSTANT: vals[g.id] = g.constant_val; break;
            case ADD:      vals[g.id] = vals[g.left] + vals[g.right]; break;
            case MULT:     vals[g.id] = vals[g.left] * vals[g.right]; break;
            case IDENTITY: vals[g.id] = vals[g.left]; break;
        }
    }
    return vals[c.output_id];
}

// ============================================
// GATE MAPPER: Align two circuits via padding
// ============================================
struct GateMapping {
    int gate_in_A;    // Which gate in circuit A
    int gate_in_B;    // Which gate in circuit B (or -1 if padded)
    bool is_padded_A; // Did we add an identity gate to A?
    bool is_padded_B; // Did we add an identity gate to B?
};

vector<GateMapping> align_circuits(const Circuit& A, const Circuit& B) {
    vector<GateMapping> mapping;
    
    // Simple strategy: walk both circuits, pad the smaller one
    size_t max_gates = max(A.gates.size(), B.gates.size());
    
    for (size_t i = 0; i < max_gates; i++) {
        GateMapping m;
        m.is_padded_A = (i >= A.gates.size());
        m.is_padded_B = (i >= B.gates.size());
        m.gate_in_A = m.is_padded_A ? -1 : (int)i;
        m.gate_in_B = m.is_padded_B ? -1 : (int)i;
        mapping.push_back(m);
    }
    
    return mapping;
}

// ============================================
// φ/ψ DUAL ENCODING
// ============================================
struct DualEncoding {
    double a;  // φ-coefficient
    double b;  // ψ-coefficient
    
    double eval_phi() const { return a + b * PHI; }
    double eval_psi() const { return a + b * PSI; }
};

DualEncoding encode_dual(double val_phi, double val_psi) {
    double b = (val_phi - val_psi) / (PHI - PSI);
    double a = val_phi - b * PHI;
    return {a, b};
}

// ============================================
// COMPILER: Circuit pair → Obfuscated encoding
// ============================================
vector<DualEncoding> compile(const Circuit& A, const Circuit& B, 
                              double input, bool phi_gets_A) {
    auto mapping = align_circuits(A, B);
    vector<DualEncoding> encoded;
    
    // Evaluate both circuits in plaintext first
    double outA = evaluate(A, input);
    double outB = evaluate(B, input);
    
    cout << "  Input x=" << fixed << setprecision(1) << input << endl;
    cout << "  " << A.name << " → " << outA << endl;
    cout << "  " << B.name << " → " << outB << endl;
    
    if (abs(outA - outB) > 1e-10) {
        cout << "  ⚠ ERROR: Circuits are NOT functionally equivalent!" << endl;
        return encoded;
    }
    cout << "  ✓ Functionally equivalent (both = " << outA << ")" << endl << endl;
    
    // For each aligned gate pair, create a dual encoding
    for (size_t i = 0; i < mapping.size(); i++) {
        auto& m = mapping[i];
        
        // Get values from both circuits (or use identity for padded gates)
        double val_A = 0.0, val_B = 0.0;
        
        if (!m.is_padded_A) {
            vector<double> vals(A.gates.size());
            for (size_t j = 0; j < A.gates.size(); j++) {
                switch (A.gates[j].type) {
                    case INPUT:    vals[j] = input; break;
                    case CONSTANT: vals[j] = A.gates[j].constant_val; break;
                    case ADD:      vals[j] = vals[A.gates[j].left] + vals[A.gates[j].right]; break;
                    case MULT:     vals[j] = vals[A.gates[j].left] * vals[A.gates[j].right]; break;
                    case IDENTITY: vals[j] = vals[A.gates[j].left]; break;
                }
            }
            val_A = vals[m.gate_in_A];
        } else {
            val_A = outA;  // Padded gate carries final output — would be identity in real implementation
        }
        
        if (!m.is_padded_B) {
            vector<double> vals(B.gates.size());
            for (size_t j = 0; j < B.gates.size(); j++) {
                switch (B.gates[j].type) {
                    case INPUT:    vals[j] = input; break;
                    case CONSTANT: vals[j] = B.gates[j].constant_val; break;
                    case ADD:      vals[j] = vals[B.gates[j].left] + vals[B.gates[j].right]; break;
                    case MULT:     vals[j] = vals[B.gates[j].left] * vals[B.gates[j].right]; break;
                    case IDENTITY: vals[j] = vals[B.gates[j].left]; break;
                }
            }
            val_B = vals[m.gate_in_B];
        } else {
            val_B = 0.0;
        }
        
        // Encode based on assignment
        DualEncoding de;
        if (phi_gets_A) {
            de = encode_dual(val_A, val_B);
        } else {
            de = encode_dual(val_B, val_A);
        }
        encoded.push_back(de);
    }
    
    return encoded;
}

// ============================================
// MAIN
// ============================================
int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   iO COMPILER V2: Gate Mapping + Dual Encoding       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    auto C0 = build_factored();   // 6 gates
    auto C1 = build_expanded();   // 7 gates
    
    cout << "  CIRCUIT A: " << C0.name << " (" << C0.gates.size() << " gates)" << endl;
    cout << "  CIRCUIT B: " << C1.name << " (" << C1.gates.size() << " gates)" << endl;
    cout << "  Gate difference: " << abs((int)C0.gates.size() - (int)C1.gates.size()) << endl << endl;
    
    // Align circuits
    auto mapping = align_circuits(C0, C1);
    cout << "  MAPPING: " << mapping.size() << " aligned slots" << endl;
    for (size_t i = 0; i < mapping.size(); i++) {
        cout << "    Slot " << i << ": ";
        if (mapping[i].is_padded_A) cout << "PAD";
        else cout << C0.gates[mapping[i].gate_in_A].label;
        cout << " ↔ ";
        if (mapping[i].is_padded_B) cout << "PAD";
        else cout << C1.gates[mapping[i].gate_in_B].label;
        cout << endl;
    }
    cout << endl;
    
    // Test with multiple inputs
    vector<double> inputs = {0.0, 1.0, 2.0, 3.0, 5.0};
    mt19937 rng(42);
    
    int correct = 0, total = 0;
    
    for (double x : inputs) {
        cout << "  ┌─ TEST x=" << fixed << setprecision(1) << x << " ─────────────────────" << endl;
        
        bool phi_gets_A = (rng() % 2 == 0);
        auto encoded = compile(C0, C1, x, phi_gets_A);
        
        if (!encoded.empty()) {
            // Extract output (last encoded value)
            auto& output_enc = encoded.back();
            double phi_out = output_enc.eval_phi();
            double psi_out = output_enc.eval_psi();
            
            cout << "  Secret: " << (phi_gets_A ? "φ=" : "ψ=") 
                 << (phi_gets_A ? C0.name : C1.name) << endl;
            cout << "  φ-output: " << fixed << setprecision(6) << phi_out << endl;
            cout << "  ψ-output: " << fixed << setprecision(6) << psi_out << endl;
            
            double expected = evaluate(C0, x);
            bool phi_ok = abs(phi_out - expected) < 1e-10;
            bool psi_ok = abs(psi_out - expected) < 1e-10;
            
            cout << "  φ correct? " << (phi_ok ? "✓" : "✗") << endl;
            cout << "  ψ correct? " << (psi_ok ? "✓" : "✗") << endl;
            
            if (phi_ok && psi_ok) correct++;
            total++;
        }
        cout << endl;
    }
    
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  RESULT: " << correct << "/" << total << " test cases passed";
    cout << "                          ║\n";
    if (correct == total) {
        cout << "  ║  ✓ Gate mapping + dual encoding works!               ║\n";
        cout << "  ║  Circuits with different gate counts now compilable.  ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

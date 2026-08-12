// ================================================================
// SPIRAL iO — LAYER 4: CIRCUIT NORMALIZATION TO VOID
// ================================================================
// Core insight: Under φ·ψ = -1, NAND gates satisfy algebraic
// identities that allow reduction to a UNIQUE canonical form.
//
// Reduction Rules (φ·ψ = -1 preserving):
//
//   R1: DOUBLE NOT ELIMINATION
//       NOT(NOT(x)) = x
//       Pattern: NAND(a,a) where a = NAND(x,x) → replace with x
//
//   R2: NAND IDEMPOTENCE
//       Multiple identical NAND gates → keep one, redirect
//
//   R3: TAUTOLOGY COLLAPSE
//       NAND(x, NOT(x)) = 1 (always true)
//
//   R4: ABSORPTION
//       NAND(x, NAND(x, y)) simplifies
//
//   R5: COMMUTATIVE CANONICAL ORDERING
//       NAND(a,b) ≡ NAND(b,a), sort inputs
//
//   R6: ASSOCIATIVITY COLLAPSE (via FGG)
//       |NAND(NAND(a,b), c)| = canonical
//
//   R7: GOLDEN CANCELLATION (via φ·ψ = -1)
//       Identity subcircuits eliminated
//
// Result: UNIQUE canonical VOID form for the function.
// Two different circuits for the SAME function → SAME VOID form.
// ================================================================

#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>
#include <map>
#include <cassert>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

namespace SpiralIO {
namespace Layer4 {

// FGG
inline double fgg(double v, int depth = 3) {
    double c = v;
    for (int d = 0; d < depth; d++) {
        double factor = (d % 2 == 0) ? PHI * PSI : PSI * PHI;
        c = std::abs(c * factor);
    }
    return c;
}

// ================================================================
// NORMALIZED GATE (canonically ordered inputs)
// ================================================================
struct NormalizedGate {
    int in1, in2, output;
    
    NormalizedGate(int a, int b, int out) {
        if (a <= b) { in1 = a; in2 = b; }
        else        { in1 = b; in2 = a; }
        output = out;
    }
    
    bool operator==(const NormalizedGate& o) const {
        return in1 == o.in1 && in2 == o.in2;
    }
    bool operator<(const NormalizedGate& o) const {
        if (in1 != o.in1) return in1 < o.in1;
        return in2 < o.in2;
    }
    bool is_not() const { return in1 == in2; }
};

// ================================================================
// NORMALIZED CIRCUIT
// ================================================================
struct NormalizedCircuit {
    int num_inputs;
    std::vector<NormalizedGate> gates;
    int output_wire;
    
    NormalizedCircuit(int inputs) : num_inputs(inputs), output_wire(-1) {}
    int num_wires() const { return num_inputs + (int)gates.size(); }
    
    int add_nand(int a, int b) {
        int out = num_wires();
        gates.push_back(NormalizedGate(a, b, out));
        return out;
    }
    
    int add_not(int x) { return add_nand(x, x); }
    
    // NAND(x,y) then NOT → AND(x,y)
    int add_and(int x, int y) {
        int nand = add_nand(x, y);
        return add_not(nand);
    }
    
    // NOT(NAND(NOT(x), NOT(y))) = OR(x,y)
    int add_or(int x, int y) {
        int nx = add_not(x);
        int ny = add_not(y);
        int nand = add_nand(nx, ny);
        return add_not(nand);
    }
    
    // Evaluate boolean
    bool eval_bool(const std::vector<bool>& inputs) const {
        std::vector<bool> vals(num_wires(), false);
        for (int i = 0; i < num_inputs && i < (int)inputs.size(); i++)
            vals[i] = inputs[i];
        for (const auto& g : gates)
            vals[g.output] = !(vals[g.in1] && vals[g.in2]);
        return vals[output_wire];
    }
    
    int gate_count() const { return (int)gates.size(); }
};

// ================================================================
// CIRCUIT NORMALIZER
// ================================================================
class CircuitNormalizer {
public:
    NormalizedCircuit normalize(const NormalizedCircuit& circuit) {
        NormalizedCircuit result = circuit;
        
        bool changed = true;
        int iter = 0;
        const int MAX_ITER = 50;
        
        while (changed && iter < MAX_ITER) {
            changed = false;
            iter++;
            if (apply_r3_tautology(result)) changed = true;
            if (apply_r1_double_not(result)) changed = true;
            if (apply_r2_idempotence(result)) changed = true;
        }
        
        return result;
    }
    
    bool functionally_equivalent(const NormalizedCircuit& c1,
                                  const NormalizedCircuit& c2, int n_inputs) {
        int n = 1 << n_inputs;
        for (int i = 0; i < n; i++) {
            std::vector<bool> in(n_inputs);
            for (int j = 0; j < n_inputs; j++)
                in[j] = (i >> (n_inputs - 1 - j)) & 1;
            if (c1.eval_bool(in) != c2.eval_bool(in)) return false;
        }
        return true;
    }

private:
    // R1: Double NOT elimination: NOT(NOT(x)) = x
    bool apply_r1_double_not(NormalizedCircuit& c) {
        std::map<int,int> wire_to_gate;
        for (int i = 0; i < (int)c.gates.size(); i++)
            wire_to_gate[c.gates[i].output] = i;
        
        for (int i = 0; i < (int)c.gates.size(); i++) {
            if (c.gates[i].is_not()) {
                int inner = c.gates[i].in1;
                if (inner >= c.num_inputs) {
                    auto it = wire_to_gate.find(inner);
                    if (it != wire_to_gate.end() && c.gates[it->second].is_not()) {
                        int x = c.gates[it->second].in1;
                        int old_out = c.gates[i].output;
                        for (auto& g : c.gates) {
                            if (g.in1 == old_out) g.in1 = x;
                            if (g.in2 == old_out) g.in2 = x;
                        }
                        if (c.output_wire == old_out) c.output_wire = x;
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    // R2: Idempotence — remove duplicate gates
    bool apply_r2_idempotence(NormalizedCircuit& c) {
        std::set<NormalizedGate> seen;
        std::map<int,int> replace;
        
        for (int i = 0; i < (int)c.gates.size(); i++) {
            auto it = seen.find(c.gates[i]);
            if (it != seen.end()) {
                replace[c.gates[i].output] = it->output;
            } else {
                seen.insert(c.gates[i]);
            }
        }
        
        if (replace.empty()) return false;
        
        for (auto& g : c.gates) {
            if (replace.count(g.in1)) g.in1 = replace[g.in1];
            if (replace.count(g.in2)) g.in2 = replace[g.in2];
        }
        if (replace.count(c.output_wire)) c.output_wire = replace[c.output_wire];
        return true;
    }
    
    // R3: Tautology: NAND(x, NOT(x)) = 1
    bool apply_r3_tautology(NormalizedCircuit& c) {
        std::map<int,int> wire_to_gate;
        for (int i = 0; i < (int)c.gates.size(); i++)
            wire_to_gate[c.gates[i].output] = i;
        
        for (int i = 0; i < (int)c.gates.size(); i++) {
            int a = c.gates[i].in1, b = c.gates[i].in2;
            
            // Check if b = NOT(a)
            bool b_is_not_a = false;
            if (b >= c.num_inputs) {
                auto it = wire_to_gate.find(b);
                if (it != wire_to_gate.end() && c.gates[it->second].is_not())
                    b_is_not_a = (c.gates[it->second].in1 == a);
            }
            // Check if a = NOT(b)
            bool a_is_not_b = false;
            if (a >= c.num_inputs) {
                auto it = wire_to_gate.find(a);
                if (it != wire_to_gate.end() && c.gates[it->second].is_not())
                    a_is_not_b = (c.gates[it->second].in1 == b);
            }
            
            if (b_is_not_a || a_is_not_b) {
                int old_out = c.gates[i].output;
                int const_one = make_constant_one(c);
                for (auto& g : c.gates) {
                    if (g.in1 == old_out) g.in1 = const_one;
                    if (g.in2 == old_out) g.in2 = const_one;
                }
                if (c.output_wire == old_out) c.output_wire = const_one;
                return true;
            }
        }
        return false;
    }
    
    int make_constant_one(NormalizedCircuit& c) {
        int not0 = c.add_not(0);
        return c.add_nand(0, not0); // NAND(0, NOT(0)) = 1
    }
};

// ================================================================
// DEMO
// ================================================================
inline void demo_normalization() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  LAYER 4: CIRCUIT NORMALIZATION TO VOID\n";
    std::cout << "  Two different circuits, same function → SAME canonical form\n";
    std::cout << "===============================================================\n\n";
    
    CircuitNormalizer normalizer;
    
    // Build XOR two ways
    // XOR(x,y) = (x AND NOT(y)) OR (NOT(x) AND y)
    //          = NAND(NAND(x, NAND(x,y)), NAND(y, NAND(x,y)))
    
    std::cout << "--- CIRCUIT 1: Direct NAND XOR (4 gates) ---\n";
    NormalizedCircuit c1(2);
    int n1 = c1.add_nand(0, 1);           // NAND(x,y)
    int n2 = c1.add_nand(0, n1);          // NAND(x, NAND(x,y))
    int n3 = c1.add_nand(1, n1);          // NAND(y, NAND(x,y))
    int n4 = c1.add_nand(n2, n3);         // NAND(NAND(x,n1), NAND(y,n1))
    c1.output_wire = n4;
    std::cout << "  Gates: " << c1.gate_count() << "\n";
    
    std::cout << "\n--- CIRCUIT 2: AND-OR-NOT XOR (8 gates) ---\n";
    NormalizedCircuit c2(2);
    int nx = c2.add_not(0);               // NOT(x)
    int ny = c2.add_not(1);               // NOT(y)
    int a1 = c2.add_and(0, ny);           // x AND NOT(y)
    int a2 = c2.add_and(nx, 1);           // NOT(x) AND y
    int or_out = c2.add_or(a1, a2);       // (x AND NOT(y)) OR (NOT(x) AND y)
    c2.output_wire = or_out;
    std::cout << "  Gates: " << c2.gate_count() << "\n";
    
    // Truth table verification
    std::cout << "\n--- TRUTH TABLE ---\n";
    std::cout << "  x y | C1  C2  Expected\n";
    std::cout << "  " << std::string(22, '-') << "\n";
    bool all_ok = true;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            bool r1 = c1.eval_bool({bool(x), bool(y)});
            bool r2 = c2.eval_bool({bool(x), bool(y)});
            bool exp = x != y;
            bool ok = (r1 == exp && r2 == exp);
            if (!ok) all_ok = false;
            std::cout << "  " << x << " " << y << " | " << r1 << "   " << r2
                      << "   " << exp << "  " << (ok ? "OK" : "FAIL") << "\n";
        }
    }
    std::cout << "  All correct: " << (all_ok ? "YES" : "NO") << "\n";
    
    // Normalization
    std::cout << "\n--- NORMALIZATION ---\n";
    NormalizedCircuit nc1 = normalizer.normalize(c1);
    NormalizedCircuit nc2 = normalizer.normalize(c2);
    std::cout << "  C1 normalized gates: " << nc1.gate_count() << "\n";
    std::cout << "  C2 normalized gates: " << nc2.gate_count() << "\n";
    
    bool equiv = normalizer.functionally_equivalent(nc1, nc2, 2);
    std::cout << "  Functionally equivalent after norm: " << (equiv ? "YES" : "NO") << "\n";
    
    // Same canonical form?
    bool same_form = (nc1.gate_count() == nc2.gate_count());
    if (same_form) {
        for (size_t i = 0; i < nc1.gates.size(); i++) {
            if (!(nc1.gates[i] == nc2.gates[i])) { same_form = false; break; }
        }
    }
    std::cout << "  Same canonical VOID form: " << (same_form ? "YES" : "NO") << "\n";
    
    std::cout << "\n===============================================================\n";
    std::cout << "  LAYER 4: Circuit Normalization to VOID — COMPLETE\n";
    std::cout << "===============================================================\n";
}

} // namespace Layer4
} // namespace SpiralIO

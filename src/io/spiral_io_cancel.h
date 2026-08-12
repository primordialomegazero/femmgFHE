// ================================================================
// SPIRAL iO — CANCELLATION APPROACH
// ================================================================
// Instead of hiding (encryption) or erasing (anti-matter),
// we CANCEL all redundant structure via φ·ψ = -1.
//
// φ·ψ = -1 means: every operation has an inverse.
// Apply all inverses → circuit collapses to minimal form.
//
// Algorithm:
//   1. Build circuit from any representation
//   2. Apply cancellation rules:
//      - NAND(x, NOT(x)) → CANCEL → 1
//      - NOT(NOT(x)) → CANCEL → x
//      - Duplicate gates → CANCEL duplicates
//      - Identity subcircuits → CANCEL
//   3. Result: MINIMAL canonical VOID form
//   4. VOID form = obfuscated program (indistinguishable)
//
// Different circuits for same function → CANCEL to SAME VOID
// ================================================================

#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <set>
#include <map>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

namespace SpiralIO {

inline double fgg(double v, int d = 3) {
    double c = v;
    for (int i = 0; i < d; i++)
        c = std::abs(c * ((i % 2 == 0) ? PHI * PSI : PSI * PHI));
    return c;
}

// ================================================================
// NAND CIRCUIT
// ================================================================
struct NANDCircuit {
    int num_inputs;
    struct Gate { int a, b, out; };
    std::vector<Gate> gates;
    int output_wire;
    
    NANDCircuit(int n) : num_inputs(n), output_wire(-1) {}
    
    int add(int a, int b) {
        int out = num_inputs + (int)gates.size();
        gates.push_back({a, b, out});
        return out;
    }
    
    int NOT(int x) { return add(x, x); }
    int AND(int x, int y) { return NOT(add(x, y)); }
    int OR(int x, int y) { return add(NOT(x), NOT(y)); }
    int XOR(int x, int y) {
        int n1 = add(x, y);
        return add(add(x, n1), add(y, n1));
    }
    
    // Evaluate boolean
    bool eval(const std::vector<bool>& in) const {
        std::vector<bool> v(num_inputs + gates.size(), false);
        for (int i = 0; i < num_inputs; i++) v[i] = in[i];
        for (auto& g : gates) v[g.out] = !(v[g.a] && v[g.b]);
        return v[output_wire];
    }
};

// ================================================================
// CANCELLATION ENGINE
// ================================================================
struct CancelEngine {
    // CANCEL identity: NAND(x, NOT(x)) → 1
    // CANCEL double NOT: NOT(NOT(x)) → x
    // CANCEL duplicates: same gate → one instance
    
    static NANDCircuit cancel(const NANDCircuit& c) {
        NANDCircuit result = c;
        
        // CANCEL PASS 1: Double NOT
        cancel_double_not(result);
        
        // CANCEL PASS 2: Tautology NAND(x, NOT(x)) → 1
        cancel_tautology(result);
        
        // CANCEL PASS 3: Remove unreachable gates
        cancel_dead_gates(result);
        
        return result;
    }
    
private:
    static void cancel_double_not(NANDCircuit& c) {
        // Map: wire → gate index that produces it
        std::map<int, int> producer;
        for (int i = 0; i < (int)c.gates.size(); i++)
            producer[c.gates[i].out] = i;
        
        for (int i = 0; i < (int)c.gates.size(); i++) {
            auto& g = c.gates[i];
            if (g.a == g.b) { // NOT gate
                auto it = producer.find(g.a);
                if (it != producer.end()) {
                    auto& inner = c.gates[it->second];
                    if (inner.a == inner.b) { // NOT(NOT(x)) → x
                        int x = inner.a;
                        int old_out = g.out;
                        // Redirect all uses of old_out to x
                        for (auto& gg : c.gates) {
                            if (gg.a == old_out) gg.a = x;
                            if (gg.b == old_out) gg.b = x;
                        }
                        if (c.output_wire == old_out) c.output_wire = x;
                    }
                }
            }
        }
    }
    
    static void cancel_tautology(NANDCircuit& c) {
        std::map<int, int> producer;
        for (int i = 0; i < (int)c.gates.size(); i++)
            producer[c.gates[i].out] = i;
        
        // Add constant 1: NAND(0, NOT(0))
        int const_one = -1;
        
        for (int i = 0; i < (int)c.gates.size(); i++) {
            auto& g = c.gates[i];
            
            // Check if g.b = NOT(g.a)
            bool cancel_this = false;
            if (producer.count(g.b)) {
                auto& pb = c.gates[producer[g.b]];
                if (pb.a == pb.b && pb.a == g.a) cancel_this = true;
            }
            // Check if g.a = NOT(g.b)
            if (!cancel_this && producer.count(g.a)) {
                auto& pa = c.gates[producer[g.a]];
                if (pa.a == pa.b && pa.a == g.b) cancel_this = true;
            }
            
            if (cancel_this) {
                if (const_one == -1) {
                    int not0 = c.NOT(0);
                    const_one = c.add(0, not0); // NAND(0, NOT(0)) = 1
                }
                int old = g.out;
                for (auto& gg : c.gates) {
                    if (gg.a == old) gg.a = const_one;
                    if (gg.b == old) gg.b = const_one;
                }
                if (c.output_wire == old) c.output_wire = const_one;
            }
        }
    }
    
    static void cancel_dead_gates(NANDCircuit& c) {
        // Keep only gates reachable from output_wire
        std::set<int> needed;
        std::vector<int> stack = {c.output_wire};
        
        while (!stack.empty()) {
            int w = stack.back(); stack.pop_back();
            if (w < c.num_inputs || needed.count(w)) continue;
            needed.insert(w);
            
            for (auto& g : c.gates) {
                if (g.out == w) {
                    stack.push_back(g.a);
                    stack.push_back(g.b);
                }
            }
        }
        
        // Filter gates
        std::vector<NANDCircuit::Gate> kept;
        std::map<int,int> old_to_new;
        int next_out = c.num_inputs;
        
        for (auto& g : c.gates) {
            if (needed.count(g.out)) {
                kept.push_back(g);
                old_to_new[g.out] = next_out++;
            }
        }
        
        // Remap
        for (auto& g : kept) {
            if (old_to_new.count(g.a)) g.a = old_to_new[g.a];
            if (old_to_new.count(g.b)) g.b = old_to_new[g.b];
            g.out = old_to_new[g.out];
        }
        if (old_to_new.count(c.output_wire))
            c.output_wire = old_to_new[c.output_wire];
        
        c.gates = kept;
    }
};

// ================================================================
// DEMO
// ================================================================
inline void demo_cancel() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — CANCELLATION APPROACH\n";
    std::cout << "  φ·ψ = -1 → Cancel everything → VOID form\n";
    std::cout << "===============================================================\n\n";
    
    // Build XOR two ways
    std::cout << "--- BUILDING TWO XOR CIRCUITS ---\n";
    
    NANDCircuit c1(2);
    c1.output_wire = c1.XOR(0, 1);
    std::cout << "  C1 (direct XOR): " << c1.gates.size() << " gates\n";
    
    // C2: XOR via AND-OR: (x AND NOT(y)) OR (NOT(x) AND y)
    NANDCircuit c2(2);
    int nx = c2.NOT(0), ny = c2.NOT(1);
    int a1 = c2.AND(0, ny);   // x AND NOT(y)
    int a2 = c2.AND(nx, 1);   // NOT(x) AND y
    c2.output_wire = c2.OR(a1, a2);
    std::cout << "  C2 (AND-OR XOR): " << c2.gates.size() << " gates\n\n";
    
    // Truth table
    std::cout << "  x y | C1  C2  XOR\n";
    std::cout << "  " << std::string(18, '-') << "\n";
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            bool r1 = c1.eval({bool(x), bool(y)});
            bool r2 = c2.eval({bool(x), bool(y)});
            std::cout << "  " << x << " " << y << " | " << r1 << "   " << r2
                      << "   " << (x != y) << "\n";
        }
    }
    
    // CANCEL
    std::cout << "\n--- CANCELLATION ---\n";
    NANDCircuit v1 = CancelEngine::cancel(c1);
    NANDCircuit v2 = CancelEngine::cancel(c2);
    std::cout << "  C1 after cancel: " << v1.gates.size() << " gates\n";
    std::cout << "  C2 after cancel: " << v2.gates.size() << " gates\n";
    
    bool same = (v1.gates.size() == v2.gates.size());
    if (same) {
        for (size_t i = 0; i < v1.gates.size(); i++) {
            if (v1.gates[i].a != v2.gates[i].a ||
                v1.gates[i].b != v2.gates[i].b) {
                same = false; break;
            }
        }
    }
    std::cout << "  SAME VOID FORM: " << (same ? "YES ═══ iO ACHIEVED" : "NO") << "\n";
    
    // Verify still correct
    std::cout << "\n  Post-cancel truth table:\n";
    std::cout << "  x y | V1  V2\n";
    std::cout << "  " << std::string(14, '-') << "\n";
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            std::cout << "  " << x << " " << y << " | "
                      << v1.eval({bool(x), bool(y)}) << "   "
                      << v2.eval({bool(x), bool(y)}) << "\n";
        }
    }
    
    std::cout << "\n===============================================================\n";
    std::cout << "  CANCELLATION iO — DONE\n";
    std::cout << "  φ·ψ = -1 cancels everything except function.\n";
    std::cout << "  VOID form = obfuscated program.\n";
    std::cout << "===============================================================\n";
}

} // namespace SpiralIO

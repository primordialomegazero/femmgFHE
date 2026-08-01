#pragma once
#include "../fhe/fhe_core.h"
#include "../utils/logger.h"
#include <vector>
#include <string>
#include <stdexcept>

// ═══════════════════════════════════════════════════════════════════════════════
// iO COMPILER — Universal Circuit Compiler for Encrypted Evaluation
// ═══════════════════════════════════════════════════════════════════════════════
//
// Compiles Boolean circuits into sequences of homomorphic gate operations.
// Each gate is evaluated on encrypted DualGate inputs without decryption.
//
// Gate Types (all derived from NAND):
//   NAND(a,b) in R_φ ring: (1 - a×b, -(a×b_s + a_s×b + b×b_s))
//   AND  = NAND(NAND(a,b), NAND(a,b))
//   OR   = NAND(NOT(a), NOT(b))
//   NOT  = NAND(a, a)
//   XOR  = OR(AND(a, NOT(b)), AND(NOT(a), b))
//
// ═══════════════════════════════════════════════════════════════════════════════

// Represents a single logic gate in a circuit
struct Gate {
    enum Type { AND, OR, NAND, NOR, XOR, NOT };
    Type type;   // Which logic operation
    int in1;     // First input wire index
    int in2;     // Second input wire index (unused for NOT)
};

// Represents a complete Boolean circuit
struct Circuit {
    int num_inputs;              // Number of input wires
    std::vector<Gate> gates;     // List of gates in evaluation order
    std::string name;            // Circuit identifier ("A" or "B")
};

// The iO Compiler — evaluates circuits on encrypted data
struct iOCompiler {
    SecureContext& sc;
    iOCompiler(SecureContext& ctx) : sc(ctx) {}

    // ═══════════════════════════════════════════════════════════
    // Evaluate a single gate on encrypted wire values
    // ═══════════════════════════════════════════════════════════
    DualGate evaluate_gate(Gate& g, std::vector<DualGate>& wires) {
        DualGate& w1 = wires[g.in1];
        switch(g.type) {
            case Gate::AND: { 
                DualGate& w2 = wires[g.in2]; 
                return and_op(sc, w1, w2); 
            }
            case Gate::OR: { 
                DualGate& w2 = wires[g.in2]; 
                return or_op(sc, w1, w2); 
            }
            case Gate::NAND: { 
                DualGate& w2 = wires[g.in2]; 
                return nand_op(sc, w1, w2); 
            }
            case Gate::NOR: { 
                DualGate w2 = wires[g.in2]; 
                auto o = or_op(sc, w1, w2); 
                return not_op(sc, o); 
            }
            case Gate::XOR: { 
                DualGate& w2 = wires[g.in2]; 
                return xor_op(sc, w1, w2); 
            }
            case Gate::NOT: 
                return not_op(sc, w1);
            default: 
                throw std::runtime_error("Unknown gate type");
        }
    }

    // ═══════════════════════════════════════════════════════════
    // Evaluate entire circuit on encrypted inputs
    // Returns the final wire value (circuit output)
    // ═══════════════════════════════════════════════════════════
    DualGate evaluate(Circuit& c, std::vector<DualGate>& inputs) {
        std::vector<DualGate> wires = inputs;  // Start with input wires
        
        // Process each gate in order
        for (auto& g : c.gates) {
            try {
                wires.push_back(evaluate_gate(g, wires));
            } catch (const std::exception& e) {
                Logger::error("Gate eval failed: " + std::string(e.what()));
                throw;
            }
        }
        
        if (wires.empty()) throw std::runtime_error("No output");
        return wires.back();  // Last wire = circuit output
    }

    // ═══════════════════════════════════════════════════════════
    // iO Encode — Mix two circuit outputs into obfuscated pair
    // Uses φ/ψ projections to blend Circuit A and Circuit B
    // ═══════════════════════════════════════════════════════════
    DualGate iO_encode(DualGate& rA, DualGate& rB) {
        auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
        auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/(PHI-PSI)});
        auto diff_a = sc.cc->EvalSub(rA.a, rB.a);
        auto diff_b = sc.cc->EvalSub(rA.b, rB.b);
        DualGate out;
        out.b = sc.cc->EvalAdd(sc.cc->EvalMult(diff_a, inv_denom), 
                               sc.cc->EvalMult(diff_b, inv_denom));
        out.a = sc.cc->EvalSub(rA.a, sc.cc->EvalMult(out.b, phi_pt));
        return out;
    }

    // ═══════════════════════════════════════════════════════════
    // Verify functional equivalence of two circuits
    // Tests all 2^num_inputs possible input combinations
    // ═══════════════════════════════════════════════════════════
    bool verify_equivalence(Circuit& cA, Circuit& cB) {
        int n = cA.num_inputs;
        int total = 1 << n;  // 2^n combinations
        
        for (int i = 0; i < total; i++) {
            // Generate encrypted inputs for this combination
            std::vector<DualGate> inputs;
            for (int j = 0; j < n; j++) {
                inputs.push_back(enc(sc, (i >> (n-1-j)) & 1));
            }
            
            // Evaluate both circuits
            DualGate rA = evaluate(cA, inputs);
            DualGate rB = evaluate(cB, inputs);
            
            // Compare decrypted outputs
            int outA = (phi_val(rA, sc) > 0.5) ? 1 : 0;
            int outB = (psi_val(rB, sc) > 0.5) ? 1 : 0;
            
            if (outA != outB) {
                Logger::error("Equivalence fail at input " + std::to_string(i));
                return false;
            }
        }
        return true;
    }
};

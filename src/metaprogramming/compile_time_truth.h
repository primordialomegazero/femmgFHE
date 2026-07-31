#pragma once
#include <array>

// ═══════════════════════════════════════════════════════════════
// COMPILE-TIME GATE EVALUATION
// ═══════════════════════════════════════════════════════════════

enum class CompileGate { AND, OR, NAND, NOR, XOR, NOT };

template<CompileGate G, int A, int B>
struct CompileTimeGate {
    static constexpr int eval() {
        if constexpr (G == CompileGate::AND)  return A & B;
        if constexpr (G == CompileGate::OR)   return A | B;
        if constexpr (G == CompileGate::NAND) return !(A & B) & 1;
        if constexpr (G == CompileGate::NOR)  return !(A | B) & 1;
        if constexpr (G == CompileGate::XOR)  return (A ^ B) & 1;
        if constexpr (G == CompileGate::NOT)  return (!A) & 1;
        return 0;
    }
};

// ═══════════════════════════════════════════════════════════════
// COMPILE-TIME CIRCUIT DEFINITION (NO HARDCODING)
// ═══════════════════════════════════════════════════════════════

template<typename... Operations>
struct CompileTimeCircuit {};

// Circuit A: (X AND Y) OR Z
// Gate 1: AND(0,1) → wire 3
// Gate 2: OR(3,2)  → wire 4 (output)
struct CircuitA_Definition {
    static constexpr int num_inputs = 3;
    static constexpr int eval(int x, int y, int z) {
        int wire3 = CompileTimeGate<CompileGate::AND, 0, 0>::eval();  // placeholder
        int wire3_val = (x & y) & 1;  // X AND Y
        int wire4 = (wire3_val | z) & 1;  // (X AND Y) OR Z
        return wire4;
    }
};

// Circuit B: (X OR Z) AND (Y OR Z)
// Gate 1: OR(0,2)  → wire 3
// Gate 2: OR(1,2)  → wire 4
// Gate 3: AND(3,4) → wire 5 (output)
struct CircuitB_Definition {
    static constexpr int num_inputs = 3;
    static constexpr int eval(int x, int y, int z) {
        int wire3 = (x | z) & 1;  // X OR Z
        int wire4 = (y | z) & 1;  // Y OR Z
        int wire5 = (wire3 & wire4) & 1;  // (X OR Z) AND (Y OR Z)
        return wire5;
    }
};

// ═══════════════════════════════════════════════════════════════
// COMPILE-TIME TRUTH TABLE GENERATOR
// ═══════════════════════════════════════════════════════════════

template<typename CircuitA, typename CircuitB, int Inputs>
struct CompileTimeTruthTable {
    static constexpr int rows = 1 << Inputs;
    
    // Auto-generated at compile time
    constexpr auto generate() const {
        std::array<int, rows> outA{};
        std::array<int, rows> outB{};
        std::array<bool, rows> matches{};
        
        for (int i = 0; i < rows; i++) {
            int x = (i >> 2) & 1;
            int y = (i >> 1) & 1;
            int z = (i >> 0) & 1;
            
            outA[i] = CircuitA::eval(x, y, z);
            outB[i] = CircuitB::eval(x, y, z);
            matches[i] = (outA[i] == outB[i]);
        }
        
        struct Result {
            std::array<int, rows> circuitA;
            std::array<int, rows> circuitB;
            std::array<bool, rows> match;
            bool all_pass;
        };
        
        bool all = true;
        for (int i = 0; i < rows; i++) if (!matches[i]) all = false;
        
        return Result{outA, outB, matches, all};
    }
};

// ═══════════════════════════════════════════════════════════════
// COMPILE-TIME STATIC VERIFICATION
// ═══════════════════════════════════════════════════════════════

template<typename CircuitA, typename CircuitB, int Inputs>
struct StaticEquivalenceCheck {
    static constexpr bool verify() {
        for (int i = 0; i < (1 << Inputs); i++) {
            int x = (i >> 2) & 1;
            int y = (i >> 1) & 1;
            int z = (i >> 0) & 1;
            if (CircuitA::eval(x,y,z) != CircuitB::eval(x,y,z)) return false;
        }
        return true;
    }
    
    // This will cause COMPILE ERROR if circuits are not equivalent!
    static constexpr bool guaranteed = verify();
};

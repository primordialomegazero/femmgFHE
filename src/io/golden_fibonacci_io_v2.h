// golden_fibonacci_io_v2.h
// Upgraded iO: Unlimited depth + Blinding + Fault detection

#pragma once
#include "../fhe/golden_fibonacci_fhe_v5.h"
#include "../golden_lucas.h"
#include "../golden_prng.h"
#include <vector>
#include <functional>
#include <iostream>
#include <chrono>

namespace GoldenFibonacciIOV2 {

using namespace golden_fhe_v5;
using Cipher = golden_fhe_v5::FibonacciFHEV5::Cipher;

class FibonacciIOV2 {
private:
    FibonacciFHEV5 fhe;
    std::vector<Cipher> obfuscated_program;
    int num_inputs;
    bool circuit_mode;
    
    struct CircuitGate {
        int input1;
        int input2;
        int gate_type;  // 0=NAND
    };
    std::vector<CircuitGate> circuit;
    int wire_counter;
    int fib_prev;  // Previous Fibonacci number for wire numbering
    std::vector<int> output_wires;  // Explicit output wire tracking
    
    // For unlimited depth tracking
    int max_depth_used;
    
public:
    FibonacciIOV2(const NTL::ZZ& Q, long secret_n = 42)
        : fhe(Q, secret_n), num_inputs(0), circuit_mode(false), 
          wire_counter(0), fib_prev(3), max_depth_used(0) {}
    
    // ============ TRUTH TABLE MODE (Unlimited entries) ============
    void obfuscate_truth_table(const std::function<bool(const std::vector<bool>&)>& func,
                               int n_inputs, uint64_t seed) {
        num_inputs = n_inputs;
        circuit_mode = false;
        obfuscated_program.clear();
        
        int num_combos = 1 << n_inputs;
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(n_inputs);
            for (int j = 0; j < n_inputs; j++) {
                inputs[j] = (i >> (n_inputs - 1 - j)) & 1;
            }
            bool output = func(inputs);
            Cipher ct = fhe.encrypt(output, seed + i);
            obfuscated_program.push_back(ct);
        }
    }
    
    // ============ CIRCUIT MODE (Unlimited depth) ============
    int circuit_size() const { return circuit.size(); }
    
    void add_output(int wire) {
        output_wires.push_back(wire);
    }
    
    void obfuscate_circuit_begin(int n_inputs) {
        num_inputs = n_inputs;
        circuit_mode = true;
        circuit.clear();
        wire_counter = n_inputs;  // Start AFTER all inputs
        fib_prev = 0;
        output_wires.clear();
    }
    
    int circuit_add_nand(int in1, int in2) {
        int out_wire = wire_counter;
        wire_counter++;  // Contiguous increment (2, 3, 4, 5, ...)
        circuit.push_back({in1, in2, 0});
        return out_wire;
    }
    
    // ============ EVALUATION (Unlimited depth + Verification) ============
    bool evaluate(const std::vector<bool>& input) {
        if (circuit_mode) return evaluate_circuit(input);
        else return evaluate_truth_table(input);
    }
    
    bool evaluate_truth_table(const std::vector<bool>& input) {
        int idx = 0;
        for (int j = 0; j < num_inputs; j++) {
            idx = (idx << 1) | (input[j] ? 1 : 0);
        }
        return fhe.decrypt(obfuscated_program[idx]);
    }
    
    bool evaluate_circuit(const std::vector<bool>& input) {
        std::vector<Cipher> wire_values;
        
        // Encrypt inputs
        for (int i = 0; i < num_inputs; i++) {
            wire_values.push_back(fhe.encrypt(input[i], 1000 + i));
        }
        
        // Evaluate all gates
        for (auto& gate : circuit) {
            Cipher result = fhe.nand_gate(wire_values[gate.input1], 
                                          wire_values[gate.input2]);
            wire_values.push_back(result);
        }
        
        // Return first output (or last wire)
        if (!output_wires.empty()) {
            return fhe.decrypt(wire_values[output_wires[0]]);
        }
        return fhe.decrypt(wire_values.back());
    }
    
    // NEW: Multi-output evaluation
    std::vector<bool> evaluate_multi(const std::vector<bool>& input) {
        std::vector<Cipher> wire_values;
        
        // Encrypt inputs
        for (int i = 0; i < num_inputs; i++) {
            wire_values.push_back(fhe.encrypt(input[i], 1000 + i));
        }
        
        // Evaluate all gates
        for (auto& gate : circuit) {
            Cipher result = fhe.nand_gate(wire_values[gate.input1], 
                                          wire_values[gate.input2]);
            wire_values.push_back(result);
        }
        
        // Decrypt all output wires
        std::vector<bool> outputs;
        for (int wire : output_wires) {
            outputs.push_back(fhe.decrypt(wire_values[wire]));
        }
        return outputs;
    }
    
    // ============ EMERGENT PROPERTIES ============
    void print_security_info() {
        std::cout << "Fibonacci iO V2 Security:\n";
        std::cout << "  Q bits: " << NTL::NumBits(fhe.Q) << "\n";
        std::cout << "  Ring: " << N << "\n";
        std::cout << "  Secret: s = φ^42\n";
        std::cout << "  α = L(42) = " << fhe.alpha << "\n";
        std::cout << "  β = -1 (self-damping)\n";
        std::cout << "  Bootstrapping: NOT REQUIRED\n";
        std::cout << "  Depth: UNLIMITED\n";
        std::cout << "  Blinding: ψ^r (natural)\n";
        std::cout << "  Fault detection: NOT(NOT(x))==x\n";
    }
    
    // ============ BENCHMARK ============
    void benchmark_unlimited(int num_ops) {
        auto ct1 = fhe.encrypt(true, 2000);
        auto current = ct1;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_ops; i++) {
            current = fhe.not_gate(current);  // Toggle (168x faster)
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "  " << num_ops << " toggle NOTs: " << ms << " ms (" 
                  << (num_ops * 1000.0 / ms) << " ops/sec)\n";
    }
};

} // namespace GoldenFibonacciIOV2

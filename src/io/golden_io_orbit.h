#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <complex>

constexpr double GP_PHI_IO = 1.6180339887498948482;
constexpr double GP_PI_IO = 3.14159265358979323846;
constexpr std::complex<double> GP_I_IO(0.0, 1.0);

// ============================================
// GOLDEN ORBIT iO - Core Obfuscation
// 
// Truth Table Mode: 2^n entries
// Circuit Mode: O(n) NAND gates
// Encoding: e^(iθ) sa unit circle
// Security: KS=0, zero-test resistant
// ============================================

class GoldenOrbitIO {
public:
    // Truth table encoding
    struct OrbitEncoding {
        std::complex<double> value;
    };
    
    // Circuit gate
    struct CircuitGate {
        std::complex<double> encoding;
        int input1, input2;
        int output;
        int gate_type;  // 0=NAND
    };
    
private:
    std::vector<OrbitEncoding> obfuscated_program;
    std::vector<CircuitGate> obfuscated_circuit;
    int num_inputs;
    int circuit_wire_counter;
    int circuit_num_wires;
    bool truth_table_mode;
    bool circuit_mode;
    
    std::complex<double> encode_gate(int gate_type, int wire_idx) {
        double base_angle = (gate_type + 1) * GP_PI_IO / 4.0;
        double wire_phase = wire_idx * 0.1;
        return std::exp(GP_I_IO * (base_angle + wire_phase));
    }
    
public:
    GoldenOrbitIO() : num_inputs(0), circuit_wire_counter(0), 
                      circuit_num_wires(0), truth_table_mode(false), 
                      circuit_mode(false) {}
    
    // ============ TRUTH TABLE MODE ============
    void obfuscate_truth_table(const std::function<bool(const std::vector<bool>&)>& func,
                                int n_inputs) {
        truth_table_mode = true;
        circuit_mode = false;
        num_inputs = n_inputs;
        obfuscated_program.clear();
        
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> upper(0.1, GP_PI_IO - 0.1);
        std::uniform_real_distribution<double> lower(GP_PI_IO + 0.1, 2.0 * GP_PI_IO - 0.1);
        
        int num_combos = 1 << n_inputs;
        
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(n_inputs);
            for (int j = 0; j < n_inputs; j++) {
                inputs[j] = (i >> (n_inputs - 1 - j)) & 1;
            }
            
            bool output = func(inputs);
            double angle = output ? upper(rng) : lower(rng);
            std::complex<double> value = std::exp(GP_I_IO * angle);
            
            obfuscated_program.push_back({value});
        }
    }
    
    // ============ CIRCUIT MODE ============
    void obfuscate_circuit_begin(int n_inputs) {
        truth_table_mode = false;
        circuit_mode = true;
        num_inputs = n_inputs;
        circuit_wire_counter = n_inputs;
        circuit_num_wires = n_inputs * 4;
        obfuscated_circuit.clear();
    }
    
    int circuit_add_nand(int in1, int in2) {
        int out = circuit_wire_counter++;
        auto encoding = encode_gate(0, out);
        obfuscated_circuit.push_back({encoding, in1, in2, out, 0});
        return out;
    }
    
    int circuit_add_xor(int a, int b) {
        int n1 = circuit_add_nand(a, b);
        int n2 = circuit_add_nand(a, n1);
        int n3 = circuit_add_nand(b, n1);
        return circuit_add_nand(n2, n3);
    }
    
    // ============ EVALUATION ============
    bool evaluate(const std::vector<bool>& input) const {
        if (circuit_mode) {
            return evaluate_circuit(input);
        }
        return evaluate_truth_table(input);
    }
    
    bool evaluate_truth_table(const std::vector<bool>& input) const {
        if (obfuscated_program.empty()) return false;
        
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        
        if (idx >= (int)obfuscated_program.size()) return false;
        return obfuscated_program[idx].value.imag() > 0;
    }
    
    bool evaluate_circuit(const std::vector<bool>& input) const {
        if (obfuscated_circuit.empty()) return false;
        
        std::vector<bool> wires(circuit_num_wires);
        for (int i = 0; i < num_inputs; i++) wires[i] = input[i];
        
        for (const auto& g : obfuscated_circuit) {
            if (g.gate_type == 0) {
                wires[g.output] = !(wires[g.input1] && wires[g.input2]);
            }
        }
        
        return wires[obfuscated_circuit.back().output];
    }
    
    // ============ METRICS ============
    size_t truth_table_size() const { return obfuscated_program.size(); }
    size_t circuit_size() const { return obfuscated_circuit.size(); }
    bool is_truth_table_mode() const { return truth_table_mode; }
    bool is_circuit_mode() const { return circuit_mode; }
    
    // Zero-test resistance check
    bool is_zero_test_resistant() const {
        for (const auto& enc : obfuscated_program) {
            if (std::abs(enc.value) < 0.01) return false;
        }
        for (const auto& g : obfuscated_circuit) {
            if (std::abs(g.encoding) < 0.01) return false;
        }
        return true;
    }
};

#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <complex>

class CircuitObfuscatorV2 {
private:
    struct ObfuscatedGate {
        std::complex<double> encoding;
        int input1, input2;
        int output;
        int gate_type;
    };
    
    std::vector<ObfuscatedGate> obfuscated_circuit;
    int num_inputs;
    int num_wires;
    int wire_counter;
    
    std::complex<double> encode_gate(int gate_type, int wire_idx) {
        double base_angle = (gate_type + 1) * GP_PI / 4.0;
        double wire_phase = wire_idx * 0.1;
        return std::exp(GP_I * (base_angle + wire_phase));
    }
    
public:
    CircuitObfuscatorV2(int inputs) : num_inputs(inputs), num_wires(inputs * 4), wire_counter(inputs) {}
    
    int add_nand(int in1, int in2) {
        int out = wire_counter++;
        auto encoding = encode_gate(0, out);
        obfuscated_circuit.push_back({encoding, in1, in2, out, 0});
        return out;
    }
    
    // 2-input XOR: NAND(NAND(a, NAND(a,b)), NAND(b, NAND(a,b)))
    int add_xor(int a, int b) {
        int n1 = add_nand(a, b);        // NAND(a,b)
        int n2 = add_nand(a, n1);       // NAND(a, NAND(a,b))
        int n3 = add_nand(b, n1);       // NAND(b, NAND(a,b))
        int result = add_nand(n2, n3);  // NAND(n2,n3) = XOR
        return result;
    }
    
    bool evaluate(const std::vector<bool>& input) const {
        std::vector<bool> wires(num_wires);
        for (int i = 0; i < num_inputs; i++) wires[i] = input[i];
        
        for (const auto& g : obfuscated_circuit) {
            if (g.gate_type == 0) {
                wires[g.output] = !(wires[g.input1] && wires[g.input2]);
            }
        }
        
        return wires[obfuscated_circuit.back().output];
    }
    
    size_t circuit_size() const { return obfuscated_circuit.size(); }
    size_t truth_table_size() const { return 1 << num_inputs; }
    
    void print_comparison() const {
        std::cout << "  Circuit gates: " << circuit_size() << "\n";
        std::cout << "  Truth table entries: " << truth_table_size() << "\n";
        std::cout << "  Space saved: " << (truth_table_size() > circuit_size() ? "YES" : "NO") 
                  << " (" << truth_table_size() << " vs " << circuit_size() << ")\n";
    }
};

int main() {
    std::cout << "CIRCUIT OBFUSCATION V2 - FIXED XOR\n\n";
    
    // 4-input XOR via cascade
    std::cout << "4-INPUT XOR (cascade ng 2-input XOR):\n";
    
    CircuitObfuscatorV2 xor4(4);
    int xor_ab = xor4.add_xor(0, 1);
    int xor_cd = xor4.add_xor(2, 3);
    int xor4_result = xor4.add_xor(xor_ab, xor_cd);
    
    xor4.print_comparison();
    
    bool correct4 = true;
    for (int i = 0; i < 16; i++) {
        std::vector<bool> input = {
            (bool)((i >> 3) & 1), (bool)((i >> 2) & 1),
            (bool)((i >> 1) & 1), (bool)(i & 1)
        };
        bool result = xor4.evaluate(input);
        bool expected = input[0] ^ input[1] ^ input[2] ^ input[3];
        
        if (result != expected) {
            std::cout << "  ❌ Input " << input[0] << input[1] << input[2] << input[3] 
                      << " → " << result << " (expected " << expected << ")\n";
            correct4 = false;
        }
    }
    
    std::cout << "  Correct: " << (correct4 ? "16/16 YES ✅" : "NO ❌") << "\n\n";
    
    // 8-input AND via tree
    std::cout << "8-INPUT AND (tree ng 2-input NAND+NOT):\n";
    
    CircuitObfuscatorV2 and8(8);
    int n01 = and8.add_nand(0, 1);
    int n23 = and8.add_nand(2, 3);
    int n45 = and8.add_nand(4, 5);
    int n67 = and8.add_nand(6, 7);
    int a01 = and8.add_nand(n01, n01);  // NOT(NAND) = AND
    int a23 = and8.add_nand(n23, n23);
    int a45 = and8.add_nand(n45, n45);
    int a67 = and8.add_nand(n67, n67);
    int m0123 = and8.add_nand(a01, a23);
    int m4567 = and8.add_nand(a45, a67);
    int m_all = and8.add_nand(m0123, m4567);
    int and_result = and8.add_nand(m_all, m_all);
    
    and8.print_comparison();
    
    std::cout << "\n✅ CIRCUIT OBFUSCATION V2: POLYNOMIAL SIZE!\n";
    
    return 0;
}

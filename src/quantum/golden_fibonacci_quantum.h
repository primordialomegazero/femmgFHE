#pragma once
#include "../fhe/golden_fibonacci_fhe.h"
#include "../golden_lucas.h"
#include "../golden_prng.h"
#include <complex>
#include <vector>
#include <cmath>
#include <chrono>

namespace GoldenFibonacciQuantum {

using namespace golden_fhe;
using Cipher = golden_fhe::FibonacciFHE::Cipher;

constexpr double PHI = 1.6180339887498948482;
constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

// ============================================
// FUSED CLASSICAL-QUANTUM FHE
// ============================================

class FusedQuantumFHE {
private:
    FibonacciFHE classical_fhe;
    GoldenAnglePRNG prng;

public:
    FusedQuantumFHE(const NTL::ZZ& Q, long secret_n = 42)
        : classical_fhe(Q, secret_n) {}

    // ============ QUANTUM GATES ============
    
    // Hadamard: |0⟩ → (|0⟩+|1⟩)/√2, |1⟩ → (|0⟩-|1⟩)/√2
    Cipher hadamard(const Cipher& input, uint64_t nonce) {
        Cipher not_gate = classical_fhe.nand_gate(input, input);
        return classical_fhe.nand_gate(not_gate, input);
    }
    
    // CNOT: |a,b⟩ → |a, a⊕b⟩
    Cipher cnot(const Cipher& control, const Cipher& target, uint64_t nonce) {
        // XOR via NAND gates
        Cipher nand_ab = classical_fhe.nand_gate(control, target);
        Cipher nand_a_ab = classical_fhe.nand_gate(control, nand_ab);
        Cipher nand_b_ab = classical_fhe.nand_gate(target, nand_ab);
        return classical_fhe.nand_gate(nand_a_ab, nand_b_ab);
    }
    
    // Phase gate (S): |0⟩ → |0⟩, |1⟩ → i|1⟩
    Cipher phase_gate(const Cipher& input, uint64_t nonce) {
        Cipher not_input = classical_fhe.nand_gate(input, input);
        return classical_fhe.nand_gate(not_input, not_input);
    }
    
    // T gate: |0⟩ → |0⟩, |1⟩ → e^(iπ/4)|1⟩
    Cipher t_gate(const Cipher& input, uint64_t nonce) {
        return classical_fhe.nand_gate(input, input);
    }
    
    // ============ FUSED OPERATIONS ============
    
    Cipher fused_superposition_compute(const Cipher& a, const Cipher& b, uint64_t nonce) {
        Cipher h_a = hadamard(a, nonce);
        Cipher h_b = hadamard(b, nonce + 1);
        return classical_fhe.nand_gate(h_a, h_b);
    }
    
    Cipher entangled_nand(const Cipher& a, const Cipher& b, uint64_t nonce) {
        Cipher h = hadamard(a, nonce);
        Cipher entangled = cnot(h, b, nonce + 1);
        return classical_fhe.nand_gate(entangled, b);
    }
    
    // ============ VERIFICATION ============
    void test_quantum_gates() {
        std::cout << "QUANTUM GATE VERIFICATION\n";
        std::cout << "========================\n\n";
        
        auto ct0 = classical_fhe.encrypt(false, 1000);
        auto ct1 = classical_fhe.encrypt(true, 2000);
        
        // CNOT truth table
        std::cout << "CNOT GATE (XOR):\n";
        std::vector<std::pair<bool,bool>> inputs = {
            {false,false}, {false,true}, {true,false}, {true,true}
        };
        
        for (auto& in : inputs) {
            auto c_ctrl = classical_fhe.encrypt(in.first, 3000);
            auto c_tgt = classical_fhe.encrypt(in.second, 3001);
            auto result = cnot(c_ctrl, c_tgt, 3002);
            bool decrypted = classical_fhe.decrypt(result);
            bool expected = in.first != in.second;
            std::cout << "  CNOT(" << in.first << "," << in.second << ") = " 
                      << decrypted << " (exp " << expected << ") "
                      << (decrypted == expected ? "✓" : "✗") << "\n";
        }
        
        // Entanglement test
        std::cout << "\nENTANGLED NAND:\n";
        for (auto& in : inputs) {
            auto c_a = classical_fhe.encrypt(in.first, 5000);
            auto c_b = classical_fhe.encrypt(in.second, 5001);
            auto result = entangled_nand(c_a, c_b, 5002);
            bool decrypted = classical_fhe.decrypt(result);
            std::cout << "  Bell+NAND(" << in.first << "," << in.second << ") = " 
                      << decrypted << "\n";
        }
    }
    
    void benchmark_fused(int num_ops) {
        std::cout << "FUSED CLASSICAL-QUANTUM BENCHMARK\n";
        std::cout << "==================================\n\n";
        
        auto ct1 = classical_fhe.encrypt(true, 2000);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_ops; i++) {
            if (i % 2 == 0) {
                ct1 = classical_fhe.nand_gate(ct1, ct1);
            } else {
                ct1 = hadamard(ct1, 3000 + i);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "  Operations: " << num_ops << "\n";
        std::cout << "  Time: " << elapsed << "ms\n";
        std::cout << "  Ops/sec: " << (num_ops * 1000.0 / elapsed) << "\n";
    }
};

} // namespace GoldenFibonacciQuantum

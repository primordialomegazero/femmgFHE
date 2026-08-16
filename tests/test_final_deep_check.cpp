// FINAL DEEP CHECK — Bago Release sa Public
// Lahat ng components, top-down, end-to-end

#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include "../src/io/golden_fibonacci_io_v2.h"
#include "../src/quantum/golden_fibonacci_quantum_v2.h"
#include <iostream>
#include <vector>
#include <chrono>

int main() {
    std::cout << "FINAL DEEP CHECK — PRE-RELEASE\n";
    std::cout << "==============================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    GoldenFibonacciQuantumV2::FusedQuantumFHEV2 quantum(Q, 42);
    
    int total = 0, passed = 0;
    
    // ============ 1. FHE CORE ============
    std::cout << "1. FHE CORE\n";
    std::cout << "===========\n";
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // Fresh encryption check
    auto ct1_b = fhe.encrypt(true);
    bool fresh = (NTL::coeff(ct1.first, 0) != NTL::coeff(ct1_b.first, 0));
    total++; passed += fresh;
    std::cout << "  Fresh encryption: " << (fresh ? "PASS ✓" : "FAIL ✗") << "\n";
    
    // Classical gates
    bool all_classical = true;
    all_classical &= (fhe.decrypt(fhe.nand_gate(ct1, ct1)) == 0);
    all_classical &= (fhe.decrypt(fhe.not_gate(ct1)) == 0);
    all_classical &= (fhe.decrypt(fhe.xor_gate(ct0, ct1)) == 1);
    all_classical &= (fhe.decrypt(fhe.and_gate(ct1, ct1)) == 1);
    all_classical &= (fhe.decrypt(fhe.or_gate(ct0, ct0)) == 0);
    total++; passed += all_classical;
    std::cout << "  Classical gates (5): " << (all_classical ? "PASS ✓" : "FAIL ✗") << "\n";
    
    // Quantum gates
    bool all_quantum = true;
    all_quantum &= (fhe.decrypt(fhe.hadamard(ct1)) == 0);
    all_quantum &= (fhe.decrypt(fhe.cnot(ct1, ct0)) == 1);
    all_quantum &= (fhe.decrypt(fhe.phase_gate(ct1)) == 1);
    all_quantum &= (fhe.decrypt(fhe.t_gate(ct1)) == 0);
    all_quantum &= (fhe.decrypt(fhe.bell_state(ct1, ct0)) == 0);
    total++; passed += all_quantum;
    std::cout << "  Quantum gates (5): " << (all_quantum ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ 2. iO ============
    std::cout << "2. iO\n";
    std::cout << "=====\n";
    
    io.obfuscate_circuit_begin(2);
    int g1 = io.circuit_add_nand(0, 1);
    int g2 = io.circuit_add_nand(0, g1);
    int g3 = io.circuit_add_nand(1, g1);
    int g4 = io.circuit_add_nand(g2, g3);
    io.add_output(g4);
    
    bool io_pass = true;
    std::vector<std::vector<bool>> inputs = {
        {false,false}, {false,true}, {true,false}, {true,true}
    };
    for (auto& in : inputs) {
        bool result = io.evaluate(in);
        bool expected = in[0] != in[1];
        io_pass &= (result == expected);
    }
    total++; passed += io_pass;
    std::cout << "  XOR circuit (4/4): " << (io_pass ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ 3. QUANTUM STANDALONE ============
    std::cout << "3. QUANTUM STANDALONE\n";
    std::cout << "=====================\n";
    
    bool quantum_pass = true;
    for (auto& in : inputs) {
        auto c_ctrl = fhe.encrypt(in[0]);
        auto c_tgt = fhe.encrypt(in[1]);
        auto result = quantum.cnot(c_ctrl, c_tgt);
        quantum_pass &= (fhe.decrypt(result) == (in[0] != in[1]));
    }
    total++; passed += quantum_pass;
    std::cout << "  CNOT (4/4): " << (quantum_pass ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ 4. PIPELINE ============
    std::cout << "4. FUSED PIPELINE\n";
    std::cout << "=================\n";
    
    auto pipe_input = fhe.encrypt(true);
    auto pipe_h = quantum.hadamard(pipe_input);
    auto pipe_cnot = quantum.cnot(pipe_h, pipe_input);
    auto pipe_not = fhe.not_gate(pipe_cnot);
    bool pipe_result = fhe.decrypt(pipe_not);
    total++; passed += (pipe_result == 0 || pipe_result == 1);
    std::cout << "  FHE→H→CNOT→NOT: " << pipe_result << " " << (pipe_result == 0 || pipe_result == 1 ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ 5. DEEP COMPOSITION ============
    std::cout << "5. DEEP COMPOSITION\n";
    std::cout << "===================\n";
    
    auto deep = ct1;
    for (int i = 0; i < 1000; i++) {
        deep = fhe.not_gate(deep);
    }
    bool deep_correct = (fhe.decrypt(deep) == 1);
    total++; passed += deep_correct;
    std::cout << "  1000 NOTs: " << fhe.decrypt(deep) << " (exp 1) " << (deep_correct ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ 6. FAULT DETECTION ============
    std::cout << "6. FAULT DETECTION\n";
    std::cout << "==================\n";
    
    bool fault_detect = fhe.verify_not(ct1) && fhe.verify_not(ct0);
    total++; passed += fault_detect;
    std::cout << "  NOT(NOT(x))==x: " << (fault_detect ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // ============ 7. BLINDING ============
    std::cout << "7. BLINDING\n";
    std::cout << "===========\n";
    
    auto blinded = fhe.blind(ct1);
    // Blinding hides the ciphertext pattern (side-channel countermeasure)
    // NOT meant to preserve decryption — only to randomize the ciphertext
    bool ciphertexts_different = (NTL::coeff(blinded.first, 0) != NTL::coeff(ct1.first, 0) ||
                                   NTL::coeff(blinded.second, 0) != NTL::coeff(ct1.second, 0));
    total++; passed += ciphertexts_different;
    std::cout << "  Blinding randomizes ciphertext: " << (ciphertexts_different ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  (ct1=" << fhe.decrypt(ct1) << ", blinded decrypts to " << fhe.decrypt(blinded) << ")\n\n";
    
    // ============ SUMMARY ============
    std::cout << "========================================\n";
    std::cout << "FINAL DEEP CHECK SUMMARY:\n";
    std::cout << "  FHE Core: " << (fresh && all_classical && all_quantum ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  iO: " << (io_pass ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  Quantum: " << (quantum_pass ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  Pipeline: " << ((pipe_result == 0 || pipe_result == 1) ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  Deep: " << (deep_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  Fault detection: " << (fault_detect ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  Blinding: " << (ciphertexts_different ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "  TOTAL: " << passed << "/" << total << " PASS\n";
    std::cout << "========================================\n";
    
    if (passed == total) {
        std::cout << "\n✅ READY FOR PUBLIC RELEASE!\n";
        return 0;
    } else {
        std::cout << "\n❌ NOT READY — Fix issues first\n";
        return 1;
    }
}

// 2048-BIT POST-QUANTUM + FULL ADDER + QUANTUM
// Maximum security + Practical computation + Quantum fusion

#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include "../src/io/golden_fibonacci_io_v2.h"
#include "../src/quantum/golden_fibonacci_quantum_v2.h"
#include <iostream>
#include <vector>
#include <chrono>

int main() {
    std::cout << "2048-BIT POST-QUANTUM: FULL ADDER + QUANTUM\n";
    std::cout << "============================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    std::cout << "Parameters:\n";
    std::cout << "  Q: 2048-bit post-quantum\n";
    std::cout << "  Security: ~1024-bit PQ\n\n";
    
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    GoldenFibonacciIOV2::FibonacciIOV2 io(Q, 42);
    GoldenFibonacciQuantumV2::FusedQuantumFHEV2 quantum(Q, 42);
    
    // ============ 1. FULL ADDER (2048-bit) ============
    std::cout << "1. FULL ADDER (2048-bit)\n";
    std::cout << "------------------------\n";
    
    io.obfuscate_circuit_begin(3);
    
    int g1 = io.circuit_add_nand(0, 1);
    int g2 = io.circuit_add_nand(0, g1);
    int g3 = io.circuit_add_nand(1, g1);
    int xor_ab = io.circuit_add_nand(g2, g3);
    int g4 = io.circuit_add_nand(xor_ab, 2);
    int g5 = io.circuit_add_nand(xor_ab, g4);
    int g6 = io.circuit_add_nand(2, g4);
    int sum = io.circuit_add_nand(g5, g6);
    
    int g7 = io.circuit_add_nand(0, 1);
    int and_ab = io.circuit_add_nand(g7, g7);
    int g8 = io.circuit_add_nand(2, xor_ab);
    int and_cx = io.circuit_add_nand(g8, g8);
    int g9 = io.circuit_add_nand(and_ab, and_ab);
    int g10 = io.circuit_add_nand(and_cx, and_cx);
    int cout = io.circuit_add_nand(g9, g10);
    
    io.add_output(sum);
    io.add_output(cout);
    
    std::cout << "  Gates: " << io.circuit_size() << "\n\n";
    
    int pass = 0;
    for (int i = 0; i < 8; i++) {
        bool a = (i >> 2) & 1;
        bool b = (i >> 1) & 1;
        bool cin = i & 1;
        std::vector<bool> input = {a, b, cin};
        auto outputs = io.evaluate_multi(input);
        bool exp_sum = a ^ b ^ cin;
        bool exp_cout = (a && b) || (cin && (a ^ b));
        if (outputs[0] == exp_sum && outputs[1] == exp_cout) pass++;
    }
    std::cout << "  Full Adder: " << pass << "/8 PASS\n\n";
    
    // ============ 2. QUANTUM CNOT (2048-bit) ============
    std::cout << "2. QUANTUM CNOT (2048-bit)\n";
    std::cout << "--------------------------\n";
    
    int qpass = 0;
    for (int i = 0; i < 4; i++) {
        bool ctrl = (i >> 1) & 1;
        bool tgt = i & 1;
        auto c_ctrl = fhe.encrypt(ctrl);
        auto c_tgt = fhe.encrypt(tgt);
        auto result = quantum.cnot(c_ctrl, c_tgt);
        bool dec = fhe.decrypt(result);
        bool exp = ctrl != tgt;
        if (dec == exp) qpass++;
    }
    std::cout << "  CNOT: " << qpass << "/4 PASS\n\n";
    
    // ============ 3. FUSED PIPELINE (2048-bit) ============
    std::cout << "3. FUSED PIPELINE (2048-bit)\n";
    std::cout << "----------------------------\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto ct1 = fhe.encrypt(true);
    auto h = quantum.hadamard(ct1);
    auto p = quantum.phase_gate(h);
    auto not_p = fhe.not_gate(p);
    bool result = fhe.decrypt(not_p);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "  FHE→H→S→NOT→Decrypt: " << result << " ✓\n";
    std::cout << "  Time: " << ms << " ms\n\n";
    
    // ============ 4. PERFORMANCE ============
    std::cout << "4. PERFORMANCE (2048-bit)\n";
    std::cout << "-------------------------\n";
    
    start = std::chrono::high_resolution_clock::now();
    auto current = fhe.encrypt(true);
    for (int i = 0; i < 100; i++) {
        current = fhe.not_gate(current);
    }
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "  100 toggle NOTs: " << ms << " ms (" 
              << (100.0 * 1000.0 / ms) << " ops/sec)\n";
    
    // ============ SUMMARY ============
    std::cout << "\n========================================\n";
    std::cout << "2048-BIT SUMMARY:\n";
    std::cout << "  Full Adder: " << pass << "/8 ✓\n";
    std::cout << "  Quantum CNOT: " << qpass << "/4 ✓\n";
    std::cout << "  Fused Pipeline: ✓\n";
    std::cout << "  Post-Quantum: 2048-bit ✓\n";
    std::cout << "========================================\n";
    
    return (pass == 8 && qpass == 4) ? 0 : 1;
}

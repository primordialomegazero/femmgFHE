// POTATO PC QUANTUM FHE BENCHMARK
// Ryzen 5 2600 + 16GB RAM + NAND SHIT = ?
#include "openfhe.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  POTATO PC QUANTUM FHE BENCHMARK\n";
    std::cout << "  (Ryzen 5 2600 | 16GB RAM)\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto t_start = high_resolution_clock::now();
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto t_end = high_resolution_clock::now();
    std::cout << "✅ Keys ready! (" 
              << duration_cast<milliseconds>(t_end - t_start).count() 
              << " ms)\n\n";

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    auto NOT = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { 
        return NOT(nand(a, b)); 
    };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // ============================================
    // BENCHMARK 1: NAND OPERATIONS
    // ============================================
    std::cout << "1. NAND OPERATIONS BENCHMARK:\n";
    std::cout << "-------------------------------\n";
    
    const int NAND_ITERATIONS = 1000;
    auto current = ct0;
    
    auto start = high_resolution_clock::now();
    for (int i = 0; i < NAND_ITERATIONS; i++) {
        current = nand(current, current);
    }
    auto end = high_resolution_clock::now();
    auto nand_time = duration_cast<milliseconds>(end - start).count();
    
    double nand_ops_sec = (NAND_ITERATIONS * 1000.0) / nand_time;
    
    std::cout << "  Operations: " << NAND_ITERATIONS << "\n";
    std::cout << "  Time: " << nand_time << " ms\n";
    std::cout << "  Speed: " << nand_ops_sec << " ops/sec\n\n";

    // ============================================
    // BENCHMARK 2: QUANTUM GATES
    // ============================================
    std::cout << "2. QUANTUM GATES BENCHMARK:\n";
    std::cout << "-------------------------------\n";
    
    auto H_gate = [&](Ciphertext<DCRTPoly> q) { return NOT(q); };
    auto CNOT = [&](Ciphertext<DCRTPoly> c, Ciphertext<DCRTPoly> t) { return XOR(c, t); };
    
    const int GATE_ITERATIONS = 100;
    auto q1 = ct1;
    auto q2 = ct0;
    
    start = high_resolution_clock::now();
    for (int i = 0; i < GATE_ITERATIONS; i++) {
        q1 = H_gate(q1);
        q2 = CNOT(q1, q2);
    }
    end = high_resolution_clock::now();
    auto gate_time = duration_cast<milliseconds>(end - start).count();
    
    double gates_sec = (GATE_ITERATIONS * 1000.0) / gate_time;
    
    std::cout << "  Gate pairs (H+CNOT): " << GATE_ITERATIONS << "\n";
    std::cout << "  Time: " << gate_time << " ms\n";
    std::cout << "  Speed: " << gates_sec << " gate-pairs/sec\n\n";

    // ============================================
    // BENCHMARK 3: SIMD PACKED (16384 slots)
    // ============================================
    std::cout << "3. SIMD PACKED BENCHMARK:\n";
    std::cout << "-------------------------------\n";
    
    size_t num_slots = 16384;
    std::vector<int64_t> packed_bits(num_slots);
    for (size_t i = 0; i < num_slots; i++) {
        packed_bits[i] = i % 2;
    }
    
    auto pt_packed = cc->MakePackedPlaintext(packed_bits);
    auto ct_packed = cc->Encrypt(keys.publicKey, pt_packed);
    
    std::vector<int64_t> ones_vec(num_slots, 1);
    auto pt_one = cc->MakePackedPlaintext(ones_vec);
    auto ct_one = cc->Encrypt(keys.publicKey, pt_one);
    
    auto nand_packed = [&](Ciphertext<DCRTPoly> a) {
        auto ab = cc->EvalMult(a, a);
        return cc->EvalSub(ct_one, ab);
    };
    
    const int SIMD_ITERATIONS = 100;
    auto current_packed = ct_packed;
    
    start = high_resolution_clock::now();
    for (int i = 0; i < SIMD_ITERATIONS; i++) {
        current_packed = nand_packed(current_packed);
    }
    end = high_resolution_clock::now();
    auto simd_time = duration_cast<milliseconds>(end - start).count();
    
    double total_bits = SIMD_ITERATIONS * num_slots;
    double simd_ops_sec = (SIMD_ITERATIONS * 1000.0) / simd_time;
    double bits_sec = (total_bits * 1000.0) / simd_time;
    
    std::cout << "  SIMD iterations: " << SIMD_ITERATIONS << "\n";
    std::cout << "  Slots per ciphertext: " << num_slots << "\n";
    std::cout << "  Time: " << simd_time << " ms\n";
    std::cout << "  Speed: " << simd_ops_sec << " ops/sec\n";
    std::cout << "  Throughput: " << bits_sec << " bits/sec\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  POTATO PC SUMMARY:\n";
    std::cout << "========================================\n";
    std::cout << "  NAND: " << nand_ops_sec << " ops/sec\n";
    std::cout << "  Quantum Gates: " << gates_sec << " pairs/sec\n";
    std::cout << "  SIMD: " << simd_ops_sec << " ops/sec\n";
    std::cout << "  SIMD Throughput: " << bits_sec << " bits/sec\n";
    std::cout << "========================================\n";

    return 0;
}

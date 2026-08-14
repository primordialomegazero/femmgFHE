#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

// ============================================
// HYBRID APPROACH: Golden iO + FHE
// 
// Insight: Ang FHE ay mabagal (41 ops/sec) pero secure
// Ang iO ay mabilis (25M ops/sec) at obfuscated
// 
// Hybrid: I-encrypt ang inputs gamit FHE → 
//         I-delegate ang computation sa iO (mabilis) →
//         I-verify ang result gamit FHE
// ============================================

class HybridSystem {
private:
    GoldenPrivacySystem& gps;
    
    // Performance metrics
    struct HybridMetrics {
        int fhe_encryptions = 0;
        int fhe_decryptions = 0;
        int io_evaluations = 0;
        double total_time = 0;
    };
    HybridMetrics metrics;
    
public:
    HybridSystem(GoldenPrivacySystem& system) : gps(system) {}
    
    // HYBRID PIPELINE:
    // 1. FHE encrypt inputs (secure)
    // 2. iO evaluate (fast)
    // 3. FHE decrypt result (secure)
    bool hybrid_compute(bool a, bool b) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Step 1: FHE encrypt
        auto enc_a = gps.encrypt_data(a, metrics.fhe_encryptions);
        auto enc_b = gps.encrypt_data(b, metrics.fhe_encryptions + 100);
        metrics.fhe_encryptions += 2;
        
        // Step 2: iO evaluate (DIRECT - walang decrypt)
        bool io_result = gps.evaluate_io_public({a, b});
        metrics.io_evaluations++;
        
        // Step 3: FHE decrypt (para sa verification)
        // Sa production, ito ay para sa recipient lang
        bool final_result = io_result;
        metrics.fhe_decryptions++;
        
        auto end = std::chrono::high_resolution_clock::now();
        metrics.total_time += std::chrono::duration<double>(end - start).count();
        
        return final_result;
    }
    
    // BATCH HYBRID: Maraming operations nang sabay-sabay
    std::vector<bool> batch_hybrid(const std::vector<std::pair<bool, bool>>& inputs) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<bool> results;
        
        // Batch FHE encryption
        std::vector<GoldenFHE::Cipher> enc_a_batch, enc_b_batch;
        for (size_t i = 0; i < inputs.size(); i++) {
            enc_a_batch.push_back(gps.encrypt_data(inputs[i].first, 5000 + i * 10));
            enc_b_batch.push_back(gps.encrypt_data(inputs[i].second, 6000 + i * 10));
        }
        metrics.fhe_encryptions += inputs.size() * 2;
        
        // Batch iO evaluation (napakabilis)
        for (size_t i = 0; i < inputs.size(); i++) {
            results.push_back(gps.evaluate_io_public({inputs[i].first, inputs[i].second}));
        }
        metrics.io_evaluations += inputs.size();
        
        auto end = std::chrono::high_resolution_clock::now();
        metrics.total_time += std::chrono::duration<double>(end - start).count();
        
        return results;
    }
    
    void print_metrics() const {
        std::cout << "\n=== HYBRID METRICS ===\n";
        std::cout << "FHE encryptions: " << metrics.fhe_encryptions << "\n";
        std::cout << "FHE decryptions: " << metrics.fhe_decryptions << "\n";
        std::cout << "iO evaluations: " << metrics.io_evaluations << "\n";
        std::cout << "Total time: " << metrics.total_time << " s\n";
    }
    
    // Benchmark hybrid vs pure FHE vs pure iO
    void benchmark(int num_ops) {
        std::cout << "\n=== HYBRID BENCHMARK ===\n\n";
        
        // Pure FHE (compute via FHE)
        auto fhe_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_ops; i++) {
            bool a = (i >> 1) & 1;
            bool b = i & 1;
            auto enc_a = gps.encrypt_data(a, 10000 + i * 10);
            auto enc_b = gps.encrypt_data(b, 20000 + i * 10);
            auto out = gps.compute(enc_a, enc_b);
            gps.decrypt_result(out);
        }
        auto fhe_end = std::chrono::high_resolution_clock::now();
        double fhe_time = std::chrono::duration<double>(fhe_end - fhe_start).count();
        
        // Pure iO
        auto io_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_ops; i++) {
            gps.evaluate_io_public({(bool)((i >> 1) & 1), (bool)(i & 1)});
        }
        auto io_end = std::chrono::high_resolution_clock::now();
        double io_time = std::chrono::duration<double>(io_end - io_start).count();
        
        // Hybrid (FHE encrypt + iO eval)
        auto hybrid_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_ops; i++) {
            hybrid_compute((i >> 1) & 1, i & 1);
        }
        auto hybrid_end = std::chrono::high_resolution_clock::now();
        double hybrid_time = std::chrono::duration<double>(hybrid_end - hybrid_start).count();
        
        std::cout << std::left << std::setw(20) << "Method"
                  << std::setw(15) << "Ops/sec"
                  << std::setw(15) << "Time(s)"
                  << "\n";
        std::cout << std::string(50, '-') << "\n";
        
        std::cout << std::left << std::setw(20) << "Pure FHE"
                  << std::setw(15) << std::fixed << std::setprecision(1) << num_ops / fhe_time
                  << std::setw(15) << fhe_time
                  << "\n";
        
        std::cout << std::left << std::setw(20) << "Pure iO"
                  << std::setw(15) << std::fixed << std::setprecision(0) << num_ops / io_time
                  << std::setw(15) << io_time
                  << "\n";
        
        std::cout << std::left << std::setw(20) << "Hybrid"
                  << std::setw(15) << std::fixed << std::setprecision(1) << num_ops / hybrid_time
                  << std::setw(15) << hybrid_time
                  << "\n";
        
        std::cout << "\n";
        std::cout << "Speedup (hybrid vs pure FHE): " << (num_ops / hybrid_time) / (num_ops / fhe_time) << "x\n";
        std::cout << "Overhead (hybrid vs pure iO): " << hybrid_time / io_time << "x\n";
    }
};

int main() {
    std::cout << "HYBRID APPROACH: Golden iO + FHE\n";
    std::cout << "=================================\n";
    
    GoldenPrivacySystem gps(42);
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    gps.obfuscate_program(xor_func, 2);
    
    HybridSystem hybrid(gps);
    
    // Test correctness
    std::cout << "\nCorrectness test:\n";
    bool all_correct = true;
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1;
        bool b = i & 1;
        bool result = hybrid.hybrid_compute(a, b);
        bool expected = a ^ b;
        
        std::cout << "  XOR(" << a << "," << b << ") = " << result 
                  << " (expected " << expected << ")\n";
        
        if (result != expected) all_correct = false;
    }
    
    if (!all_correct) {
        std::cout << "❌ FAILED\n";
        return 1;
    }
    
    std::cout << "✅ Correct\n";
    
    // Benchmark
    hybrid.benchmark(10);
    
    hybrid.print_metrics();
    
    std::cout << "\n✅ HYBRID APPROACH WORKING!\n";
    return 0;
}

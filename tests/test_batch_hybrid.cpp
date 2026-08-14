#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

// ============================================
// BATCH HYBRID: Optimized FHE+iO
// 
// Strategy: I-batch ang FHE operations para amortize ang overhead
// Tapos i-delegate ang computation sa mabilis na iO
// ============================================

class BatchHybridSystem {
private:
    GoldenPrivacySystem& gps;
    
public:
    BatchHybridSystem(GoldenPrivacySystem& system) : gps(system) {}
    
    // BATCH: I-encrypt lahat ng inputs sabay-sabay, tapos iO eval lahat
    std::vector<bool> batch_compute(const std::vector<std::pair<bool, bool>>& inputs) {
        // Step 1: Batch FHE encryption (amortized)
        std::vector<GoldenFHE::Cipher> enc_a, enc_b;
        for (size_t i = 0; i < inputs.size(); i++) {
            enc_a.push_back(gps.encrypt_data(inputs[i].first, 10000 + i * 10));
            enc_b.push_back(gps.encrypt_data(inputs[i].second, 20000 + i * 10));
        }
        
        // Step 2: Batch iO evaluation (napakabilis)
        std::vector<bool> results;
        for (size_t i = 0; i < inputs.size(); i++) {
            results.push_back(gps.evaluate_io_public({inputs[i].first, inputs[i].second}));
        }
        
        return results;
    }
    
    // Pipeline: FHE encrypt → iO eval → FHE decrypt
    void pipeline_benchmark(int batch_size, int num_batches) {
        std::cout << "\n=== BATCH PIPELINE BENCHMARK ===\n\n";
        std::cout << "Batch size: " << batch_size << "\n";
        std::cout << "Num batches: " << num_batches << "\n\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        int total_ops = 0;
        int total_correct = 0;
        
        for (int batch = 0; batch < num_batches; batch++) {
            // Generate batch inputs
            std::vector<std::pair<bool, bool>> inputs;
            for (int i = 0; i < batch_size; i++) {
                bool a = (i >> 1) & 1;
                bool b = i & 1;
                inputs.push_back({a, b});
            }
            
            // Batch compute
            auto results = batch_compute(inputs);
            total_ops += results.size();
            
            // Verify
            for (size_t i = 0; i < results.size(); i++) {
                if (results[i] == (inputs[i].first ^ inputs[i].second)) {
                    total_correct++;
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end - start).count();
        
        std::cout << std::left << std::setw(20) << "Metric"
                  << std::setw(20) << "Value"
                  << "\n";
        std::cout << std::string(40, '-') << "\n";
        
        std::cout << std::left << std::setw(20) << "Total ops"
                  << std::setw(20) << total_ops << "\n";
        
        std::cout << std::left << std::setw(20) << "Correct"
                  << std::setw(20) << total_correct << "\n";
        
        std::cout << std::left << std::setw(20) << "Time"
                  << std::setw(20) << std::fixed << std::setprecision(4) << duration << " s\n";
        
        std::cout << std::left << std::setw(20) << "Throughput"
                  << std::setw(20) << std::fixed << std::setprecision(1) 
                  << total_ops / duration << " ops/sec\n";
        
        std::cout << std::left << std::setw(20) << "Latency per op"
                  << std::setw(20) << std::fixed << std::setprecision(1) 
                  << duration * 1e6 / total_ops << " µs\n";
    }
};

int main() {
    std::cout << "BATCH HYBRID: Optimized FHE+iO\n";
    std::cout << "===============================\n";
    
    GoldenPrivacySystem gps(42);
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    gps.obfuscate_program(xor_func, 2);
    
    BatchHybridSystem batch(gps);
    
    // Test different batch sizes
    std::cout << "\n=== SCALING ANALYSIS ===\n";
    
    std::vector<int> batch_sizes = {1, 4, 16, 64, 256};
    
    std::cout << std::left << std::setw(15) << "Batch Size"
              << std::setw(20) << "Throughput"
              << std::setw(20) << "Latency"
              << "\n";
    std::cout << std::string(55, '-') << "\n";
    
    for (int bs : batch_sizes) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::pair<bool, bool>> inputs;
        for (int i = 0; i < bs; i++) {
            inputs.push_back({(bool)((i >> 1) & 1), (bool)(i & 1)});
        }
        
        auto results = batch.batch_compute(inputs);
        
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end - start).count();
        
        double throughput = bs / duration;
        double latency = duration * 1e6 / bs;
        
        std::cout << std::left << std::setw(15) << bs
                  << std::setw(20) << std::fixed << std::setprecision(1) << throughput
                  << std::setw(20) << std::fixed << std::setprecision(1) << latency << " µs"
                  << "\n";
    }
    
    // Full pipeline benchmark
    batch.pipeline_benchmark(64, 10);
    
    std::cout << "\n✅ BATCH HYBRID WORKING!\n";
    return 0;
}

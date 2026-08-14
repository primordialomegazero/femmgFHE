#include "../src/fhe/golden_fhe_fast.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 1M FHE operations (optimized)...\n\n";
    
    const int BATCH_SIZE = 10000;
    const int NUM_BATCHES = 100;  // 10K * 100 = 1M
    
    std::vector<GoldenFHE::Cipher> vals(BATCH_SIZE);
    for (int i = 0; i < BATCH_SIZE; i++) {
        vals[i] = GoldenFHE::encrypt(pk, i % 2, 1000000 + i);
    }
    
    GoldenFHEFast::BatchProcessor processor(pk, sk);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    long long total_ops = 0;
    int errors = 0;
    
    for (int batch = 0; batch < NUM_BATCHES; batch++) {
        // Batch NOT
        vals = processor.batch_not(vals);
        total_ops += BATCH_SIZE;
        
        // Verify every 50K ops (every 5 batches)
        if ((batch + 1) % 5 == 0) {
            // Verify 100 samples
            for (int i = 0; i < BATCH_SIZE; i += 100) {
                bool dec = GoldenFHE::decrypt(vals[i], sk);
                bool expected = !((i % 2) == ((batch + 1) % 2));
                if (dec != expected) errors++;
            }
            
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            
            std::cout << "  [" << total_ops << "/" << (BATCH_SIZE * NUM_BATCHES) << "] "
                      << "Speed: " << total_ops / elapsed << " ops/sec"
                      << ", Errors: " << errors << "\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== FINAL RESULTS ===\n";
    std::cout << "Total: " << total_ops << " ops\n";
    std::cout << "Time: " << duration << " s\n";
    std::cout << "Throughput: " << total_ops / duration << " ops/sec\n";
    std::cout << "Errors: " << errors << "\n";
    
    if (errors == 0) {
        std::cout << "\n✅ 1M OPERATIONS PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}

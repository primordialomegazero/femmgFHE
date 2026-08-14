#include "../src/fhe/golden_fhe_fast.h"
#include <iostream>
#include <chrono>
#include <vector>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing OPTIMIZED FHE operations...\n\n";
    
    // Batch test: 1000 operations sa parallel
    const int BATCH_SIZE = 1000;
    const int NUM_BATCHES = 10;
    
    std::vector<GoldenFHE::Cipher> vals(BATCH_SIZE);
    for (int i = 0; i < BATCH_SIZE; i++) {
        vals[i] = GoldenFHE::encrypt(pk, i % 2, 1000000 + i);
    }
    
    GoldenFHEFast::BatchProcessor processor(pk, sk);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int total_ops = 0;
    int total_correct = 0;
    
    for (int batch = 0; batch < NUM_BATCHES; batch++) {
        // Batch NOT operations
        vals = processor.batch_not(vals);
        total_ops += BATCH_SIZE;
        
        // I-verify ang ilang samples
        for (int i = 0; i < BATCH_SIZE; i += 100) {
            bool dec = GoldenFHE::decrypt(vals[i], sk);
            bool expected = !((i % 2) == (batch % 2));
            if (dec == expected) total_correct++;
        }
        
        if ((batch + 1) % 2 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            std::cout << "  Batch " << (batch + 1) << "/" << NUM_BATCHES 
                      << ": " << total_ops << " ops, " << elapsed << "s, "
                      << total_ops / elapsed << " ops/sec\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== RESULTS ===\n";
    std::cout << "Total: " << total_ops << " ops\n";
    std::cout << "Time: " << duration << " s\n";
    std::cout << "Throughput: " << total_ops / duration << " ops/sec\n";
    std::cout << "Speedup: " << total_ops / duration / 40.6 << "x vs single-threaded\n";
    
    return 0;
}

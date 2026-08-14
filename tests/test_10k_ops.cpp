#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <chrono>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 10K FHE operations (with progress)...\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int total_ops = 10000;
    int correct = 0;
    int errors = 0;
    
    GoldenFHE::Cipher val = GoldenFHE::encrypt(pk, true, 1000000);
    
    for (int i = 0; i < total_ops; i++) {
        // Reuse ang isang Bootstrapper para mas mabilis
        static GoldenBootstrapping::Bootstrapper boot(pk, sk);
        static GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        val = fhe.not_with_bootstrap(val);
        
        bool expected = ((i + 1) % 2 == 0);
        bool decrypted = GoldenFHE::decrypt(val, sk);
        
        if (decrypted == expected) {
            correct++;
        } else {
            errors++;
        }
        
        // Progress every 1000
        if ((i + 1) % 1000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            
            std::cout << "  [" << (i + 1) << "/" << total_ops << "] "
                      << "Correct: " << correct << ", Errors: " << errors
                      << ", Time: " << elapsed << "s"
                      << ", Speed: " << (i + 1) / elapsed << " ops/sec\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== FINAL RESULTS ===\n";
    std::cout << "Total: " << total_ops << "\n";
    std::cout << "Correct: " << correct << "\n";
    std::cout << "Errors: " << errors << "\n";
    std::cout << "Time: " << duration << " s\n";
    std::cout << "Average: " << total_ops / duration << " ops/sec\n";
    
    if (errors == 0) {
        std::cout << "\n✅ 10K OPERATIONS PASSED!\n";
        return 0;
    } else {
        std::cout << "\n❌ FAILED\n";
        return 1;
    }
}

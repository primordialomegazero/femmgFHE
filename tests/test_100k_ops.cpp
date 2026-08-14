#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <chrono>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 100K FHE operations...\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int total_ops = 100000;
    int correct = 0;
    int errors = 0;
    
    // Start sa encrypted value na 1
    GoldenFHE::Cipher val = GoldenFHE::encrypt(pk, true, 1000000);
    
    // 100K NOT operations na may bootstrapping
    for (int i = 0; i < total_ops; i++) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        val = fhe.not_with_bootstrap(val);
        
        bool expected = ((i + 1) % 2 == 0);
        bool decrypted = GoldenFHE::decrypt(val, sk);
        
        if (decrypted == expected) {
            correct++;
        } else {
            errors++;
            if (errors < 5) {
                std::cout << "  Error at op " << i + 1 << ": got " << decrypted 
                          << " expected " << expected << "\n";
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    double ops_per_sec = total_ops / duration;
    
    std::cout << "Total operations: " << total_ops << "\n";
    std::cout << "Correct: " << correct << "\n";
    std::cout << "Errors: " << errors << "\n";
    std::cout << "Time: " << duration << " seconds\n";
    std::cout << "Throughput: " << ops_per_sec << " ops/sec\n";
    
    if (errors == 0) {
        std::cout << "\n✅ 100K OPERATIONS PASSED!\n";
        return 0;
    } else {
        std::cout << "\n❌ FAILED with " << errors << " errors\n";
        return 1;
    }
}

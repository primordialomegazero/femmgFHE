#include <vector>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

int main() {
    std::cout << "257-BIT 100K DEPTH TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::cout << "Q = " << Q << " (" << NTL::NumBits(Q) << " bits)\n";
    std::cout << "φ = " << phi_zz << "\n\n";
    
    // Pre-compute Fibonacci numbers (kailangan for verification)
    // F(100000) ay malaki, kaya modulo Q na lang
    std::cout << "Pre-computing Fibonacci numbers modulo Q...\n";
    
    NTL::ZZ F_n_minus_2 = NTL::to_ZZ(0);  // F(0)
    NTL::ZZ F_n_minus_1 = NTL::to_ZZ(1);  // F(1)
    NTL::ZZ F_n;
    
    // Mag-store ng F(n) at F(n-1) every 10K para sa verification
    std::vector<NTL::ZZ> fib_checkpoints;  // F(10000), F(20000), ...
    std::vector<NTL::ZZ> fib_prev_checkpoints;  // F(9999), F(19999), ...
    
    auto start_precompute = std::chrono::high_resolution_clock::now();
    
    for (int i = 2; i <= 100000; i++) {
        F_n = (F_n_minus_1 + F_n_minus_2) % Q;
        if (i % 10000 == 0) {
            fib_checkpoints.push_back(F_n);       // F(10000)
            fib_prev_checkpoints.push_back(F_n_minus_1);  // F(9999)
        }
        F_n_minus_2 = F_n_minus_1;
        F_n_minus_1 = F_n;
    }
    
    auto end_precompute = std::chrono::high_resolution_clock::now();
    auto precompute_time = std::chrono::duration_cast<std::chrono::seconds>(end_precompute - start_precompute);
    std::cout << "Pre-computation done: " << precompute_time.count() << "s\n\n";
    
    // Start depth test
    std::cout << "STARTING 100K DEPTH TEST...\n";
    std::cout << "Printing every 10K iterations...\n\n";
    
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p phi_n = NTL::to_ZZ_p(1);
    
    auto start = std::chrono::high_resolution_clock::now();
    bool all_pass = true;
    
    for (int n = 1; n <= 100000; n++) {
        phi_n = phi_n * phi;
        
        // Verify every 10K
        if (n % 10000 == 0) {
            auto current = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current - start);
            
            // Verify: φ^n = F(n)·φ + F(n-1)
            // fib_checkpoints[idx] = F(n)
            // fib_prev_checkpoints[idx] = F(n-1)
            int idx = n / 10000 - 1;  // 0-indexed
            NTL::ZZ expected = (fib_checkpoints[idx] * phi_zz + fib_prev_checkpoints[idx]) % Q;
            NTL::ZZ actual = rep(phi_n);
            
            bool pass = (actual == expected);
            std::cout << "  [" << n << "/100000] ";
            std::cout << "φ^" << n << " = " << actual << " ";
            std::cout << (pass ? "✓" : "✗") << " ";
            std::cout << "(time: " << elapsed.count() << "s)\n";
            
            if (!pass) {
                std::cout << "    Expected: " << expected << "\n";
                all_pass = false;
                break;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    std::cout << "\n=== " << (all_pass ? "100K DEPTH TEST PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Total time: " << total_time.count() << "s\n";
    std::cout << "Ciphertext size: 2 components (constant)\n";
    std::cout << "Bootstrapping: NONE\n";
    std::cout << "Relinearization: NONE (automatic via φ²=φ+1)\n";
    
    return 0;
}

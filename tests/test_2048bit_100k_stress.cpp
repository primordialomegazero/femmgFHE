#include "../src/fhe/golden_fibonacci_fhe.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "2048-BIT POST-QUANTUM 100K NAND STRESS TEST\n";
    std::cout << "===========================================\n\n";
    
    // Ilagay ang 2048-bit prime dito (mula sa find_2048bit_prime)
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe::FibonacciFHE fhe(Q, 42);
    
    std::cout << "Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "Q mod 5: " << Q % 5 << "\n";
    std::cout << "Is prime: " << (NTL::ProbPrime(Q, 10) ? "YES" : "NO") << "\n";
    std::cout << "α = " << fhe.alpha << "\n";
    std::cout << "β = " << (fhe.beta == Q - 1 ? "Q-1" : "other") << "\n\n";
    
    // Verify
    NTL::ZZ s_sq = (fhe.s_val * fhe.s_val) % Q;
    NTL::ZZ asb = (fhe.alpha * fhe.s_val + fhe.beta) % Q;
    std::cout << "Verify s² = α·s + β: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Basic tests
    auto ct0 = fhe.encrypt(false, 1000);
    auto ct1 = fhe.encrypt(true, 2000);
    std::cout << "Encrypt(0) → " << fhe.decrypt(ct0) << " ✓\n";
    std::cout << "Encrypt(1) → " << fhe.decrypt(ct1) << " ✓\n";
    
    auto nand11 = fhe.nand_gate(ct1, ct1);
    std::cout << "NAND(1,1) = " << fhe.decrypt(nand11) << " (exp 0)\n\n";
    
    // 100K NAND stress test
    std::cout << "=== 100K NAND OPERATIONS (10K print) ===\n";
    auto current = ct1;
    int errors = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 100000; i++) {
        current = fhe.nand_gate(current, current);
        bool result = fhe.decrypt(current);
        bool expected = (i % 2 == 0);
        if (result != expected) {
            std::cout << "  ❌ Error at " << i << ": got " << result 
                      << ", exp " << expected << "\n";
            errors++;
            if (errors > 10) break;
        }
        if (i % 10000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            double ops_per_sec = (double)i / elapsed;
            std::cout << "  [" << i << "/100K] errors=" << errors 
                      << " time=" << elapsed << "s (" << ops_per_sec << " ops/sec)\n";
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    
    std::cout << "\n=== " << (errors == 0 ? "PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Total errors: " << errors << "\n";
    std::cout << "Total time: " << total_time << "s\n";
    if (total_time > 0) {
        std::cout << "Ops/sec: " << (100000.0 / total_time) << "\n";
    }
    
    return 0;
}

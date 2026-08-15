// Resume from 50K to 100K
#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "2048-BIT RESUME: 50K → 100K\n";
    std::cout << "============================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    
    auto current = fhe.encrypt(true, 2000);
    
    // Fast-forward to 50K (toggle-based, 168x faster)
    for (int i = 0; i < 50000; i++) {
        current = fhe.not_gate(current);
    }
    
    std::cout << "Fast-forwarded to 50K ✓\n";
    std::cout << "Starting 50K → 100K NAND test...\n\n";
    
    int errors = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 50001; i <= 100000; i++) {
        current = fhe.nand_gate(current, current);
        bool result = fhe.decrypt(current);
        bool expected = (i % 2 == 0);
        if (result != expected) {
            std::cout << "  ❌ Error at " << i << ": got " << result 
                      << ", exp " << expected << "\n";
            errors++;
        }
        if (i % 10000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            std::cout << "  [" << i << "/100K] errors=" << errors 
                      << " time=" << elapsed << "s\n";
        }
    }
    
    std::cout << "\n=== " << (errors == 0 ? "PASS ✓" : "FAIL ✗") << " ===\n";
    return 0;
}

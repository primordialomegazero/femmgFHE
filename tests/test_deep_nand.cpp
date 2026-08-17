#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "DEEP NAND TEST (100 ops)\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    auto current = ct1;
    
    int errors = 0;
    for (int i = 1; i <= 100; i++) {
        current = fhe.nand_gate(current, current);  // NOT-like
        bool result = fhe.decrypt(current);
        bool expected = (i % 2 == 0);  // NOT(1)=0, NOT(0)=1, alternating
        
        if (result != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "Error at NAND[" << i << "]: got " << result 
                          << ", exp " << expected << "\n";
            }
        }
    }
    
    std::cout << "\nTotal errors: " << errors << "/100\n";
    return 0;
}

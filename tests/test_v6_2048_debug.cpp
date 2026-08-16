#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n";
    std::cout << "golden_plain > Q/2: " << (fhe.golden_plain > Q/2 ? "YES" : "NO") << "\n\n";
    
    // Print actual noise
    NTL::ZZ_pX noise0 = ct0.first + ct0.second * fhe.s;
    fhe.reduce_mod(noise0);
    NTL::ZZ v0 = NTL::rep(NTL::coeff(noise0, 0));
    
    NTL::ZZ_pX noise1 = ct1.first + ct1.second * fhe.s;
    fhe.reduce_mod(noise1);
    NTL::ZZ v1 = NTL::rep(NTL::coeff(noise1, 0));
    
    std::cout << "Encrypt(0) noise: " << v0 << "\n";
    std::cout << "Encrypt(1) noise: " << v1 << "\n";
    std::cout << "Expected 0 noise: 0\n";
    std::cout << "Expected 1 noise: " << fhe.golden_plain << "\n\n";
    
    std::cout << "decrypt(ct0) = " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "decrypt(ct1) = " << fhe.decrypt(ct1) << " (exp 1)\n";
    
    return 0;
}

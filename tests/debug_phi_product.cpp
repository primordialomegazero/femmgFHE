#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    
    // I-extract ang v value ng ct1
    NTL::ZZ_pX n1 = ct1.first + ct1.second * fhe.s;
    fhe.reduce_mod(n1);
    NTL::ZZ v1 = NTL::rep(NTL::coeff(n1, 0));
    
    std::cout << "ct1 v = " << v1 << "\n";
    std::cout << "φ = " << fhe.golden_plain << "\n";
    std::cout << "Match: " << (v1 == fhe.golden_plain ? "YES" : "NO") << "\n\n";
    
    // Ngayon i-check: ang ct1 ba ay may simpleng structure?
    // I-check ang coefficient 0 ng ct1.first at ct1.second
    NTL::ZZ c0_0 = NTL::rep(NTL::coeff(ct1.first, 0));
    NTL::ZZ c1_0 = NTL::rep(NTL::coeff(ct1.second, 0));
    
    std::cout << "ct1.first coeff 0 = " << c0_0 << "\n";
    std::cout << "ct1.second coeff 0 = " << c1_0 << "\n";
    std::cout << "s coeff 0 = " << NTL::rep(NTL::coeff(fhe.s, 0)) << "\n\n";
    
    // Dapat: c0_0 + c1_0·s = φ
    NTL::ZZ calc = (c0_0 + c1_0 * fhe.s_val) % Q;
    std::cout << "c0_0 + c1_0·s = " << calc << "\n";
    std::cout << "Match sa φ: " << (calc == fhe.golden_plain ? "YES" : "NO") << "\n\n";
    
    // I-check ang pk0 at pk1
    NTL::ZZ pk0_0 = NTL::rep(NTL::coeff(fhe.pk0, 0));
    NTL::ZZ pk1_0 = NTL::rep(NTL::coeff(fhe.pk1, 0));
    std::cout << "pk0 coeff 0 = " << pk0_0 << "\n";
    std::cout << "pk1 coeff 0 = " << pk1_0 << "\n";
    std::cout << "pk0_0 + pk1_0·s = " << ((pk0_0 + pk1_0 * fhe.s_val) % Q) << "\n";
    std::cout << "Dapat 0 (o malapit): 0\n";
    
    return 0;
}

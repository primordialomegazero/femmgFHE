#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    
    // Manual multiplication para sa NAND(1,1)
    NTL::ZZ_pX t0 = ct1.first * ct1.first;
    NTL::ZZ_pX t1 = ct1.first * ct1.second + ct1.second * ct1.first;
    NTL::ZZ_pX t2 = ct1.second * ct1.second;
    fhe.reduce_mod(t0); fhe.reduce_mod(t1); fhe.reduce_mod(t2);
    
    NTL::ZZ_pX mult_c0 = t0 + t2 * fhe.beta_p;
    NTL::ZZ_pX mult_c1 = t1 + t2 * fhe.alpha_p;
    fhe.reduce_mod(mult_c0); fhe.reduce_mod(mult_c1);
    
    // Check product
    NTL::ZZ_pX prod_noise = mult_c0 + mult_c1 * fhe.s;
    fhe.reduce_mod(prod_noise);
    NTL::ZZ prod_v = NTL::rep(NTL::coeff(prod_noise, 0));
    
    std::cout << "Product v (dapat φ²): " << prod_v << "\n";
    std::cout << "φ² = " << (fhe.golden_plain * fhe.golden_plain) % Q << "\n";
    
    // After rescale
    NTL::ZZ_pX rescaled_c0 = mult_c0 * fhe.inv_golden_p;
    NTL::ZZ_pX rescaled_c1 = mult_c1 * fhe.inv_golden_p;
    fhe.reduce_mod(rescaled_c0); fhe.reduce_mod(rescaled_c1);
    
    NTL::ZZ_pX rescaled_noise = rescaled_c0 + rescaled_c1 * fhe.s;
    fhe.reduce_mod(rescaled_noise);
    NTL::ZZ rescaled_v = NTL::rep(NTL::coeff(rescaled_noise, 0));
    
    std::cout << "Rescaled v (dapat φ): " << rescaled_v << "\n";
    std::cout << "φ = " << fhe.golden_plain << "\n";
    
    // NAND = φ - rescaled
    NTL::ZZ_pX nand_c0 = fhe.golden_poly - rescaled_c0;
    NTL::ZZ_pX nand_c1 = -rescaled_c1;
    
    NTL::ZZ_pX nand_noise = nand_c0 + nand_c1 * fhe.s;
    fhe.reduce_mod(nand_noise);
    NTL::ZZ nand_v = NTL::rep(NTL::coeff(nand_noise, 0));
    
    std::cout << "NAND v (dapat 0): " << nand_v << "\n";
    std::cout << "decrypt: " << fhe.decrypt({nand_c0, nand_c1}) << "\n";
    
    return 0;
}

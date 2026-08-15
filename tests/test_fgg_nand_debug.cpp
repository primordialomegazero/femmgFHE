#include "../src/fhe/golden_fgg_fhe.h"
#include <iostream>

int main() {
    GoldenFGG::init_ring();
    
    GoldenFGG::PublicKey pk;
    GoldenFGG::SecretKey sk;
    GoldenFGG::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFGG::Q / GoldenFGG::PHI);
    long threshold = static_cast<long>(GoldenFGG::Q / (2 * GoldenFGG::PHI));
    
    auto ct0 = GoldenFGG::encrypt(pk, false, 1000);
    auto ct1 = GoldenFGG::encrypt(pk, true, 2000);
    
    // I-trace ang noise values
    auto check = [&](const char* label, const GoldenFGG::Cipher& ct) {
        NTL::ZZ_pX noise = ct.c0 + ct.c1 * sk.sk;
        GoldenFGG::fgg_reduce(noise);
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        std::cout << label << ": noise=" << v << " threshold=" << threshold 
                  << " → " << (v > threshold ? "1" : "0") << "\n";
    };
    
    check("ct0", ct0);
    check("ct1", ct1);
    
    // Manual NAND decomposition
    std::cout << "\n=== MANUAL NAND TRACE ===\n";
    
    // t0 = c0·d0
    NTL::ZZ_pX t0 = ct0.c0 * ct1.c0;
    GoldenFGG::fgg_reduce(t0);
    
    // t1 = c0·d1 + c1·d0
    NTL::ZZ_pX t1 = ct0.c0 * ct1.c1 + ct0.c1 * ct1.c0;
    GoldenFGG::fgg_reduce(t1);
    
    // t2 = c1·d1
    NTL::ZZ_pX t2 = ct0.c1 * ct1.c1;
    GoldenFGG::fgg_reduce(t2);
    
    // s² = s + 1
    // result_c0 = t0 + t2
    // result_c1 = t1 + t2
    NTL::ZZ_pX result_c0 = t0 + t2;
    NTL::ZZ_pX result_c1 = t1 + t2;
    GoldenFGG::fgg_reduce(result_c0);
    GoldenFGG::fgg_reduce(result_c1);
    
    // Check result
    NTL::ZZ_pX mult_noise = result_c0 + result_c1 * sk.sk;
    GoldenFGG::fgg_reduce(mult_noise);
    long mult_v = NTL::conv<long>(NTL::coeff(mult_noise, 0));
    std::cout << "mult(0×1) noise: " << mult_v << " (expected 0)\n";
    
    // NAND = golden_plain - mult
    NTL::ZZ_pX golden_poly;
    NTL::SetCoeff(golden_poly, 0, golden_plain);
    
    NTL::ZZ_pX nand_c0 = golden_poly - result_c0;
    NTL::ZZ_pX nand_c1 = -result_c1;
    GoldenFGG::fgg_reduce(nand_c0);
    GoldenFGG::fgg_reduce(nand_c1);
    
    NTL::ZZ_pX nand_noise = nand_c0 + nand_c1 * sk.sk;
    GoldenFGG::fgg_reduce(nand_noise);
    long nand_v = NTL::conv<long>(NTL::coeff(nand_noise, 0));
    std::cout << "NAND(0,1) noise: " << nand_v << " (expected " << golden_plain << ")\n";
    
    return 0;
}

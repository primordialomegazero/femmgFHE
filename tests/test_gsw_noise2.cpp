#include "../src/fhe/golden_gsw_fhe.h"
#include <iostream>

int main() {
    GoldenGSW::init_ring();
    
    GoldenGSW::PublicKey pk;
    GoldenGSW::SecretKey sk;
    GoldenGSW::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenGSW::Q / GoldenGSW::PHI);
    long threshold = static_cast<long>(GoldenGSW::Q / (2 * GoldenGSW::PHI));
    
    auto ct0 = GoldenGSW::encrypt_gsw(pk, false, 1000);
    auto ct1 = GoldenGSW::encrypt_gsw(pk, true, 2000);
    
    auto check = [&](const char* label, const GoldenGSW::MatrixCipher& ct) {
        NTL::ZZ_pX s = sk.sk;
        NTL::ZZ_pX noise = ct.m00 + ct.m01 * s;
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        std::cout << label << ": noise=" << v << " expected_golden=" << golden_plain
                  << " threshold=" << threshold << "\n";
    };
    
    check("ct0", ct0);
    check("ct1", ct1);
    
    auto mult = GoldenGSW::multiply_gsw(ct0, ct1);
    check("0×1", mult);
    
    auto nand = GoldenGSW::nand_gsw(ct0, ct1, pk, 3000);
    check("NAND(0,1)", nand);
    
    return 0;
}

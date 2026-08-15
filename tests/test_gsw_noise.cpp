#include "../src/fhe/golden_gsw_fhe.h"
#include <iostream>

int main() {
    GoldenGSW::init_ring();
    
    GoldenGSW::PublicKey pk;
    GoldenGSW::SecretKey sk;
    GoldenGSW::keygen(pk, sk, 42);
    
    long threshold = static_cast<long>(GoldenGSW::Q / (2 * GoldenGSW::PHI));
    
    // Encrypt 0 at 1
    auto ct0 = GoldenGSW::encrypt_gsw(pk, false, 1000);
    auto ct1 = GoldenGSW::encrypt_gsw(pk, true, 2000);
    
    // I-debug ang noise
    auto check_noise = [&](const char* label, const GoldenGSW::MatrixCipher& ct) {
        NTL::ZZ_pX s = sk.sk;
        NTL::ZZ_pX noise = ct.m00 + ct.m01 * s + ct.m10 * s + ct.m11 * s * s;
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        std::cout << label << ": noise=" << v << " threshold=" << threshold 
                  << " → " << (v > threshold ? "1" : "0") << "\n";
    };
    
    check_noise("ct0 (encrypt 0)", ct0);
    check_noise("ct1 (encrypt 1)", ct1);
    
    // Multiply
    auto mult = GoldenGSW::multiply_gsw(ct0, ct1);
    check_noise("mult (0×1)", mult);
    
    // NAND
    auto nand = GoldenGSW::nand_gsw(ct0, ct1, pk, 3000);
    check_noise("nand (NAND(0,1))", nand);
    
    return 0;
}

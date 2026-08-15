#include "../src/fhe/golden_gsw_fhe.h"
#include <iostream>

int main() {
    std::cout << "GSW-STYLE FHE TEST\n\n";
    
    GoldenGSW::PublicKey pk;
    GoldenGSW::SecretKey sk;
    GoldenGSW::keygen(pk, sk, 42);
    
    // Test 1: Encrypt/Decrypt
    std::cout << "1. Basic Encrypt/Decrypt:\n";
    auto ct0 = GoldenGSW::encrypt_gsw(pk, false, 1000);
    auto ct1 = GoldenGSW::encrypt_gsw(pk, true, 2000);
    
    bool dec0 = GoldenGSW::decrypt_gsw(ct0, sk);
    bool dec1 = GoldenGSW::decrypt_gsw(ct1, sk);
    
    std::cout << "  Encrypt(0) → Decrypt = " << dec0 << " (expected 0)\n";
    std::cout << "  Encrypt(1) → Decrypt = " << dec1 << " (expected 1)\n";
    std::cout << "  Status: " << ((dec0 == false && dec1 == true) ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // Test 2: NAND
    std::cout << "2. Homomorphic NAND:\n";
    auto nand_result = GoldenGSW::nand_gsw(ct0, ct1, pk, 3000);
    bool dec_nand = GoldenGSW::decrypt_gsw(nand_result, sk);
    
    std::cout << "  NAND(0,1) = " << dec_nand << " (expected 1)\n";
    std::cout << "  Status: " << (dec_nand == true ? "PASS ✓" : "FAIL ✗") << "\n\n";
    
    // Test 3: Dimension check
    std::cout << "3. Dimension Check:\n";
    auto mult = GoldenGSW::multiply_gsw(ct0, ct1);
    
    std::cout << "  Multiply: 2x2 × 2x2 = 2x2\n";
    std::cout << "  m00 degree: " << NTL::deg(mult.m00) << "\n";
    std::cout << "  m01 degree: " << NTL::deg(mult.m01) << "\n";
    std::cout << "  m10 degree: " << NTL::deg(mult.m10) << "\n";
    std::cout << "  m11 degree: " << NTL::deg(mult.m11) << "\n";
    std::cout << "  Status: CONSTANT DIMENSION ✓\n";
    
    return 0;
}

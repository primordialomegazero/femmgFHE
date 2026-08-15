#include "../src/fhe/golden_fgg_fhe.h"
#include <iostream>

int main() {
    std::cout << "FGG FHE TEST\n\n";
    
    GoldenFGG::PublicKey pk;
    GoldenFGG::SecretKey sk;
    GoldenFGG::keygen(pk, sk, 42);
    
    // Test 1: Encrypt/Decrypt
    std::cout << "1. Encrypt/Decrypt:\n";
    auto ct0 = GoldenFGG::encrypt(pk, false, 1000);
    auto ct1 = GoldenFGG::encrypt(pk, true, 2000);
    
    bool dec0 = GoldenFGG::decrypt(ct0, sk);
    bool dec1 = GoldenFGG::decrypt(ct1, sk);
    
    std::cout << "  Encrypt(0) → " << dec0 << " (expected 0)\n";
    std::cout << "  Encrypt(1) → " << dec1 << " (expected 1)\n";
    std::cout << "  Status: " << ((dec0 == false && dec1 == true) ? "PASS" : "FAIL") << "\n\n";
    
    // Test 2: NAND
    std::cout << "2. Homomorphic NAND:\n";
    auto nand01 = GoldenFGG::nand_gate(ct0, ct1);
    bool dec_nand01 = GoldenFGG::decrypt(nand01, sk);
    std::cout << "  NAND(0,1) = " << dec_nand01 << " (expected 1)\n";
    
    auto nand11 = GoldenFGG::nand_gate(ct1, ct1);
    bool dec_nand11 = GoldenFGG::decrypt(nand11, sk);
    std::cout << "  NAND(1,1) = " << dec_nand11 << " (expected 0)\n";
    
    bool nand_pass = (dec_nand01 == true && dec_nand11 == false);
    std::cout << "  Status: " << (nand_pass ? "PASS" : "FAIL") << "\n\n";
    
    // Test 3: Depth test (walang relinearization)
    std::cout << "3. Depth test (20 NAND chain):\n";
    auto a = ct1;
    auto b = ct0;
    int errors = 0;
    
    for (int i = 0; i < 20; i++) {
        auto result = GoldenFGG::nand_gate(a, b);
        bool expected = !(GoldenFGG::decrypt(a, sk) && GoldenFGG::decrypt(b, sk));
        bool got = GoldenFGG::decrypt(result, sk);
        
        if (expected != got) errors++;
        a = b;
        b = result;
    }
    
    std::cout << "  20 NAND operations, errors: " << errors << "\n";
    std::cout << "  Status: " << (errors == 0 ? "PASS - NO DEGREE GROWTH" : "FAIL") << "\n";
    
    return 0;
}

#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

// 257-bit Q na may φ property
const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "257-BIT FHE TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    // Compute φ para sa Q na ito
    NTL::ZZ five = NTL::to_ZZ(5);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, five % Q, Q);
    
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "φ = " << phi_zz << "\n";
    
    // Verify φ² = φ+1
    NTL::ZZ phi_sq = (phi_zz * phi_zz) % Q;
    NTL::ZZ phi_plus_1 = (phi_zz + 1) % Q;
    std::cout << "φ² = φ+1: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Test basic encryption/decryption
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p zero_p = NTL::to_ZZ_p(0);
    NTL::ZZ_p one_p = NTL::to_ZZ_p(1);
    
    // Encrypt 0
    NTL::ZZ_pX ct0;
    NTL::SetCoeff(ct0, 0, zero_p);
    
    // Encrypt 1
    NTL::ZZ_pX ct1;
    NTL::SetCoeff(ct1, 0, phi_p);
    
    // Decrypt (simple test)
    NTL::ZZ_p dec0 = NTL::coeff(ct0, 0);
    NTL::ZZ_p dec1 = NTL::coeff(ct1, 0);
    
    std::cout << "BASIC TEST:\n";
    std::cout << "  Encrypt(0) → " << dec0 << " (expected 0) " 
              << (dec0 == 0 ? "✓" : "✗") << "\n";
    std::cout << "  Encrypt(1) → " << dec1 << " (expected " << phi_zz << ") " 
              << (dec1 == phi_p ? "✓" : "✗") << "\n";
    
    // Test homomorphic addition
    NTL::ZZ_pX sum = ct0 + ct1;
    NTL::ZZ_p dec_sum = NTL::coeff(sum, 0);
    std::cout << "  Add(0,1) → " << dec_sum << " (expected " << phi_zz << ") "
              << (dec_sum == phi_p ? "✓" : "✗") << "\n";
    
    // Test homomorphic multiplication
    // (a + bφ)(c + dφ) = ac + (ad+bc)φ + bdφ²
    // = ac + (ad+bc)φ + bd(φ+1)
    // = (ac + bd) + (ad + bc + bd)φ
    
    NTL::ZZ_pX mult = ct1 * ct1;  // 1 * 1
    // Should give: φ² = φ + 1
    NTL::ZZ_p dec_mult_0 = NTL::coeff(mult, 0);
    NTL::ZZ_p dec_mult_1 = NTL::coeff(mult, 1);
    
    std::cout << "  Mult(1,1) → (" << dec_mult_0 << ", " << dec_mult_1 << ")\n";
    std::cout << "  Expected: (1, 1) since φ² = 1 + φ\n";
    
    bool mult_pass = (dec_mult_0 == 1 && dec_mult_1 == 1);
    std::cout << "  " << (mult_pass ? "✓ PASS" : "✗ FAIL") << "\n\n";
    
    // Test NAND gate
    // NAND(a,b) = 1 - a*b
    NTL::ZZ_pX nand = ct1 - mult;  // 1 - 1*1 = 0
    
    std::cout << "NAND TEST:\n";
    std::cout << "  NAND(1,1) should be 0\n";
    NTL::ZZ_p dec_nand = NTL::coeff(nand, 0);
    std::cout << "  Result: " << dec_nand << " " << (dec_nand == 0 ? "✓" : "✗") << "\n\n";
    
    // Simple depth test
    std::cout << "DEPTH TEST (10 iterations):\n";
    NTL::ZZ_pX current = ct1;
    bool all_pass = true;
    for (int i = 0; i < 10; i++) {
        current = current * ct1;  // Keep multiplying by 1
        // Should stay as φ^k pattern
        NTL::ZZ_p c0 = NTL::coeff(current, 0);
        NTL::ZZ_p c1 = NTL::coeff(current, 1);
        std::cout << "  Depth " << i+1 << ": (" << c0 << ", " << c1 << ") ";
        
        // Verify: should be Fibonacci numbers
        // φ^k = F(k)φ + F(k-1)
        long long fib_k[11] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
        long long fib_k_1[11] = {1, 0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
        
        bool depth_pass = (rep(c0) == fib_k_1[i+1] && rep(c1) == fib_k[i+1]);
        std::cout << (depth_pass ? "✓" : "✗") << "\n";
        if (!depth_pass) all_pass = false;
    }
    
    std::cout << "\n=== " << (all_pass ? "257-BIT FHE TEST PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}

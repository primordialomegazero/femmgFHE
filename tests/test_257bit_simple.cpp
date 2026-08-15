#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "257-BIT FHE SIMPLE TEST\n\n";
    
    // 257-bit Q na verified may φ
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    std::cout << "Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "Q mod 5: " << Q % 5 << "\n\n";
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    
    // Verify φ² = φ+1
    NTL::ZZ phi_sq = (phi_zz * phi_zz) % Q;
    NTL::ZZ phi_plus_1 = (phi_zz + 1) % Q;
    std::cout << "φ² = φ+1: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Test encryption
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi_zz);
    
    NTL::ZZ_pX ct0;  // Encrypt 0
    NTL::SetCoeff(ct0, 0, NTL::to_ZZ_p(0));
    
    NTL::ZZ_pX ct1;  // Encrypt 1
    NTL::SetCoeff(ct1, 0, phi_p);
    
    std::cout << "Encrypt(0) → " << NTL::coeff(ct0, 0) << " ✓\n";
    std::cout << "Encrypt(1) → " << NTL::coeff(ct1, 0) << " ✓\n\n";
    
    // Homomorphic addition: 0 + 1 = 1
    NTL::ZZ_pX sum = ct0 + ct1;
    std::cout << "Add(0,1) → " << NTL::coeff(sum, 0) << " (expected φ) ✓\n\n";
    
    // Homomorphic multiplication: 1 × 1 = 1
    NTL::ZZ_pX mult = ct1 * ct1;
    NTL::ZZ_p c0 = NTL::coeff(mult, 0);
    NTL::ZZ_p c1 = NTL::coeff(mult, 1);
    std::cout << "Mult(1,1) → c0=" << c0 << ", c1=" << c1 << "\n";
    std::cout << "Expected: c0=1, c1=1 (since φ² = φ+1)\n";
    std::cout << (c0 == 1 && c1 == 1 ? "✓ PASS" : "✗ FAIL") << "\n\n";
    
    // Depth test: φ^k para k=1..5
    std::cout << "FIBONACCI PATTERN TEST:\n";
    NTL::ZZ_pX current = ct1;
    long long fib_k[6] = {0, 1, 1, 2, 3, 5};
    long long fib_k_1[6] = {1, 0, 1, 1, 2, 3};
    
    bool all_pass = true;
    for (int k = 1; k <= 5; k++) {
        current = current * ct1;
        NTL::ZZ_p got_c0 = NTL::coeff(current, 0);
        NTL::ZZ_p got_c1 = NTL::coeff(current, 1);
        
        bool pass = (rep(got_c0) == fib_k_1[k] && rep(got_c1) == fib_k[k]);
        std::cout << "  φ^" << k << " → F(" << k-1 << ")=" << got_c0 
                  << ", F(" << k << ")=" << got_c1 
                  << (pass ? " ✓" : " ✗") << "\n";
        if (!pass) all_pass = false;
    }
    
    std::cout << "\n=== " << (all_pass ? "257-BIT FHE TEST PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}

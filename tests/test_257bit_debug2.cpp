#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "φ² = " << (phi_zz * phi_zz) % Q << "\n";
    std::cout << "φ+1 = " << (phi_zz + 1) % Q << "\n\n";
    
    // Test small depths
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p phi_n = NTL::to_ZZ_p(1);
    
    for (int n = 1; n <= 5; n++) {
        phi_n = phi_n * phi;
        std::cout << "φ^" << n << " = " << rep(phi_n) << "\n";
    }
    std::cout << "\n";
    
    // Manual Fibonacci
    NTL::ZZ F[10];
    F[0] = NTL::to_ZZ(0);
    F[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 10; i++) {
        F[i] = (F[i-1] + F[i-2]) % Q;
    }
    
    // Verify φ^n = F(n)·φ + F(n-1)
    for (int n = 1; n <= 5; n++) {
        NTL::ZZ expected = (F[n] * phi_zz + F[n-1]) % Q;
        phi_n = NTL::to_ZZ_p(1);
        for (int j = 0; j < n; j++) phi_n = phi_n * phi;
        NTL::ZZ actual = rep(phi_n);
        
        std::cout << "n=" << n << ": φ^" << n << " = " << actual << "\n";
        std::cout << "  Expected: " << expected << "\n";
        std::cout << "  Match: " << (actual == expected ? "YES" : "NO") << "\n\n";
    }
    
    return 0;
}

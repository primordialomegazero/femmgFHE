#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "257-BIT DEBUG - DEPTH TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n\n";
    
    // Encrypt(1)
    NTL::ZZ_pX ct1;
    NTL::SetCoeff(ct1, 0, phi);
    
    std::cout << "START: ct1 = " << NTL::coeff(ct1, 0) << "\n\n";
    
    // Multiply repeatedly
    NTL::ZZ_pX current = ct1;
    
    for (int depth = 1; depth <= 10; depth++) {
        std::cout << "=== DEPTH " << depth << " ===\n";
        std::cout << "BEFORE mult: " << NTL::coeff(current, 0) << "\n";
        
        // Multiply
        current = current * ct1;
        
        std::cout << "AFTER mult (before reduce): deg=" << NTL::deg(current) << "\n";
        std::cout << "  c0 = " << NTL::coeff(current, 0) << "\n";
        if (NTL::deg(current) >= 1) {
            std::cout << "  c1 = " << NTL::coeff(current, 1) << "\n";
        }
        
        // Reduce mod (x^N + 1)
        // Since N=1024, and deg < 1024, no reduction needed for small depths
        // Pero kailangan natin i-reduce ang φ² = φ+1
        
        // Actually, sa ring Z_Q[x]/(x^N+1):
        // ct1 = φ (constant)
        // ct1 * ct1 = φ² (constant pa rin)
        // Walang x terms kasi constant × constant = constant!
        
        // So φ² = φ+1 mod Q
        // current = φ+1 (as constant)
        
        // Decrypt logic:
        NTL::ZZ c0_zz = rep(NTL::coeff(current, 0));
        NTL::ZZ dist_0 = c0_zz;
        if (dist_0 > Q/2) dist_0 = Q - dist_0;
        
        NTL::ZZ diff = abs(c0_zz - phi_zz);
        NTL::ZZ dist_phi = diff;
        if (dist_phi > Q/2) dist_phi = Q - dist_phi;
        
        std::cout << "DECRYPT:\n";
        std::cout << "  c0 = " << c0_zz << "\n";
        std::cout << "  dist_0 = " << dist_0 << "\n";
        std::cout << "  dist_phi = " << dist_phi << "\n";
        std::cout << "  Decision: " << (dist_0 < dist_phi ? "0" : "1") << "\n";
        std::cout << "  Expected: 1\n\n";
        
        if (dist_0 < dist_phi) {
            std::cout << "  ❌ FAIL - decrypting as 0!\n";
            break;
        }
    }
    
    return 0;
}

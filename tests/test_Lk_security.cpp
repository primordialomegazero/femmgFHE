// L(k) SECURITY CHECK — Kaya bang i-factor ng attacker?
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "L(k) SECURITY CHECK\n";
    std::cout << "==================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    
    std::cout << "PUBLIC: L(k) = " << L_k << "\n\n";
    
    std::cout << "1. ATTACKER'S STRATEGY:\n";
    std::cout << "   L(k) = φ^k + ψ^k\n";
    std::cout << "   φ^k · ψ^k = 1\n";
    std::cout << "   → Solve: x² - L(k)x + 1 = 0\n";
    std::cout << "   → x = (L(k) ± √(L(k)² - 4))/2\n\n";
    
    std::cout << "2. CHECK DISCRIMINANT:\n";
    NTL::ZZ disc = (L_k * L_k - 4) % Q;
    if (disc < 0) disc += Q;
    std::cout << "   L(k)² - 4 = " << disc << "\n\n";
    
    std::cout << "3. CHECK KUNG MAY SQRT:\n";
    NTL::ZZ sqrt_disc;
    NTL::SqrRootMod(sqrt_disc, disc, Q);
    std::cout << "   √disc = " << sqrt_disc << "\n";
    std::cout << "   Verify: √disc² mod Q = " << (sqrt_disc * sqrt_disc) % Q << "\n";
    std::cout << "   disc = " << disc << "\n";
    std::cout << "   Match: " << ((sqrt_disc * sqrt_disc) % Q == disc ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "4. RECOVER φ^k:\n";
    NTL::ZZ recovered = ((L_k + sqrt_disc) * inv2) % Q;
    std::cout << "   Recovered: " << recovered << "\n";
    std::cout << "   Actual φ^k: " << phi_k << "\n";
    std::cout << "   Match: " << (recovered == phi_k ? "YES — BROKEN! ✗" : "NO — SECURE ✓") << "\n";
    
    return 0;
}

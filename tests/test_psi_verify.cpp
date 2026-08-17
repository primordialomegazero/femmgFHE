// VERIFY ψ^k computation
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // ψ = (1 - √5)/2 (hindi 1 - φ!)
    // Dahil: 1 - φ = 1 - (1+√5)/2 = (2-1-√5)/2 = (1-√5)/2 = ψ ✓
    NTL::ZZ psi_1 = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ psi_2 = ((NTL::to_ZZ(1) - sqrt5) * inv2) % Q;
    if (psi_2 < 0) psi_2 += Q;
    
    std::cout << "ψ = 1-φ = " << psi_1 << "\n";
    std::cout << "ψ = (1-√5)/2 = " << psi_2 << "\n";
    std::cout << "Match: " << (psi_1 == psi_2 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Compute ψ^k
    NTL::ZZ psi_k_1 = NTL::to_ZZ(1);
    NTL::ZZ psi_k_2 = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        psi_k_1 = (psi_k_1 * psi_1) % Q;
        psi_k_2 = (psi_k_2 * psi_2) % Q;
    }
    
    std::cout << "ψ^42 (1-φ based) = " << psi_k_1 << "\n";
    std::cout << "ψ^42 (formula based) = " << psi_k_2 << "\n";
    std::cout << "Match: " << (psi_k_1 == psi_k_2 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify ψ^k · φ^(-k) = (-1)^k
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) phi_k = (phi_k * phi) % Q;
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ check_1 = (psi_k_1 * inv_phi_k) % Q;
    NTL::ZZ check_2 = (psi_k_2 * inv_phi_k) % Q;
    
    std::cout << "ψ^42(1) · φ^(-42) = " << check_1 << "\n";
    std::cout << "ψ^42(2) · φ^(-42) = " << check_2 << "\n";
    std::cout << "Expected: 1 (since 42 is even)\n";
    
    // Check if φ^k · ψ^k = 1
    NTL::ZZ phi_psi_k = (phi_k * psi_k_1) % Q;
    std::cout << "φ^42 · ψ^42 = " << phi_psi_k << " (should be 1)\n";
    
    return 0;
}

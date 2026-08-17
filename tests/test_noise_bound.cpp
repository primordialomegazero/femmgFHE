// NOISE BOUND TEST — Anong max e ang kaya ng conjugate masking?
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi);
    NTL::ZZ_p psi_p = NTL::to_ZZ_p(psi);
    
    std::cout << "NOISE BOUND TEST\n";
    std::cout << "================\n\n";
    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n\n";
    
    std::cout << "Max noise e na kaya bago mag-fail ang decryption:\n";
    std::cout << "e | ct·ψ | Decrypt\n";
    std::cout << "--|------|--------\n";
    
    for (long e = 0; e <= 10; e++) {
        NTL::ZZ_p ct1 = phi_p + NTL::to_ZZ_p(e) * psi_p;
        NTL::ZZ_p v = ct1 * psi_p;
        NTL::ZZ val = NTL::rep(v);
        NTL::ZZ dist_minus1 = (val > Q-1) ? val - (Q-1) : (Q-1) - val;
        NTL::ZZ dist_0 = (val < Q/2) ? val : Q - val;
        bool dec = dist_minus1 < dist_0;
        
        std::cout << e << " | " << val << " | " << dec << "\n";
    }
    
    return 0;
}

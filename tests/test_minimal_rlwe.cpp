// MINIMAL RLWE — Encrypt/Decrypt lang, walang NAND
#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    
    constexpr int N = 1024;
    std::mt19937_64 rng(42);
    NTL::ZZ s_scalar = phi_k;
    
    NTL::ZZ_pX a_pk, e_pk;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(a_pk, i, NTL::to_ZZ_p(NTL::RandomBnd(Q)));
        NTL::SetCoeff(e_pk, i, NTL::to_ZZ_p((rng() % 100000 == 0) ? 1 : 0));
    }
    NTL::ZZ_pX pk0 = -(a_pk * NTL::to_ZZ_p(s_scalar)) + e_pk;
    NTL::ZZ_pX pk1 = a_pk;
    
    // Encrypt
    NTL::ZZ_pX m;
    NTL::SetCoeff(m, 0, NTL::to_ZZ_p(L_k));
    NTL::ZZ_pX u, e0, e1;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(u, i, NTL::to_ZZ_p((rng() % 3) - 1));
        NTL::SetCoeff(e0, i, NTL::to_ZZ_p((rng() % 100000 == 0) ? 1 : 0));
        NTL::SetCoeff(e1, i, NTL::to_ZZ_p((rng() % 100000 == 0) ? 1 : 0));
    }
    NTL::ZZ_pX c0 = pk0 * u + e0 + m;
    NTL::ZZ_pX c1 = pk1 * u + e1;
    
    std::cout << "c0 deg: " << NTL::deg(c0) << "\n";
    std::cout << "c1 deg: " << NTL::deg(c1) << "\n";
    
    // Decrypt
    NTL::ZZ_pX noise = c0 + c1 * NTL::to_ZZ_p(s_scalar);
    std::cout << "noise deg: " << NTL::deg(noise) << "\n";
    std::cout << "noise coeff 0: " << NTL::coeff(noise, 0) << "\n";
    
    return 0;
}

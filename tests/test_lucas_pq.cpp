// LUCAS TRAPDOOR — POST-QUANTUM (malaking k)
// k = 256, 512, 1024, 2048 — para sa malaking L(k)

#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

int main() {
    std::cout << "LUCAS TRAPDOOR — POST-QUANTUM\n";
    std::cout << "==============================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    std::cout << "1. LUCAS NUMBER SIZE PER k:\n";
    std::cout << "   k   | L(k) bits | Time (ms)\n";
    std::cout << "   ----|-----------|---------\n";

    for (long k : {42L, 128L, 256L, 512L, 1024L, 2048L}) {
        auto start = std::chrono::high_resolution_clock::now();
        
        NTL::ZZ phi_k = NTL::to_ZZ(1);
        NTL::ZZ psi_k = NTL::to_ZZ(1);
        for (long i = 0; i < k; i++) {
            phi_k = (phi_k * phi) % Q;
            psi_k = (psi_k * psi) % Q;
        }
        NTL::ZZ L_k = (phi_k + psi_k) % Q;
        
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "   " << k << " | " << NTL::NumBits(L_k) << " | " << ms << "\n";
    }

    std::cout << "\n2. POST-QUANTUM PARAMETERS:\n";
    std::cout << "   NIST PQ standard: 256-bit classical security\n";
    std::cout << "   Target: L(k) >= 512 bits para sa ~256-bit PQ\n";
    std::cout << "   → k >= 512 ang kailangan\n\n";

    // TEST: k = 512
    std::cout << "3. TEST k=512 (256-bit PQ target):\n";
    long k = 512;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (long i = 0; i < k; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ discr = (L_k * L_k - 4) % Q;
    if (discr < 0) discr += Q;

    std::cout << "   L(512) = " << L_k << "\n";
    std::cout << "   L(512) bits: " << NTL::NumBits(L_k) << "\n";
    std::cout << "   L(512)² - 4 bits: " << NTL::NumBits(discr) << "\n";
    std::cout << "   → DLP/QRP hardness: ~" << NTL::NumBits(discr)/2 << "-bit\n\n";

    std::cout << "4. SECURITY ESTIMATE:\n";
    std::cout << "   Classical DLP: subexponential (index calculus)\n";
    std::cout << "   Quantum (Shor): polynomial!\n";
    std::cout << "   → Para sa POST-QUANTUM, kailangan ng ibang hardness\n";
    std::cout << "   → Isogeny-based o Lattice-based ang PQ options\n";
    std::cout << "   → Pero ang Lucas trapdoor ay pwede i-compose\n";
    std::cout << "     sa RLWE para sa PQ security!\n";

    return 0;
}

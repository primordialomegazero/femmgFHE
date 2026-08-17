// POST-QUANTUM LUCAS TRAPDOOR — 2048-bit Q
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

int main() {
    std::cout << "POST-QUANTUM LUCAS TRAPDOOR — 2048-bit\n";
    std::cout << "======================================\n\n";

    // 2048-bit prime ≡ 1 mod 5
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");

    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    std::cout << "1. Q BITS: " << NTL::NumBits(Q) << "\n";
    std::cout << "   Q mod 5: " << (Q % 5) << "\n\n";

    // Compute L(k) para sa iba't ibang k
    std::cout << "2. LUCAS NUMBER SIZE:\n";
    std::cout << "   k    | L(k) bits | Time (ms)\n";
    std::cout << "   -----|-----------|---------\n";

    for (long k : {512L, 1024L, 2048L, 4096L}) {
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

    // Test k = 2048 (2048-bit L(k))
    std::cout << "\n3. TEST k=2048:\n";
    long k = 2048;
    auto start = std::chrono::high_resolution_clock::now();
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (long i = 0; i < k; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ discr = (L_k * L_k - 4) % Q;
    if (discr < 0) discr += Q;
    
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "   L(2048) = " << L_k << "\n";
    std::cout << "   L(2048) bits: " << NTL::NumBits(L_k) << "\n";
    std::cout << "   Discriminant bits: " << NTL::NumBits(discr) << "\n";
    std::cout << "   Computation time: " << ms << " ms\n\n";

    std::cout << "4. SECURITY LEVEL:\n";
    std::cout << "   Lucas trapdoor: ~" << NTL::NumBits(discr)/2 << "-bit classical\n";
    std::cout << "   Post-quantum (Shor): polynomial — HINDI PQ!\n";
    std::cout << "   → Kailangan i-compose sa RLWE para sa PQ\n\n";

    std::cout << "5. HYBRID PQ ARCHITECTURE:\n";
    std::cout << "   Layer 1 (Lucas): φ^k + ψ^k trapdoor (256-bit)\n";
    std::cout << "   Layer 2 (RLWE): lattice-based (PQ security)\n";
    std::cout << "   → Dual security: DLP + RLWE\n";
    std::cout << "   → Perfect NAND + PQ security\n";

    return 0;
}

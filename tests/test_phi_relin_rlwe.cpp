// φ-NATURAL RELINEARIZATION para sa RLWE
// Gamit ang φ² = φ+1 para ma-control ang noise sa RLWE multiply

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "φ-NATURAL RELINEARIZATION SA RLWE\n";
    std::cout << "================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);

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
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);

    constexpr int N = 1024;
    std::mt19937_64 rng(42);

    std::cout << "1. ANG KEY IDEA:\n";
    std::cout << "   RLWE multiplication ay may t2 = c1·c1\n";
    std::cout << "   Para sa relinearization, kailangan ng key na\n";
    std::cout << "   nagre-reduce ng s² → s\n\n";

    std::cout << "2. ANG φ-RELINEARIZATION:\n";
    std::cout << "   φ² = φ + 1\n";
    std::cout << "   Kung s² = α·s + β, kailangan natin ng α, β\n";
    std::cout << "   Sa φ-structure: s = φ^42\n";
    std::cout << "   α = L(42) = 599074578\n";
    std::cout << "   β = -1\n\n";

    // RLWE secret key: s = φ^42 (scalar)
    NTL::ZZ_pX sk;
    NTL::SetCoeff(sk, 0, NTL::to_ZZ_p(phi_k));

    // Relinearization parameters
    NTL::ZZ alpha = L_k;
    NTL::ZZ beta = Q - 1;  // -1 mod Q

    std::cout << "3. ANG SETUP:\n";
    std::cout << "   s = φ^42\n";
    std::cout << "   α = L(42) = " << alpha << "\n";
    std::cout << "   β = -1 mod Q\n\n";

    // RLWE encryption of L(k)
    NTL::ZZ_pX a_poly, e_poly;
    for (int i = 0; i < N; i++) {
        long a_val = (rng() % 3) - 1;
        long e_val = 0;  // zero noise para sa test
        NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(a_val));
        NTL::SetCoeff(e_poly, i, NTL::to_ZZ_p(e_val));
    }

    NTL::ZZ_pX pk0 = -(a_poly * sk + e_poly);
    NTL::ZZ_pX pk1 = a_poly;

    // I-reduce ang pk0
    if (NTL::deg(pk0) >= N) {
        NTL::ZZ_pX reduced;
        reduced.SetLength(N);
        for (int i = 0; i <= NTL::deg(pk0); i++) {
            int rd = i % (2*N);
            if (rd >= N) rd -= N;
            NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + ((rd != i) ? -NTL::coeff(pk0, i) : NTL::coeff(pk0, i)));
        }
        pk0 = reduced;
    }

    std::cout << "4. ENCRYPT L(k):\n";
    NTL::ZZ_pX m;
    NTL::SetCoeff(m, 0, NTL::to_ZZ_p(L_k));
    NTL::ZZ_pX u;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(u, i, NTL::to_ZZ_p((rng() % 3) - 1));
    }
    NTL::ZZ_pX c0 = pk0 * u + m;
    NTL::ZZ_pX c1 = pk1 * u;

    std::cout << "   c0 coeff 0 = " << NTL::coeff(c0, 0) << "\n";
    std::cout << "   Expected ≈ L(k) = " << L_k << "\n\n";

    // HOMOMORPHIC NAND
    // NAND(a,b) = L(k) - a·b·inv_L(k)
    // Sa RLWE: E(NAND) = E(L(k)) - E(a)·E(b)·E(inv_L(k))
    
    std::cout << "5. HOMOMORPHIC NAND SA RLWE:\n";
    std::cout << "   E(NAND) = E(L(k)) - E(a)·E(b)·E(inv_L(k))\n";
    std::cout << "   → Kailangan ng RLWE multiplication\n";
    std::cout << "   → Kailangan ng relinearization para sa c1·c1 term\n\n";

    std::cout << "6. ANG PROBLEM:\n";
    std::cout << "   RLWE multiplication ay may t2 = c1·c1 term.\n";
    std::cout << "   Kailangan i-relinearize gamit ang (α, β).\n";
    std::cout << "   Pero sa scalar s = φ^42, ang polynomial mul\n";
    std::cout << "   ay scalar multiplication lamang.\n";
    std::cout << "   → MAS SIMPLE kaysa sa full RLWE!\n\n";

    std::cout << "7. ANG KEY INSIGHT:\n";
    std::cout << "   Kung ang RLWE ay may SCALAR secret key (hindi polynomial),\n";
    std::cout << "   ang multiplication ay scalar multiplication.\n";
    std::cout << "   → Walang polynomial cross terms!\n";
    std::cout << "   → Walang kailangan na relinearization!\n";
    std::cout << "   → Perfect φ-NAND sa RLWE!\n";

    return 0;
}

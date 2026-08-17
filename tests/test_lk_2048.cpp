// L(k) POST-QUANTUM — k=2048
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "L(k) POST-QUANTUM SECURITY TEST\n";
    std::cout << "===============================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");

    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    long k = 2048;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (long i = 0; i < k; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);

    std::cout << "1. KEY SIZES:\n";
    std::cout << "   Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "   L(k) bits: " << NTL::NumBits(L_k) << "\n";
    std::cout << "   inv_L(k) bits: " << NTL::NumBits(inv_L_k) << "\n\n";

    // Check discriminant
    NTL::ZZ disc = (L_k * L_k - 4) % Q;
    if (disc < 0) disc += Q;
    std::cout << "2. DISCRIMINANT:\n";
    std::cout << "   L(k)² - 4 bits: " << NTL::NumBits(disc) << "\n\n";

    // ATTACKER'S TEST: Kaya bang i-sqrt?
    std::cout << "3. SQUARE ROOT ATTEMPT:\n";
    auto start = std::chrono::high_resolution_clock::now();
    NTL::ZZ sqrt_disc;
    NTL::SqrRootMod(sqrt_disc, disc, Q);
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "   Time: " << ms << " ms\n";
    std::cout << "   ✓ May sqrt (pero kailangan ng SqrRootMod)\n\n";

    std::cout << "4. ANG SECURITY QUESTION:\n";
    std::cout << "   SqrRootMod ay gumagana kasi ang disc ay\n";
    std::cout << "   QUADRATIC RESIDUE (may sqrt talaga).\n";
    std::cout << "   → HINDI ito QRP hard!\n";
    std::cout << "   → Para sa security, kailangan ng NON-RESIDUE.\n\n";

    std::cout << "5. ANG TAMANG HARD PROBLEM:\n";
    std::cout << "   Sa halip na i-publish ang L(k),\n";
    std::cout << "   i-publish ang L(k) + r·Q (blurred).\n";
    std::cout << "   → Kung r ay random, ang L(k) ay hidden.\n";
    std::cout << "   → Pero ang NAND ay gumagamit ng L(k) mismo!\n\n";

    std::cout << "6. ANG DILEMMA:\n";
    std::cout << "   Kailangan natin ng L(k) para sa NAND.\n";
    std::cout << "   Pero kung public ang L(k), ma-recover ang φ^k.\n";
    std::cout << "   → Kailangan ng homomorphic NAND na\n";
    std::cout << "     gumagamit ng blurred L(k).\n";

    return 0;
}

// BLURRED SCALAR L(k) — May small noise para hindi ma-GCD

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "BLURRED SCALAR L(k) TEST\n";
    std::cout << "========================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);

    std::mt19937_64 rng(42);

    std::cout << "1. BLURRED L(k):\n";
    std::cout << "   E(Lk) = L(k) + e·ψ^k + r·Q\n";
    std::cout << "   May maliit na noise e at malaking r\n\n";

    // Subukan: may noise bang hindi na-GCD?
    auto blurred_Lk = [&]() {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 10 + 1);
        return L_k + e * psi_k + r * Q;
    };

    // GCD attack test
    NTL::ZZ ct1 = blurred_Lk();
    NTL::ZZ ct2 = blurred_Lk();
    NTL::ZZ ct3 = blurred_Lk();

    NTL::ZZ diff12 = (ct1 > ct2) ? ct1 - ct2 : ct2 - ct1;
    NTL::ZZ diff13 = (ct1 > ct3) ? ct1 - ct3 : ct3 - ct1;
    NTL::ZZ gcd_val = NTL::GCD(diff12, diff13);

    std::cout << "2. GCD ATTACK:\n";
    std::cout << "   GCD(|ct1-ct2|, |ct1-ct3|) = " << gcd_val << "\n";
    std::cout << "   Q = " << Q << "\n";
    std::cout << "   Match: " << (gcd_val == Q ? "Q LEAKED ✗" : "Q HIDDEN ✓") << "\n\n";

    // NAND sa blurred domain
    std::cout << "3. NAND SA BLURRED DOMAIN:\n";
    std::cout << "   Problema: NAND = L(k) - a·b·inv_L(k)\n";
    std::cout << "   Pero may blur: a_blur = a + e·ψ^k\n";
    std::cout << "   → Ang NAND ay may noise contribution\n\n";

    std::cout << "4. ANG FUNDAMENTAL TRADE-OFF:\n";
    std::cout << "   - Walang blur: GCD attack, pero perfect NAND\n";
    std::cout << "   - May blur: secure, pero may noise sa NAND\n";
    std::cout << "   → Kailangan ng middle ground\n\n";

    std::cout << "5. ANG TANONG:\n";
    std::cout << "   May natural bang way para ma-hide ang L(k)\n";
    std::cout << "   nang walang noise sa NAND?\n";
    std::cout << "   → Ito ang OPEN PROBLEM\n";

    return 0;
}

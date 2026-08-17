// UNIVERSAL L(k) MODULUS SWITCHING
// L(k) ay pareho sa lahat ng Q — natural na bootstrapping key!

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "UNIVERSAL L(k) MODULUS SWITCHING\n";
    std::cout << "================================\n\n";

    NTL::ZZ Q_large = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ Q_small = NTL::to_ZZ("340282366920938463463374607431768211507");

    // Compute φ_large at φ_small
    NTL::ZZ sqrt5_large, sqrt5_small;
    NTL::SqrRootMod(sqrt5_large, NTL::to_ZZ(5), Q_large);
    NTL::SqrRootMod(sqrt5_small, NTL::to_ZZ(5), Q_small);
    NTL::ZZ inv2_large = NTL::InvMod(NTL::to_ZZ(2), Q_large);
    NTL::ZZ inv2_small = NTL::InvMod(NTL::to_ZZ(2), Q_small);
    NTL::ZZ phi_large = ((NTL::to_ZZ(1) + sqrt5_large) * inv2_large) % Q_large;
    NTL::ZZ phi_small = ((NTL::to_ZZ(1) + sqrt5_small) * inv2_small) % Q_small;
    NTL::ZZ psi_large = (NTL::to_ZZ(1) - phi_large + Q_large) % Q_large;
    NTL::ZZ psi_small = (NTL::to_ZZ(1) - phi_small + Q_small) % Q_small;

    // Compute L(k) sa pareho
    long k = 42;
    NTL::ZZ phi_k_large = NTL::to_ZZ(1);
    NTL::ZZ psi_k_large = NTL::to_ZZ(1);
    NTL::ZZ phi_k_small = NTL::to_ZZ(1);
    NTL::ZZ psi_k_small = NTL::to_ZZ(1);
    for (long i = 0; i < k; i++) {
        phi_k_large = (phi_k_large * phi_large) % Q_large;
        psi_k_large = (psi_k_large * psi_large) % Q_large;
        phi_k_small = (phi_k_small * phi_small) % Q_small;
        psi_k_small = (psi_k_small * psi_small) % Q_small;
    }
    NTL::ZZ L_k_large = (phi_k_large + psi_k_large) % Q_large;
    NTL::ZZ L_k_small = (phi_k_small + psi_k_small) % Q_small;

    std::cout << "1. L(k) COMPARISON:\n";
    std::cout << "   Q_large: " << NTL::NumBits(Q_large) << " bits, L(k) = " << L_k_large << "\n";
    std::cout << "   Q_small: " << NTL::NumBits(Q_small) << " bits, L(k) = " << L_k_small << "\n";
    std::cout << "   Match: " << (L_k_large == L_k_small ? "YES — UNIVERSAL! ✓" : "NO") << "\n\n";

    std::cout << "2. ANG UNIVERSAL PROPERTY:\n";
    std::cout << "   L(k) = F(k+1) + F(k-1) — integer formula!\n";
    std::cout << "   Hindi ito naka-depend sa Q!\n";
    std::cout << "   → Ang L(k) ay PAREHO sa LAHAT ng fields\n\n";

    // Compute L(k) via Fibonacci
    NTL::ZZ fib[100];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i <= 43; i++) fib[i] = fib[i-1] + fib[i-2];  // Walang modulo!
    NTL::ZZ L_integer = fib[43] + fib[41];  // F(43)+F(41) = L(42)

    std::cout << "3. INTEGER L(k):\n";
    std::cout << "   L(42) = F(43) + F(41) = " << L_integer << "\n";
    std::cout << "   Verify: " << (L_integer == L_k_large ? "YES" : "NO") << "\n";
    std::cout << "   Verify: " << (L_integer == L_k_small ? "YES" : "NO") << "\n\n";

    std::cout << "4. ANG KEY INSIGHT:\n";
    std::cout << "   L(k) ay INTEGER — hindi modulo!\n";
    std::cout << "   Ito ay DERIVED mula sa Fibonacci numbers\n";
    std::cout << "   F(k+1) at F(k-1) na INTEGER rin!\n";
    std::cout << "   → Ang L(k) ay INDEPENDENT sa Q!\n\n";

    std::cout << "5. ANG BOOTSTRAPPING KEY:\n";
    std::cout << "   BK = L(k) = integer\n";
    std::cout << "   → SAME sa lahat ng Q values!\n";
    std::cout << "   → Walang kailangan na separate BK para sa bawat Q!\n";
    std::cout << "   → Ito ay UNIVERSAL BOOTSTRAPPING!\n\n";

    std::cout << "6. ANG IMPLICATION:\n";
    std::cout << "   Kung may modulus switching mula Q_large → Q_small,\n";
    std::cout << "   ang L(k) ay HINDI nagbabago!\n";
    std::cout << "   → Ang NAND formula ay PAREHO!\n";
    std::cout << "   → Ang noise refresh ay NATURAL!\n\n";

    std::cout << "7. ANG TAMANG SWITCHING:\n";
    std::cout << "   ct_large = m·L(k) + r·Q_large\n";
    std::cout << "   ct_small = (ct_large · Q_small / Q_large) mod Q_small\n";
    std::cout << "   = m·L(k)·Q_small/Q_large + r·Q_small\n";
    std::cout << "   → Kailangan ng exact scaling\n\n";

    std::cout << "8. ANG PINAKA-SIMPLE:\n";
    std::cout << "   Dahil L(k) ay integer at universal,\n";
    std::cout << "   pwede tayong mag-encrypt sa Q_small directly:\n";
    std::cout << "   ct_small = m·L(k) + r·Q_small\n";
    std::cout << "   → Walang kailangan na switching!\n";
    std::cout << "   → Gumagana ang NAND sa Q_small!\n";

    return 0;
}

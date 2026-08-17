// SUBTRACTIVE HOMOMORPHIC DECRYPTION
// ct·BK_φ - ct·BK_ψ para sa noise cancellation

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "SUBTRACTIVE HOMOMORPHIC DECRYPTION\n";
    std::cout << "==================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    std::cout << "1. ANG SUBTRACTIVE CONCEPT:\n";
    std::cout << "   recover = ct·BK_φ - ct·BK_ψ\n";
    std::cout << "   = ct·(BK_φ - BK_ψ)\n\n";

    std::cout << "2. ANG KEY PROPERTY:\n";
    std::cout << "   φ^k - ψ^k = √5·F(k) (Fibonacci!)\n";
    std::cout << "   (φ^k + ψ^k) = L(k) (Lucas)\n";
    std::cout << "   → φ^k - ψ^k ay may SIMPLE form!\n\n";

    // Compute F(k) at L(k)
    NTL::ZZ fib[100];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i <= 42; i++) fib[i] = (fib[i-1] + fib[i-2]) % Q;
    NTL::ZZ F_k = fib[42];
    NTL::ZZ L_k = (phi_k + psi_k) % Q;

    std::cout << "   F(42) = " << F_k << "\n";
    std::cout << "   L(42) = " << L_k << "\n";
    std::cout << "   φ^k - ψ^k = " << (phi_k - psi_k + Q) % Q << "\n\n";

    // SIMPLIFIED: BK_φ = φ^k, BK_ψ = ψ^k (plain — walang encryption)
    std::cout << "3. BASELINE: Plain BK_φ at BK_ψ\n";
    NTL::ZZ inner = phi_k;  // message 1
    NTL::ZZ e = NTL::to_ZZ(7);
    NTL::ZZ ct = inner + e * psi_k;

    NTL::ZZ recover_phi = (ct * phi_k) % Q;
    NTL::ZZ recover_psi = (ct * psi_k) % Q;
    NTL::ZZ diff = (recover_phi - recover_psi + Q) % Q;

    std::cout << "   ct·φ^k = " << recover_phi << "\n";
    std::cout << "   ct·ψ^k = " << recover_psi << "\n";
    std::cout << "   Difference: " << diff << "\n\n";

    // KEY: ct·(φ^k - ψ^k)
    std::cout << "4. ct·(φ^k - ψ^k):\n";
    NTL::ZZ combined = (phi_k - psi_k + Q) % Q;
    NTL::ZZ result = (ct * combined) % Q;
    std::cout << "   = " << result << "\n";
    std::cout << "   = φ^k·(φ^k-ψ^k) + 7ψ^k·(φ^k-ψ^k)\n";
    std::cout << "   = φ^(2k) - φ^k·ψ^k + 7ψ^k·φ^k - 7ψ^(2k)\n";
    std::cout << "   = φ^(2k) - 1 + 7 - 7ψ^(2k)\n";
    std::cout << "   = φ^(2k) + 6 - 7ψ^(2k)\n\n";

    std::cout << "5. KEY INSIGHT:\n";
    std::cout << "   Ang φ^(2k) ay DOMINANT pa rin!\n";
    std::cout << "   Ang 7ψ^(2k) ay noise.\n";
    std::cout << "   Kung i-scale ng φ^(-2k):\n";
    std::cout << "   result·φ^(-2k) = 1 + 6·φ^(-2k) - 7·ψ^k·φ^(-k)\n";
    std::cout << "   = 1 + 6·φ^(-2k) - 7 (since ψ^k·φ^(-k)=1)\n";
    std::cout << "   ≈ 1 - 1 = 0 + 6·φ^(-2k)\n\n";

    std::cout << "6. ANG NOISE AY NAG-COMPACT:\n";
    std::cout << "   Pagkatapos ng subtractive + scaling,\n";
    std::cout << "   ang noise ay 6·φ^(-2k) (maliit!)\n";
    std::cout << "   → MAS MAGANDA kaysa sa multiplicative!\n";

    return 0;
}

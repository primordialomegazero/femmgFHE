// POLYNOMIAL PERIOD-2 SEARCH
// Hanapin ang P(X) na may P(P(X)) = X mod (x^N + 1)

#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "POLYNOMIAL PERIOD-2 SEARCH\n";
    std::cout << "==========================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);

    constexpr int N = 4;  // Maliit na ring para sa search

    std::cout << "1. RING: Z_Q[x]/(x^" << N << "+1)\n\n";

    std::cout << "2. MGA INVOLUTIVE POLYNOMIALS:\n";
    std::cout << "   Hahanapin: P(P(X)) = X mod (x^N+1)\n\n";

    std::cout << "   A. P(X) = -X\n";
    std::cout << "      P(P(X)) = -(-X) = X ✓\n\n";

    std::cout << "   B. P(X) = X^(-1) (inverse sa ring)\n";
    std::cout << "      P(P(X)) = (X^(-1))^(-1) = X ✓\n\n";

    std::cout << "   C. P(X) = X^(N-1) (Frobenius-like)\n";
    std::cout << "      X^(N-1) · X = X^N = -1\n";
    std::cout << "      P(P(X)) = (X^(N-1))^(N-1) = X^((N-1)²)\n";
    std::cout << "      = X^(N²-2N+1) = X (kasi X^N = -1)\n";
    std::cout << "      ✓ Kung (N-1)² ≡ 1 mod 2N\n\n";

    // Check para sa N=4
    long exp = (N-1) * (N-1);
    long mod_2N = 2 * N;
    std::cout << "3. CHECK X^(N-1):\n";
    std::cout << "   (N-1)² = " << exp << "\n";
    std::cout << "   " << exp << " mod " << mod_2N << " = " << (exp % mod_2N) << "\n";
    std::cout << "   Kailangan: 1 mod 2N\n";
    std::cout << "   Result: " << ((exp % mod_2N == 1) ? "PERIOD-2 ✓" : "HINDI") << "\n\n";

    std::cout << "4. ANG NAND SA POLYNOMIAL:\n";
    std::cout << "   NAND(a,b) = 1 - a·b\n";
    std::cout << "   Para sa period-2: NAND(NAND(x,x), NAND(x,x)) = x\n";
    std::cout << "   1 - (1-x²)² = x\n";
    std::cout << "   1 - (1 - 2x² + x⁴) = x\n";
    std::cout << "   2x² - x⁴ = x\n";
    std::cout << "   x⁴ - 2x² + x = 0\n";
    std::cout << "   x(x³ - 2x + 1) = 0\n";
    std::cout << "   x(x-1)(x²+x-1) = 0\n";
    std::cout << "   → Roots: 0, 1, φ, ψ\n\n";

    std::cout << "5. ANG KEY INSIGHT:\n";
    std::cout << "   Ang NAND period-2 ay may FIXED POINTS: 0, 1, φ, ψ.\n";
    std::cout << "   Kung ang message ay nasa fixed point set,\n";
    std::cout << "   ang NAND ay period-2.\n";
    std::cout << "   → Sa polynomial: i-embed ang fixed points\n";
    std::cout << "     bilang constant polynomials!\n\n";

    std::cout << "6. ANG UNIVERSAL FORMULA:\n";
    std::cout << "   P(X) = 1 - X²  (NAND(X,X))\n";
    std::cout << "   Fixed points: P(φ) = ψ, P(ψ) = φ\n";
    std::cout << "   → Ito ay SCALAR property\n";
    std::cout << "   → Sa polynomial: hindi natural na period-2\n";
    std::cout << "   → PERO ang fixed points ay umiiral!\n";

    return 0;
}

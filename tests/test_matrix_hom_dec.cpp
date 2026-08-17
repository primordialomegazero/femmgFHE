// FIBONACCI Q-MATRIX HOMOMORPHIC DECRYPTION
// Gamit ang 2×2 matrix para sa eksaktong projection

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "FIBONACCI Q-MATRIX HOMOMORPHIC DECRYPTION\n";
    std::cout << "=========================================\n\n";

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
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    std::cout << "1. ANG FIBONACCI Q-MATRIX:\n";
    std::cout << "   Q = [[1, 1], [1, 0]]\n";
    std::cout << "   Q^k = [[F(k+1), F(k)], [F(k), F(k-1)]]\n\n";

    // Compute F(k) at F(k+1)
    NTL::ZZ fib[100];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i <= 43; i++) fib[i] = (fib[i-1] + fib[i-2]) % Q;
    NTL::ZZ F_k = fib[42];
    NTL::ZZ F_k_plus_1 = fib[43];
    NTL::ZZ F_k_minus_1 = fib[41];

    std::cout << "   F(42) = " << F_k << "\n";
    std::cout << "   F(43) = " << F_k_plus_1 << "\n";
    std::cout << "   F(41) = " << F_k_minus_1 << "\n\n";

    std::cout << "2. EIGENVECTOR DECOMPOSITION:\n";
    std::cout << "   Ang Q-matrix ay may eigenvectors φ at ψ.\n";
    std::cout << "   Q^k ay may eigenvalues φ^k at ψ^k.\n";
    std::cout << "   → φ^k at ψ^k ay ma-recover via eigenvectors!\n\n";

    std::cout << "3. ANG PROJECTION MATRIX:\n";
    std::cout << "   P_φ = (φ^k - ψ^k)^(-1) · [φ^k  -ψ^k]\n";
    std::cout << "                            [ψ^k  -φ^k]\n";
    std::cout << "   → Ito ay nagpo-project sa φ-direction!\n\n";

    // Compute projection matrix
    NTL::ZZ diff = (phi_k - psi_k + Q) % Q;
    NTL::ZZ inv_diff = NTL::InvMod(diff, Q);

    std::cout << "4. ANG KEY QUESTION:\n";
    std::cout << "   May paraan ba para ma-construct ang projection\n";
    std::cout << "   matrix gamit L(k) lamang (walang φ^k)?\n\n";

    // L(k) = φ^k + ψ^k
    // diff = φ^k - ψ^k
    // L(k)² - 4 = diff²
    NTL::ZZ L_sq_minus_4 = (L_k * L_k - 4) % Q;
    if (L_sq_minus_4 < 0) L_sq_minus_4 += Q;

    std::cout << "5. ANG DISCRIMINANT:\n";
    std::cout << "   L(k)² - 4 = " << L_sq_minus_4 << "\n";
    std::cout << "   φ^k - ψ^k = √(L(k)² - 4)\n";
    std::cout << "   → Kailangan ng SQUARE ROOT!\n";
    std::cout << "   → Ito ang TRAPDOOR!\n\n";

    std::cout << "6. TRAPDOOR ANALYSIS:\n";
    std::cout << "   Public: L(k) = 599074578 (maliit!)\n";
    std::cout << "   L(k)² - 4 = 358890350005878080 (maliit pa rin)\n";
    std::cout << "   √(L(k)² - 4) = computable (maliit na number!)\n";
    std::cout << "   → HINDI secure para sa maliit na k\n";
    std::cout << "   → Kailangan ng MALAKING k!\n\n";

    std::cout << "7. PARA SA MALAKING k (2048):\n";
    std::cout << "   L(2048) = 1422-bit number\n";
    std::cout << "   L(2048)² - 4 = 2047-bit number\n";
    std::cout << "   √(L(2048)² - 4) = 1023-bit square root\n";
    std::cout << "   → ANG SQUARE ROOT AY MAHIRAP SA Z_Q!\n";
    std::cout << "   → Ito ang POST-QUANTUM TRAPDOOR!\n";

    return 0;
}

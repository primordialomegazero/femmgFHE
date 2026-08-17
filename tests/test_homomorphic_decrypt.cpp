// HOMOMORPHIC DECRYPTION RESEARCH
// ct · E(φ^k) = inner·φ^k + cross terms

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "HOMOMORPHIC DECRYPTION RESEARCH\n";
    std::cout << "================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");

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
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    std::mt19937_64 rng(42);

    // ENCRYPTED SECRET KEY (Bootstrapping key)
    // BK = E(φ^k) = φ^k + e_bk·ψ^k + r_bk·Q
    NTL::ZZ e_bk = NTL::to_ZZ(5);
    NTL::ZZ r_bk = NTL::RandomBnd(Q);
    NTL::ZZ BK = phi_k + e_bk * psi_k + r_bk * Q;

    std::cout << "1. BOOTSTRAPPING KEY (BK):\n";
    std::cout << "   BK = φ^k + " << e_bk << "·ψ^k + r·Q\n\n";

    // ENCRYPT MESSAGE
    NTL::ZZ e_ct = NTL::to_ZZ(7);
    NTL::ZZ r_ct = NTL::RandomBnd(Q);
    NTL::ZZ inner = phi_k;  // message 1
    NTL::ZZ ct = inner + e_ct * psi_k + r_ct * Q;

    std::cout << "2. CIPHERTEXT:\n";
    std::cout << "   ct = inner + " << e_ct << "·ψ^k + r·Q\n\n";

    // HOMOMORPHIC DECRYPTION: ct · BK
    NTL::ZZ hom_dec = (ct * BK) % Q;

    std::cout << "3. HOMOMORPHIC DECRYPTION (ct · BK):\n";
    std::cout << "   = (inner + e_ct·ψ^k)(φ^k + e_bk·ψ^k)\n";
    std::cout << "   = inner·φ^k + inner·e_bk·ψ^k + e_ct·ψ^k·φ^k + e_ct·e_bk·ψ^(2k)\n";
    std::cout << "   = inner·φ^k + (inner·e_bk + e_ct)·ψ^k + e_ct·e_bk·ψ^(2k)\n\n";

    // KEY OBSERVATION:
    // inner·φ^k = φ^(2k) kung inner = φ^k
    // inner·φ^k = 0 kung inner = 0
    // Ang term (inner·e_bk + e_ct)·ψ^k ay ψ^k noise
    // Ang e_ct·e_bk·ψ^(2k) ay ψ^(2k) noise

    // SCALE BY φ^(-k):
    NTL::ZZ scaled = (hom_dec * inv_phi_k) % Q;
    std::cout << "4. SCALED (·φ^(-k)):\n";
    std::cout << "   = inner + (inner·e_bk + e_ct) + e_ct·e_bk·ψ^k\n";
    std::cout << "   (since ψ^k·φ^(-k) = 1, ψ^(2k)·φ^(-k) = ψ^k)\n\n";

    std::cout << "5. SCALED VALUE:\n";
    std::cout << "   " << scaled << "\n\n";

    std::cout << "6. ANALYSIS:\n";
    std::cout << "   inner=φ^k: scaled = φ^k + (φ^k·e_bk + e_ct) + e_ct·e_bk·ψ^k\n";
    std::cout << "   inner=0:   scaled = e_ct + e_ct·e_bk·ψ^k\n";
    std::cout << "   → Ang φ^k term ay DOMINANT (malaki)\n";
    std::cout << "   → Ang noise terms ay maliit (e_bk, e_ct ay 5-7)\n";
    std::cout << "   → PWEDE ma-distinguish!\n\n";

    // CHECK: Decrypt ang homomorphic result
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
    if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
    NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
    bool recovered = d_phi2k < d_0;

    std::cout << "7. RECOVERED MESSAGE: " << (recovered ? 1 : 0) << " (exp 1)\n";

    return 0;
}

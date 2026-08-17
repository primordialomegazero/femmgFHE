// LUCAS-ENCRYPTED BOOTSTRAPPING KEY
// BK = E(φ^k) na may Lucas trapdoor

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "LUCAS-ENCRYPTED BK RESEARCH\n";
    std::cout << "===========================\n\n";

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

    std::mt19937_64 rng(42);

    std::cout << "1. ANG IDEA:\n";
    std::cout << "   BK = φ^k + e·ψ^k (encrypted φ^k)\n";
    std::cout << "   Ang evaluator ay may BK, hindi φ^k\n\n";

    std::cout << "2. HOMOMORPHIC PROJECTION:\n";
    std::cout << "   ct·BK = (inner + e_ct·ψ^k)(φ^k + e_bk·ψ^k)\n";
    std::cout << "   = inner·φ^k + inner·e_bk·ψ^k + e_ct·ψ^k·φ^k + e_ct·e_bk·ψ^(2k)\n";
    std::cout << "   = inner·φ^k + (inner·e_bk + e_ct) + e_ct·e_bk·ψ^(2k)\n\n";

    std::cout << "3. ANG LUCAS CANCELLATION:\n";
    std::cout << "   ψ^(2k) = L(k)·ψ^k - 1\n";
    std::cout << "   → e_ct·e_bk·ψ^(2k) = e_ct·e_bk·L(k)·ψ^k - e_ct·e_bk\n";
    std::cout << "   → Ang ψ^(2k) ay na-decompose sa ψ^k at constant\n\n";

    std::cout << "4. ANG TAMANG BK CONSTRUCTION:\n";
    std::cout << "   BK = φ^k + e_bk·ψ^k\n";
    std::cout << "   + L(k)·e_bk·ψ^k - e_bk  (Lucas correction)\n";
    std::cout << "   → Ito ay magbibigay ng eksaktong cancellation!\n\n";

    // TEST: BK na may Lucas correction
    std::cout << "5. TEST WITH LUCAS CORRECTION:\n";
    NTL::ZZ e_bk = NTL::to_ZZ(5);
    NTL::ZZ BK_corrected = phi_k + e_bk * psi_k + L_k * e_bk * psi_k - e_bk;
    BK_corrected = BK_corrected % Q;
    if (BK_corrected < 0) BK_corrected += Q;

    NTL::ZZ inner = phi_k;  // message 1
    NTL::ZZ e_ct = NTL::to_ZZ(7);
    NTL::ZZ ct = inner + e_ct * psi_k;

    NTL::ZZ result = (ct * BK_corrected) % Q;
    NTL::ZZ scaled = (result * inv_phi_k) % Q;

    std::cout << "   BK_corrected·ct·φ^(-k) = " << scaled << "\n";
    std::cout << "   Expected: ~φ^k (message 1)\n\n";

    // SUBUKAN SA M=0
    NTL::ZZ ct0 = NTL::to_ZZ(0) + e_ct * psi_k;
    NTL::ZZ result0 = (ct0 * BK_corrected) % Q;
    NTL::ZZ scaled0 = (result0 * inv_phi_k) % Q;

    std::cout << "6. TEST M=0:\n";
    std::cout << "   ct0·BK·φ^(-k) = " << scaled0 << "\n";
    std::cout << "   Expected: ~0 (message 0)\n\n";

    // DECRYPTION TEST
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    auto check_msg = [&](NTL::ZZ scaled_val) {
        NTL::ZZ d_phi2k = (scaled_val > phi_2k) ? scaled_val - phi_2k : phi_2k - scaled_val;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled_val < Q/2) ? scaled_val : Q - scaled_val;
        return d_phi2k < d_0;
    };

    std::cout << "7. RECOVERY:\n";
    std::cout << "   Message 1: " << (check_msg(scaled) ? 1 : 0) << " (exp 1)\n";
    std::cout << "   Message 0: " << (check_msg(scaled0) ? 1 : 0) << " (exp 0)\n";

    return 0;
}

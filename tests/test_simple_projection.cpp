// SIMPLE PROJECTION — walang center normalization
// Direktang projection sa φ^k at ψ^k directions

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  SIMPLE PROJECTION TEST\n";
    std::cout << "  Walang center normalization\n";
    std::cout << "========================================\n\n";

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
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);

    // ENCRYPTION: ct = m·φ^k + e·ψ^k
    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    // DECODE: ct · φ^(-k) mod Q
    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };

    // DECODE: ct · ψ^(-k) mod Q
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };

    std::cout << "1. DIRECT DECODE (walang center):\n";
    std::cout << "   ct·φ^(-k) at ct·ψ^(-k)\n\n";

    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
            auto ct = encrypt(m, e);
            NTL::ZZ dec_phi = decode_phi(ct);
            NTL::ZZ dec_psi = decode_psi(ct);
            std::cout << "   m=" << m << " e=" << e
                      << " | φ-decode=" << dec_phi
                      << " ψ-decode=" << dec_psi << "\n";
        }
    }
    std::cout << "\n";

    // Ang φ-decode ay dapat m + e·ψ^k·φ^(-k)
    // ψ^k·φ^(-k) = (ψ·φ^(-1))^k
    // ψ·φ^(-1) = ψ·(ψ+1) = ψ²+ψ = (ψ+1)+ψ = 2ψ+1
    // Ang value ay HINDI 1! Kasi ψ·φ^(-1) ≠ 1
    //
    // ψ·φ^(-1) = ψ·(-ψ) = -ψ² = -(ψ+1) = -ψ-1
    // ≠ 1

    NTL::ZZ psi_over_phi = (psi * inv_phi_k) % Q;
    std::cout << "2. ψ·φ^(-1) = " << psi_over_phi << "\n";
    std::cout << "   (dapat ay -ψ-1 kung ψ²=ψ+1)\n\n";

    // ANG TOTOO: ψ·φ = -1 (hindi 1)
    // Kaya ψ^k·φ^(-k) = (ψ·φ^(-1))^k
    // ψ·φ^(-1) = ψ·(-ψ) = -ψ² = -(ψ+1) = -ψ-1
    // ≠ 1
    // Kaya ang decode ay HINDI m + e

    // ANG TAMANG DECODE AY:
    // ct·ψ^k = m·φ^k·ψ^k + e·ψ^(2k) = m·(-1)^k + e·ψ^(2k) = m + e·ψ^(2k)
    // Kung k=42 even, (-1)^k = 1
    // Kaya: ct·ψ^k = m + e·ψ^(2k)

    auto decode_correct = [&](NTL::ZZ ct) {
        return (ct * psi_k) % Q;
    };

    std::cout << "3. CORRECT DECODE: ct·ψ^k mod Q\n\n";
    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
            auto ct = encrypt(m, e);
            NTL::ZZ dec = decode_correct(ct);
            std::cout << "   m=" << m << " e=" << e
                      << " → decoded=" << dec << "\n";
        }
    }
    std::cout << "\n";

    // Ngayon ang decoded = m + e·ψ^(2k)
    // Para sa m=0: decoded = e·ψ^(2k)
    // Para sa m=1: decoded = 1 + e·ψ^(2k)
    // ANG DIFFERENCE AY 1 LANG!
    // Ibig sabihin: ang decoded value ay nag-e-encode ng m bilang offset

    std::cout << "4. KEY INSIGHT:\n";
    std::cout << "   decoded(m=1) - decoded(m=0) = 1\n";
    std::cout << "   Ang m ay offset, ang e·ψ^(2k) ay baseline\n\n";

    // Test
    auto ct_0 = encrypt(0, NTL::to_ZZ(5));
    auto ct_1 = encrypt(1, NTL::to_ZZ(5));
    NTL::ZZ dec_0 = decode_correct(ct_0);
    NTL::ZZ dec_1 = decode_correct(ct_1);
    std::cout << "   dec(m=1) - dec(m=0) = " << (dec_1 - dec_0) << "\n";
    std::cout << "   Mod Q: " << ((dec_1 - dec_0 + Q) % Q) << "\n";

    return 0;
}

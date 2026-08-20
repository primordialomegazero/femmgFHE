// DUAL-CHANNEL NOISE ISOLATION
// Ang paired encoding ay may dalawang channels:
//   Channel A: ct_a = m·φ^k + e·ψ^k
//   Channel B: ct_b = m·ψ^k + e·φ^k
//
// Ang difference ng decoded values ay dapat independent sa m

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  DUAL-CHANNEL NOISE ISOLATION\n";
    std::cout << "  Hahanapin: ang m-independent noise\n";
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

    auto center = [&](NTL::ZZ v) {
        if (v > Q/2) v -= Q;
        return v;
    };

    // PAIRED ENCRYPTION
    auto encrypt_paired = [&](int m, NTL::ZZ e) {
        return std::make_pair(
            (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q,
            (NTL::to_ZZ(m) * psi_k + e * phi_k) % Q
        );
    };

    // DECODE EACH CHANNEL SEPARATELY
    auto decode_channel_A = [&](auto ct) {
        return center((ct.first * inv_phi_k) % Q);  // m + e·ψ^k·φ^(-k) = m + e
    };
    auto decode_channel_B = [&](auto ct) {
        return center((ct.second * inv_psi_k) % Q);  // m + e·φ^k·ψ^(-k) = m + e
    };

    // Ang dalawang decoded ay pareho: parehong m + e
    // PERO ang channel A at B ay may iba't ibang error sa computation
    // Kasi ang ψ^k·φ^(-k) = 1 pero ang φ^k·ψ^(-k) = 1
    // Pareho silang 1! (k=42 even)

    std::cout << "1. VERIFY: ψ^k·φ^(-k) = φ^k·ψ^(-k) = 1\n";
    NTL::ZZ test1 = center((psi_k * inv_phi_k) % Q);
    NTL::ZZ test2 = center((phi_k * inv_psi_k) % Q);
    std::cout << "   ψ^k·φ^(-k) = " << test1 << "\n";
    std::cout << "   φ^k·ψ^(-k) = " << test2 << "\n\n";

    // Kaya ang dalawang channels ay theoretically pareho
    // HINDI sila independent!
    // Ang paired encoding ay hindi nagbibigay ng independent channels

    std::cout << "2. PAIRED DECODE:\n";
    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(5)}) {
            auto ct = encrypt_paired(m, e);
            NTL::ZZ dec_A = decode_channel_A(ct);
            NTL::ZZ dec_B = decode_channel_B(ct);
            std::cout << "   m=" << m << " e=" << e
                      << " → A=" << dec_A << " B=" << dec_B << "\n";
        }
    }
    std::cout << "\n";

    // ANG TOTOONG PROBLEMA:
    // Ang signal at noise ay nasa parehong subspace
    // Kailangan natin ng encoding na naghihiwalay sa kanila
    //
    // SOLUTION: GAMITIN ANG L-Direction at F-Direction
    // Signal: L-direction (φ^k + ψ^k)
    // Noise: F-direction (φ^k - ψ^k)

    // VERIFY: orthogonal ba ang L at F directions?
    NTL::ZZ L_k = NTL::to_ZZ(2);
    NTL::ZZ L_prev = NTL::to_ZZ(2);
    NTL::ZZ L_curr = NTL::to_ZZ(1);
    for (int i = 2; i <= 42; i++) {
        NTL::ZZ temp = L_curr;
        L_curr = (L_curr + L_prev) % Q;
        L_prev = temp;
    }
    L_k = L_curr;

    NTL::ZZ F_k = NTL::to_ZZ(1);
    NTL::ZZ F_prev = NTL::to_ZZ(0);
    NTL::ZZ F_curr = NTL::to_ZZ(1);
    for (int i = 2; i <= 42; i++) {
        NTL::ZZ temp = F_curr;
        F_curr = (F_curr + F_prev) % Q;
        F_prev = temp;
    }
    F_k = F_curr;

    NTL::ZZ D_k = (F_k * sqrt5) % Q;

    // L-direction at F-direction ay:
    // L = φ^k + ψ^k
    // D = φ^k - ψ^k
    //
    // Ang φ^k at ψ^k mismo ay:
    // φ^k = (L + D)/2
    // ψ^k = (L - D)/2
    //
    // Kaya ang encryption ay:
    // ct = m·φ^k + e·ψ^k = m·(L+D)/2 + e·(L-D)/2
    //    = (m+e)·L/2 + (m-e)·D/2
    //
    // DECODE sa L-direction: (m+e)/2
    // DECODE sa D-direction: (m-e)/2
    //
    // ANG DIFFERENCE AY INDEPENDENT SA m:
    // L-decode - D-decode = (m+e)/2 - (m-e)/2 = e
    // L-decode + D-decode = (m+e)/2 + (m-e)/2 = m

    std::cout << "3. ORTHOGONAL DECODING:\n";
    std::cout << "   L-decode at D-decode\n\n";

    auto decode_L = [&](NTL::ZZ ct) {
        NTL::ZZ result = center((ct * NTL::InvMod(L_k, Q)) % Q);
        return result;
    };
    auto decode_D = [&](NTL::ZZ ct) {
        NTL::ZZ result = center((ct * NTL::InvMod(D_k, Q)) % Q);
        return result;
    };

    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
            auto ct = encrypt_paired(m, e).first;  // Use channel A
            NTL::ZZ l_dec = decode_L(ct);
            NTL::ZZ d_dec = decode_D(ct);
            NTL::ZZ sum = l_dec + d_dec;
            NTL::ZZ diff = l_dec - d_dec;
            std::cout << "   m=" << m << " e=" << e
                      << " | L=" << l_dec << " D=" << d_dec
                      << " | L+D=" << sum << " L-D=" << diff << "\n";
        }
    }
    std::cout << "\n";

    std::cout << "4. RECOVERY FORMULA:\n";
    std::cout << "   m = L+D (signal)\n";
    std::cout << "   e = L-D (noise)\n\n";

    for (NTL::ZZ e : {NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
        for (int m : {0, 1}) {
            auto ct = encrypt_paired(m, e).first;
            NTL::ZZ l_dec = decode_L(ct);
            NTL::ZZ d_dec = decode_D(ct);
            NTL::ZZ recovered_m = l_dec + d_dec;
            NTL::ZZ recovered_e = l_dec - d_dec;
            std::cout << "   Actual: m=" << m << " e=" << e
                      << " → Recovered: m=" << recovered_m
                      << " e=" << recovered_e << "\n";
        }
    }

    return 0;
}

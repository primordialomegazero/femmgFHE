// MULTICHANNEL DEEP — 6 at 8 channels
// Tingnan kung mas maraming channels = mas mahusay

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  MULTICHANNEL DEEP TEST\n";
    std::cout << "  4, 6, at 8 channels\n";
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

    // Precompute channels
    std::vector<NTL::ZZ> phi_channels, psi_channels, inv_phi_channels, inv_psi_channels, offsets;

    for (int j = 1; j <= 8; j++) {
        NTL::ZZ phi_jk = NTL::to_ZZ(1);
        NTL::ZZ psi_jk = NTL::to_ZZ(1);
        for (int i = 0; i < 42 * j; i++) {
            phi_jk = (phi_jk * phi) % Q;
            psi_jk = (psi_jk * psi) % Q;
        }
        phi_channels.push_back(phi_jk);
        psi_channels.push_back(psi_jk);
        inv_phi_channels.push_back(NTL::InvMod(phi_jk, Q));
        inv_psi_channels.push_back(NTL::InvMod(psi_jk, Q));
        offsets.push_back((phi_jk * NTL::InvMod(psi_jk, Q)) % Q);
    }

    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };
    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

    auto decode_channel = [&](NTL::ZZ ct, int ch, bool is_phi) {
        if (is_phi) return (ct * inv_phi_channels[ch]) % Q;
        else return (ct * inv_psi_channels[ch]) % Q;
    };

    // ============================================
    // COMPARE: 4, 6, 8 channels
    // ============================================
    auto current = encrypt(1, NTL::to_ZZ(1));

    std::cout << "CHANNEL COMPARISON (30 depths, e=1):\n";
    std::cout << "--------------------------------------\n\n";

    // 4 channels: φ^k, ψ^k, φ^(2k), ψ^(2k)
    int correct_4ch = 0;
    // 6 channels: + φ^(3k), ψ^(3k)
    int correct_6ch = 0;
    // 8 channels: + φ^(4k), ψ^(4k)
    int correct_8ch = 0;

    current = encrypt(1, NTL::to_ZZ(1));

    for (int depth = 0; depth <= 30; depth++) {
        // Decode 4 channels
        int dec_phi_1 = (dist(decode_channel(current, 0, true), NTL::to_ZZ(1)) < 
                         dist(decode_channel(current, 0, true), NTL::to_ZZ(0))) ? 1 : 0;
        int dec_psi_1 = (dist(decode_channel(current, 0, false), offsets[0]) < 
                         dist(decode_channel(current, 0, false), NTL::to_ZZ(0))) ? 1 : 0;
        int dec_phi_2 = (dist(decode_channel(current, 1, true), NTL::to_ZZ(1)) < 
                         dist(decode_channel(current, 1, true), NTL::to_ZZ(0))) ? 1 : 0;
        int dec_psi_2 = (dist(decode_channel(current, 1, false), offsets[1]) < 
                         dist(decode_channel(current, 1, false), NTL::to_ZZ(0))) ? 1 : 0;

        // 6 channels: + φ^(3k), ψ^(3k)
        int dec_phi_3 = (dist(decode_channel(current, 2, true), NTL::to_ZZ(1)) < 
                         dist(decode_channel(current, 2, true), NTL::to_ZZ(0))) ? 1 : 0;
        int dec_psi_3 = (dist(decode_channel(current, 2, false), offsets[2]) < 
                         dist(decode_channel(current, 2, false), NTL::to_ZZ(0))) ? 1 : 0;

        // 8 channels: + φ^(4k), ψ^(4k)
        int dec_phi_4 = (dist(decode_channel(current, 3, true), NTL::to_ZZ(1)) < 
                         dist(decode_channel(current, 3, true), NTL::to_ZZ(0))) ? 1 : 0;
        int dec_psi_4 = (dist(decode_channel(current, 3, false), offsets[3]) < 
                         dist(decode_channel(current, 3, false), NTL::to_ZZ(0))) ? 1 : 0;

        int expected = (depth % 2 == 0) ? 1 : 0;

        // Majority voting
        int votes_4ch = dec_phi_1 + dec_psi_1 + dec_phi_2 + dec_psi_2;
        int votes_6ch = votes_4ch + dec_phi_3 + dec_psi_3;
        int votes_8ch = votes_6ch + dec_phi_4 + dec_psi_4;

        int dec_4ch = (votes_4ch >= 3) ? 1 : 0;  // majority of 4
        int dec_6ch = (votes_6ch >= 4) ? 1 : 0;  // majority of 6
        int dec_8ch = (votes_8ch >= 5) ? 1 : 0;  // majority of 8

        if (dec_4ch == expected) correct_4ch++;
        if (dec_6ch == expected) correct_6ch++;
        if (dec_8ch == expected) correct_8ch++;

        current = nand_op(current, current);
    }

    std::cout << "  4 channels: " << correct_4ch << "/31 = "
              << (100.0 * correct_4ch / 31) << "%\n";
    std::cout << "  6 channels: " << correct_6ch << "/31 = "
              << (100.0 * correct_6ch / 31) << "%\n";
    std::cout << "  8 channels: " << correct_8ch << "/31 = "
              << (100.0 * correct_8ch / 31) << "%\n\n";

    // ============================================
    // MAS MALALIM: 50 depths na may 8 channels
    // ============================================
    std::cout << "DEEP TEST: 50 depths, 8 channels\n";
    std::cout << "----------------------------------\n";

    current = encrypt(1, NTL::to_ZZ(1));
    int correct_50 = 0;
    int total_50 = 0;

    for (int depth = 0; depth <= 50; depth++) {
        int votes = 0;
        for (int ch = 0; ch < 4; ch++) {
            int phi_dec = (dist(decode_channel(current, ch, true), NTL::to_ZZ(1)) < 
                           dist(decode_channel(current, ch, true), NTL::to_ZZ(0))) ? 1 : 0;
            int psi_dec = (dist(decode_channel(current, ch, false), offsets[ch]) < 
                           dist(decode_channel(current, ch, false), NTL::to_ZZ(0))) ? 1 : 0;
            votes += phi_dec + psi_dec;
        }

        int dec = (votes >= 5) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        if (dec == expected) correct_50++;
        total_50++;

        current = nand_op(current, current);
    }

    std::cout << "  50-depth 8-channel: " << correct_50 << "/" << total_50 << " = "
              << (100.0 * correct_50 / total_50) << "%\n";

    return 0;
}

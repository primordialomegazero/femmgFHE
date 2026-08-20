// NOISE SWEET SPOT SCAN
// Hanapin kung saan ang normalization ay perfect

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  NOISE SWEET SPOT SCAN\n";
    std::cout << "  Sa anong noise level perfect ang normalization?\n";
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
    NTL::ZZ offset = (phi_k * inv_psi_k) % Q;

    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };
    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto normalize = [&](NTL::ZZ ct) {
        NTL::ZZ d_phi = decode_phi(ct);
        NTL::ZZ d_psi = decode_psi(ct);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;

        int decision = (phi_dec == psi_dec) ? phi_dec : psi_dec;

        if (decision == 1) {
            return encrypt(1, NTL::to_ZZ(0));
        } else {
            return encrypt(0, NTL::to_ZZ(0));
        }
    };

    // Scan noise levels
    std::cout << "  Noise | 10 depths | 20 depths | 30 depths | 50 depths\n";
    std::cout << "  ------|-----------|-----------|-----------|----------\n";

    for (int e_val = 0; e_val <= 20; e_val++) {
        NTL::ZZ e = NTL::to_ZZ(e_val);

        std::vector<int> errors_at = {0, 0, 0, 0};
        std::vector<int> max_depths = {10, 20, 30, 50};

        for (int idx = 0; idx < 4; idx++) {
            int max_depth = max_depths[idx];
            auto current = encrypt(1, e);
            int errors = 0;

            for (int depth = 0; depth <= max_depth; depth++) {
                NTL::ZZ d_phi = decode_phi(current);
                int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
                int expected = (depth % 2 == 0) ? 1 : 0;

                if (phi_dec != expected) errors++;

                current = nand_op(current, current);
                current = normalize(current);
            }

            errors_at[idx] = errors;
        }

        std::cout << "  e=" << e_val << "    | "
                  << errors_at[0] << "/10     | "
                  << errors_at[1] << "/20     | "
                  << errors_at[2] << "/30     | "
                  << errors_at[3] << "/50\n";
    }

    return 0;
}

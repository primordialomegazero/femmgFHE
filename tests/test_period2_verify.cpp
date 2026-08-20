// PERIOD-2 CORRECTION VERIFICATION
// I-verify ang 100% accuracy sa iba't ibang conditions

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-2 CORRECTION VERIFICATION\n";
    std::cout << "  Bakit perfect sa period=2?\n";
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

    // ============================================
    // TEST 1: DIFFERENT INITIAL NOISE LEVELS
    // ============================================
    std::cout << "TEST 1: DIFFERENT INITIAL NOISE (100 depths, period=2)\n";
    std::cout << "=====================================================\n\n";

    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10), NTL::to_ZZ(50)}) {
        auto current = encrypt(1, e);
        int errors = 0;
        int total = 100;
        int gates_since = 0;

        for (int depth = 0; depth < total; depth++) {
            NTL::ZZ d_phi = decode_phi(current);
            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;

            if (phi_dec != expected) errors++;

            current = nand_op(current, current);
            gates_since++;

            if (gates_since >= 2) {
                NTL::ZZ d_psi = decode_psi(current);
                int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
                current = encrypt(psi_dec, e);
                gates_since = 0;
            }
        }

        std::cout << "  e=" << e << ": " << errors << "/" << total << " errors ("
                  << (100.0 * (total - errors) / total) << "%)\n";
    }

    // ============================================
    // TEST 2: DEEPER CHAIN (200 depths)
    // ============================================
    std::cout << "\nTEST 2: DEEPER CHAIN (200 depths, period=2)\n";
    std::cout << "============================================\n\n";

    auto current = encrypt(1, NTL::to_ZZ(1));
    int errors = 0;
    int total = 200;
    int gates_since = 0;

    for (int depth = 0; depth < total; depth++) {
        NTL::ZZ d_phi = decode_phi(current);
        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        if (phi_dec != expected) errors++;

        current = nand_op(current, current);
        gates_since++;

        if (gates_since >= 2) {
            NTL::ZZ d_psi = decode_psi(current);
            int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
            current = encrypt(psi_dec, NTL::to_ZZ(1));
            gates_since = 0;
        }
    }

    std::cout << "  200 depths: " << errors << "/" << total << " errors ("
              << (100.0 * (total - errors) / total) << "%)\n";

    // ============================================
    // TEST 3: 500 DEPTHS
    // ============================================
    std::cout << "\nTEST 3: 500 DEPTHS\n";
    std::cout << "====================\n\n";

    current = encrypt(1, NTL::to_ZZ(1));
    errors = 0;
    total = 500;
    gates_since = 0;

    for (int depth = 0; depth < total; depth++) {
        NTL::ZZ d_phi = decode_phi(current);
        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        if (phi_dec != expected) errors++;

        current = nand_op(current, current);
        gates_since++;

        if (gates_since >= 2) {
            NTL::ZZ d_psi = decode_psi(current);
            int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
            current = encrypt(psi_dec, NTL::to_ZZ(1));
            gates_since = 0;
        }
    }

    std::cout << "  500 depths: " << errors << "/" << total << " errors ("
              << (100.0 * (total - errors) / total) << "%)\n";

    return 0;
}

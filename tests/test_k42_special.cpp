// K=42 SPECIAL ANALYSIS
// 1. Bakit 42 ang optimal? — Lucas/Fibonacci connection?
// 2. Transition depth 2 — bakit mabilis?
// 3. Multiples ng 42 — may special property ba?

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  K=42 SPECIAL ANALYSIS\n";
    std::cout << "  Bakit 42 ang Optimal?\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

    // ============================================
    // TEST 1: LUCAS/FIBONACCI CONNECTION
    // ============================================
    std::cout << "TEST 1: LUCAS/FIBONACCI CONNECTION\n";
    std::cout << "====================================\n\n";

    // Compute Lucas numbers around 42
    std::vector<NTL::ZZ> lucas;
    NTL::ZZ l0 = NTL::to_ZZ(2);
    NTL::ZZ l1 = NTL::to_ZZ(1);
    lucas.push_back(l0);
    lucas.push_back(l1);
    for (int i = 2; i <= 50; i++) {
        NTL::ZZ temp = l1;
        l1 = (l1 + l0) % Q;
        l0 = temp;
        lucas.push_back(l1);
    }

    // Compute Fibonacci numbers around 42
    std::vector<NTL::ZZ> fib;
    NTL::ZZ f0 = NTL::to_ZZ(0);
    NTL::ZZ f1 = NTL::to_ZZ(1);
    fib.push_back(f0);
    fib.push_back(f1);
    for (int i = 2; i <= 50; i++) {
        NTL::ZZ temp = f1;
        f1 = (f1 + f0) % Q;
        f0 = temp;
        fib.push_back(f1);
    }

    std::cout << "  Lucas numbers:\n";
    std::cout << "    L(41) = " << lucas[41] << "\n";
    std::cout << "    L(42) = " << lucas[42] << "\n";
    std::cout << "    L(43) = " << lucas[43] << "\n\n";

    std::cout << "  Fibonacci numbers:\n";
    std::cout << "    F(41) = " << fib[41] << "\n";
    std::cout << "    F(42) = " << fib[42] << "\n";
    std::cout << "    F(43) = " << fib[43] << "\n\n";

    // Check: May special relationship ba sa 42?
    std::cout << "  Special properties ng 42:\n";
    std::cout << "    42 = 2 × 3 × 7\n";
    std::cout << "    42 = F(9) + F(8) = 34 + 8 = 42\n";
    std::cout << "    42 ay nasa Lucas sequence? ";
    bool is_lucas = false;
    for (int i = 0; i < lucas.size(); i++) {
        if (lucas[i] == NTL::to_ZZ(42)) {
            std::cout << "YES (L(" << i << "))";
            is_lucas = true;
            break;
        }
    }
    if (!is_lucas) std::cout << "NO";
    std::cout << "\n\n";

    // ============================================
    // TEST 2: TRANSITION DEPTH 2 — BAKIT?
    // ============================================
    std::cout << "TEST 2: TRANSITION DEPTH 2\n";
    std::cout << "============================\n\n";

    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);
    NTL::ZZ offset = (phi_k * inv_psi_k) % Q;

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

    auto current = encrypt(1, NTL::to_ZZ(1));
    std::cout << "  Depth | φ-dec | ψ-dec | φ correct? | ψ correct? | φ distance | ψ distance\n";
    std::cout << "  ------|-------|-------|------------|------------|------------|----------\n";

    for (int depth = 0; depth <= 10; depth++) {
        NTL::ZZ d_phi = decode_phi(current);
        NTL::ZZ d_psi = decode_psi(current);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        NTL::ZZ phi_dist = dist(d_phi, NTL::to_ZZ(expected));
        NTL::ZZ psi_dist = dist(d_psi, (expected == 1) ? offset : NTL::to_ZZ(0));

        std::cout << "  " << depth << "     | " << phi_dec << "     | " << psi_dec
                  << "     | " << (phi_dec == expected ? "YES" : "NO")
                  << "         | " << (psi_dec == expected ? "YES" : "NO")
                  << "         | " << phi_dist
                  << " | " << psi_dist << "\n";

        current = nand_op(current, current);
    }

    // ============================================
    // TEST 3: MULTIPLES NG 42
    // ============================================
    std::cout << "\nTEST 3: MULTIPLES NG 42\n";
    std::cout << "=========================\n\n";

    for (int k_val : {42, 84, 126, 168, 210}) {
        NTL::ZZ pk = NTL::to_ZZ(1);
        NTL::ZZ sk = NTL::to_ZZ(1);
        for (int i = 0; i < k_val; i++) {
            pk = (pk * phi) % Q;
            sk = (sk * psi) % Q;
        }
        NTL::ZZ ipk = NTL::InvMod(pk, Q);
        NTL::ZZ isk = NTL::InvMod(sk, Q);
        NTL::ZZ off = (pk * isk) % Q;

        auto enc = [&](int m, NTL::ZZ e) {
            return (NTL::to_ZZ(m) * pk + e * sk) % Q;
        };
        auto dec_p = [&](NTL::ZZ ct) {
            return (ct * ipk) % Q;
        };
        auto dec_s = [&](NTL::ZZ ct) {
            return (ct * isk) % Q;
        };
        auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
            NTL::ZZ prod = (a * b) % Q;
            NTL::ZZ scaled = (prod * ipk) % Q;
            NTL::ZZ result = (pk - scaled) % Q;
            if (result < 0) result += Q;
            return result;
        };

        auto curr = enc(1, NTL::to_ZZ(1));
        int psi_ok = 0, phi_ok = 0;
        int total = 30;

        for (int depth = 0; depth < total; depth++) {
            int psi_d = (dist(dec_s(curr), off) < dist(dec_s(curr), NTL::to_ZZ(0))) ? 1 : 0;
            int phi_d = (dist(dec_p(curr), NTL::to_ZZ(1)) < dist(dec_p(curr), NTL::to_ZZ(0))) ? 1 : 0;
            int exp = (depth % 2 == 0) ? 1 : 0;

            if (psi_d == exp) psi_ok++;
            if (phi_d == exp) phi_ok++;

            curr = nand(curr, curr);
        }

        std::cout << "  k=" << k_val << ": ψ=" << (100.0 * psi_ok / total)
                  << "% φ=" << (100.0 * phi_ok / total) << "%\n";
    }

    // ============================================
    // TEST 4: GOLDEN RATIO APPROXIMATION
    // ============================================
    std::cout << "\nTEST 4: GOLDEN RATIO APPROXIMATION\n";
    std::cout << "====================================\n\n";

    // F(42)/F(41) ≈ φ
    std::cout << "  F(42)/F(41) = " << fib[42] << "/" << fib[41] << "\n";
    std::cout << "  = " << (double)NTL::to_double(fib[42]) / NTL::to_double(fib[41]) << "\n";
    std::cout << "  φ = " << (double)NTL::to_double(phi) / NTL::to_double(Q) << "\n\n";

    // L(42)/F(42) ≈ √5
    std::cout << "  L(42)/F(42) = " << (double)NTL::to_double(lucas[42]) / NTL::to_double(fib[42]) << "\n";
    std::cout << "  √5 = 2.236067977...\n\n";

    return 0;
}

// φ-PROJECTION RESEARCH — Lightweight Test
// Hindi mabigat na CKKS — scalar NTL muna
// Para ma-verify ang projection theory

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-PROJECTION RESEARCH\n";
    std::cout << "  Scalar domain muna\n";
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

    // Lucas at Fibonacci
    NTL::ZZ L_k, F_k;
    NTL::ZZ L_prev = NTL::to_ZZ(2), L_curr = NTL::to_ZZ(1);
    for (int i = 2; i <= 42; i++) {
        NTL::ZZ temp = L_curr;
        L_curr = (L_curr + L_prev) % Q;
        L_prev = temp;
    }
    L_k = L_curr;

    NTL::ZZ F_prev = NTL::to_ZZ(0), F_curr = NTL::to_ZZ(1);
    for (int i = 2; i <= 42; i++) {
        NTL::ZZ temp = F_curr;
        F_curr = (F_curr + F_prev) % Q;
        F_prev = temp;
    }
    F_k = F_curr;

    std::cout << "L(42) = " << L_k << "\n";
    std::cout << "F(42) = " << F_k << "\n\n";

    // ============================================
    // PROJECTION TEST
    // ============================================
    std::cout << "PROJECTION TESTS:\n";
    std::cout << "=================\n\n";

    // φ^k = (L + F·√5)/2
    NTL::ZZ rhs_phi = ((L_k + F_k * sqrt5) % Q * inv2) % Q;
    std::cout << "φ^k from Lucas/Fibonacci: " << (rhs_phi == phi_k ? "MATCH" : "NO MATCH") << "\n";

    // ψ^k = (L - F·√5)/2
    NTL::ZZ rhs_psi = ((L_k - F_k * sqrt5 + Q) % Q * inv2) % Q;
    std::cout << "ψ^k from Lucas/Fibonacci: " << (rhs_psi == psi_k ? "MATCH" : "NO MATCH") << "\n\n";

    // ============================================
    // SEPARATOR TEST
    // ============================================
    std::cout << "SEPARATOR TEST:\n";
    std::cout << "=================\n\n";

    // Kung x = a·φ^k + b·ψ^k:
    // Paano ihiwalay ang a at b?
    //
    // x · φ^(-k) = a + b·ψ^k·φ^(-k) = a + b·(-1)^k = a + b (k even)
    // x · ψ^(-k) = a·φ^k·ψ^(-k) + b = a·(-1)^k + b = a + b (k even)
    //
    // Pareho! Kaya hindi ma-separate...
    //
    // PERO: may ibang approach
    // x · φ^(-k) + x · ψ^(-k) = 2(a + b)
    // x · φ^(-k) - x · ψ^(-k) = 0
    //
    // Hmm... kailangan ng ibang basis.

    // L-direction at F-direction:
    // L_k = φ^k + ψ^k
    // D_k = F_k·√5 = φ^k - ψ^k
    //
    // x = a·φ^k + b·ψ^k = a·(L+D)/2 + b·(L-D)/2
    // = (a+b)·L/2 + (a-b)·D/2
    //
    // Projection sa L: x · L^(-1) = (a+b)/2
    // Projection sa D: x · D^(-1) = (a-b)/2
    //
    // RECOVER:
    // a = L-proj + D-proj
    // b = L-proj - D-proj

    NTL::ZZ D_k = (F_k * sqrt5) % Q;
    NTL::ZZ inv_L = NTL::InvMod(L_k, Q);
    NTL::ZZ inv_D = NTL::InvMod(D_k, Q);

    // Test: a=1, b=0 → x = φ^k
    NTL::ZZ x = phi_k;
    NTL::ZZ L_proj = (x * inv_L) % Q;
    NTL::ZZ D_proj = (x * inv_D) % Q;
    NTL::ZZ recovered_a = (L_proj + D_proj) % Q;
    NTL::ZZ recovered_b = (L_proj - D_proj + Q) % Q;

    std::cout << "Test x=φ^k (a=1,b=0):\n";
    std::cout << "  L-proj = " << L_proj << "\n";
    std::cout << "  D-proj = " << D_proj << "\n";
    std::cout << "  Recovered a = " << recovered_a << " (expected 1)\n";
    std::cout << "  Recovered b = " << recovered_b << " (expected 0)\n\n";

    // Test: a=0, b=1 → x = ψ^k
    x = psi_k;
    L_proj = (x * inv_L) % Q;
    D_proj = (x * inv_D) % Q;
    recovered_a = (L_proj + D_proj) % Q;
    recovered_b = (L_proj - D_proj + Q) % Q;

    std::cout << "Test x=ψ^k (a=0,b=1):\n";
    std::cout << "  L-proj = " << L_proj << "\n";
    std::cout << "  D-proj = " << D_proj << "\n";
    std::cout << "  Recovered a = " << recovered_a << " (expected 0)\n";
    std::cout << "  Recovered b = " << recovered_b << " (expected 1)\n\n";

    // ============================================
    // NOISE CANCELLATION TEST
    // ============================================
    std::cout << "NOISE CANCELLATION TEST:\n";
    std::cout << "=========================\n\n";

    // Kung may noise e sa ψ-direction:
    // ct = φ^k + e·ψ^k
    // L-proj = (φ^k + e·ψ^k)·L^(-1) = (1+e)/2
    // D-proj = (φ^k + e·ψ^k)·D^(-1) = (1-e)/2
    //
    // Recovered signal: L-proj + D-proj = 1
    // Recovered noise: L-proj - D-proj = e
    
    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
        NTL::ZZ ct = (phi_k + e * psi_k) % Q;
        L_proj = (ct * inv_L) % Q;
        D_proj = (ct * inv_D) % Q;
        recovered_a = (L_proj + D_proj) % Q;
        recovered_b = (L_proj - D_proj + Q) % Q;

        std::cout << "  e=" << e << ": signal=" << recovered_a
                  << " noise=" << recovered_b << "\n";
    }

    return 0;
}

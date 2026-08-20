// NAND NOISE HANDLING — 3 OPTIONS SABAY-SABAY
// 1. Relinearization: I-project ang ψ^(2k) pabalik sa ψ^k at φ^k
// 2. Higher-order encoding: Mas maraming basis vectors
// 3. Normalization: I-round sa pinakamalapit na valid encoding

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND NOISE HANDLING — 3 OPTIONS\n";
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

    // ψ^(2k) at φ^(-k) para sa higher-order terms
    NTL::ZZ psi_2k = (psi_k * psi_k) % Q;
    NTL::ZZ phi_neg_k = inv_phi_k;
    NTL::ZZ psi_2k_phi_neg_k = (psi_2k * phi_neg_k) % Q;

    // Lucas at Fibonacci para sa relinearization
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

    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };

    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };

    // NAND na may tracking ng components
    auto nand_with_components = [&](NTL::ZZ a, NTL::ZZ b) {
        // a = m₁·φ^k + e₁·ψ^k
        // b = m₂·φ^k + e₂·ψ^k
        // a·b = m₁m₂·φ^(2k) + (m₁e₂+m₂e₁)·φ^k·ψ^k + e₁e₂·ψ^(2k)
        //     = m₁m₂·φ^(2k) + (m₁e₂+m₂e₁)·(-1)^k + e₁e₂·ψ^(2k)
        //     = m₁m₂·φ^(2k) + (m₁e₂+m₂e₁) + e₁e₂·ψ^(2k)  [k=42 even]
        
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    std::cout << "COMPONENTS:\n";
    std::cout << "  φ^k = " << phi_k << "\n";
    std::cout << "  ψ^k = " << psi_k << "\n";
    std::cout << "  ψ^(2k)·φ^(-k) = " << psi_2k_phi_neg_k << "\n\n";

    // ============================================
    // OPTION 1: RELINEARIZATION
    // ψ^(2k) = L(2k)·ψ^k - 1 (Lucas identity)
    // Kaya ψ^(2k)·φ^(-k) = L(2k)·ψ^k·φ^(-k) - φ^(-k)
    // = L(2k)·ψ^k·φ^(-k) - φ^(-k)
    // = L(2k)·(ψ/φ)^k - φ^(-k)
    // = L(2k)·(ψ/φ)^k - φ^(-k)
    // = L(2k)·(-1)^k·(ψ/φ)^k... actually mali
    // 
    // ψ^(2k) = ψ^k·ψ^k = ψ^k·(1-φ) = ψ^k - ψ^k·φ
    // ψ^k·φ = (φ·ψ)^k/ψ^(k-1)... complex
    //
    // SIMPLER: Lucas identity
    // ψ^(2k) + ψ^(-2k) = L(2k) hindi ito...
    //
    // PINAKA-SIMPLE: ψ^(2k)·φ^(-k) = ψ^k·(ψ^k·φ^(-k))
    // = ψ^k·(ψ·φ^(-1))^k
    // = ψ^k·(ψ·(-ψ))^k  [φ^(-1) = -ψ]
    // = ψ^k·(-ψ²)^k
    // = ψ^k·(-(ψ+1))^k
    // = ψ^k·(-1)^k·(ψ+1)^k
    // = ψ^k·(ψ+1)^k  [k=42 even]
    // = ψ^k·φ^k  [(ψ+1)=φ]
    // = (ψ·φ)^k = (-1)^k = 1
    //
    // WOW! ψ^(2k)·φ^(-k) = 1 !!!
    // Kaya ang cross term ay CONSTANT!

    std::cout << "OPTION 1: RELINEARIZATION\n";
    std::cout << "--------------------------\n";
    std::cout << "  ψ^(2k)·φ^(-k) = " << psi_2k_phi_neg_k << "\n";
    std::cout << "  Dapat = 1 (k=42 even)\n\n";

    // VERIFY: ψ^(2k)·φ^(-k) = (ψ·φ)^k·ψ^k·φ^(-k) = (-1)^k·(ψ/φ)^k
    NTL::ZZ psi_over_phi = (psi_k * inv_phi_k) % Q;
    std::cout << "  (ψ/φ)^k = " << psi_over_phi << "\n\n";

    // ============================================
    // OPTION 2: HIGHER-ORDER ENCODING
    // Base vectors: φ^k, ψ^k, ψ^(2k)·φ^(-k)
    // ============================================
    std::cout << "OPTION 2: HIGHER-ORDER ENCODING\n";
    std::cout << "--------------------------------\n";

    // Verifying linear independence
    std::cout << "  Base vectors:\n";
    std::cout << "    v1 = φ^k = " << phi_k << "\n";
    std::cout << "    v2 = ψ^k = " << psi_k << "\n";
    std::cout << "    v3 = ψ^(2k)·φ^(-k) = " << psi_2k_phi_neg_k << "\n\n";

    // Check determinant
    // Kung v3 ay linear combination ng v1 at v2, dependent sila
    // v3 = a·v1 + b·v2
    // Kailangan nating i-solve
    NTL::ZZ det = (phi_k * psi_k - psi_k * phi_k) % Q;  // 0
    std::cout << "  det(v1,v2) = " << det << " (dapat 0 kung dependent)\n\n";

    // ============================================
    // OPTION 3: NORMALIZATION
    // I-round sa pinakamalapit na valid encoding
    // ============================================
    std::cout << "OPTION 3: NORMALIZATION\n";
    std::cout << "------------------------\n";

    // Ang valid states: {0, φ^k, ψ^k, φ^k+ψ^k, ...}
    // Sa NAND, ang output ay dapat nasa {0, φ^k}
    // (0 = false, φ^k = true)
    //
    // Ang φ-decode ay dapat nasa {0, 1, small_noise}

    // Test NAND sa encoded domain
    std::cout << "  NAND truth table sa ψ-decode:\n\n";

    auto ct_0 = encrypt(0, NTL::to_ZZ(0));
    auto ct_1 = encrypt(1, NTL::to_ZZ(0));

    for (auto a : {ct_0, ct_1}) {
        for (auto b : {ct_0, ct_1}) {
            auto nand_result = nand_with_components(a, b);
            NTL::ZZ dec_phi = decode_phi(nand_result);
            NTL::ZZ dec_psi = decode_psi(nand_result);
            std::cout << "    NAND(" 
                      << (decode_phi(a) == 0 ? "0" : "1") << ","
                      << (decode_phi(b) == 0 ? "0" : "1") << ") = "
                      << "φ=" << dec_phi << " ψ=" << dec_psi << "\n";
        }
    }
    std::cout << "\n";

    // ============================================
    // KEY TEST: ANG ψ^(2k)·φ^(-k) BA AY 1?
    // ============================================
    std::cout << "KEY TEST: ψ^(2k)·φ^(-k)\n";
    std::cout << "----------------------------\n";
    std::cout << "  ψ^(2k)·φ^(-k) mod Q = " << psi_2k_phi_neg_k << "\n";
    std::cout << "  1 mod Q = " << NTL::to_ZZ(1) << "\n";
    std::cout << "  Match: " << (psi_2k_phi_neg_k == NTL::to_ZZ(1)) << "\n\n";

    // Kung ψ^(2k)·φ^(-k) = 1, ang NAND ay:
    // NAND = (1-m₁m₂)·φ^k - (m₁e₂+m₂e₁)·ψ^k - e₁e₂·1
    // = (1-m₁m₂)·φ^k - (m₁e₂+m₂e₁)·ψ^k - e₁e₂
    //
    // Ang huling term -e₁e₂ ay SCALAR na pwede i-subtract

    std::cout << "KUNG ψ^(2k)·φ^(-k) = 1:\n";
    std::cout << "  NAND = (1-m₁m₂)·φ^k - (m₁e₂+m₂e₁)·ψ^k - e₁e₂\n\n";

    // Test with actual computation
    std::cout << "VERIFICATION with m₁=1,e₁=1, m₂=1,e₂=1:\n";
    auto a = encrypt(1, NTL::to_ZZ(1));
    auto b = encrypt(1, NTL::to_ZZ(1));
    auto nand_result = nand_with_components(a, b);

    NTL::ZZ expected = (NTL::to_ZZ(0) * phi_k - NTL::to_ZZ(2) * psi_k - NTL::to_ZZ(1)) % Q;
    if (expected < 0) expected += Q;

    std::cout << "  Actual NAND = " << nand_result << "\n";
    std::cout << "  Expected    = " << expected << "\n";
    std::cout << "  Match: " << (nand_result == expected) << "\n";

    return 0;
}

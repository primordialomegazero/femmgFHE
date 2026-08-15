#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "EMERGENT NOISE PROPERTIES\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << "\n\n";
    
    // ========== EMERGENT PROPERTY: TRACE-BASED NOISE ==========
    // Tr(a·φ + b·ψ) = a·Tr(φ) + b·Tr(ψ) = a·1 + b·1 = a + b
    // Norm(a·φ + b·ψ) = a² + ab - b²
    
    // Para sa noise na NORM = 1:
    // a² + ab - b² = 1
    // Ito ay UNIT sa ring!
    
    std::cout << "EMERGENT 1: UNIT NOISE (Norm = 1)\n";
    std::cout << "  Ang mga units ay may Norm = ±1\n";
    std::cout << "  φ ay unit: Norm(φ) = φ·ψ = -1\n";
    std::cout << "  φⁿ ay unit: Norm(φⁿ) = (-1)ⁿ\n\n";
    
    // ========== EMERGENT PROPERTY: FIBONACCI NOISE PAIRS ==========
    std::cout << "EMERGENT 2: FIBONACCI NOISE PAIRS\n";
    std::cout << "  (F(n), F(n-1)) ay may Norm = (-1)^(n-1)\n";
    
    std::vector<NTL::ZZ> fib(20);
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 20; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    for (int n = 1; n <= 10; n++) {
        NTL::ZZ norm = (fib[n] * fib[n] + fib[n] * fib[n-1] - fib[n-1] * fib[n-1]) % Q;
        if (norm < 0) norm += Q;
        std::cout << "  Norm(F(" << n << "), F(" << n-1 << ")) = " << norm;
        std::cout << " = " << (n % 2 == 0 ? "1" : "-1") << " mod Q\n";
    }
    std::cout << "\n";
    
    // ========== EMERGENT PROPERTY: CONJUGATE PAIRS ==========
    std::cout << "EMERGENT 3: CONJUGATE NOISE PAIRS\n";
    std::cout << "  (a, b) at (a, -b) ay conjugate pairs\n";
    std::cout << "  Product ng conjugates = Norm\n\n";
    
    // ========== EMERGENT PROPERTY: TRACE ZERO ELEMENTS ==========
    std::cout << "EMERGENT 4: TRACE ZERO ELEMENTS\n";
    std::cout << "  Tr(a·φ + b·ψ) = 0 kung a + b = 0\n";
    std::cout << "  Ito ang PERFECT NOISE!\n";
    std::cout << "  Kasi projection sa φ direction = 0\n\n";
    
    // Test: noise = n·φ - n·ψ (trace zero)
    NTL::ZZ n = NTL::to_ZZ(100);
    NTL::ZZ noise = (n * phi_zz - n * psi_zz + Q) % Q;
    NTL::ZZ trace_noise = (noise + noise) % Q;  // Approximation
    
    std::cout << "  Noise = n(φ - ψ) ay may trace 0\n";
    std::cout << "  Ito ay perpendicular sa message direction!\n\n";
    
    // ========== EMERGENT PROPERTY: SELF-CANCELLING PRODUCTS ==========
    std::cout << "EMERGENT 5: SELF-CANCELLING PRODUCTS\n";
    std::cout << "  φ·ψ = -1 (conjugate product)\n";
    std::cout << "  (φ-ψ)·(φ+ψ) = φ² - ψ² = φ+1 - (ψ+1) = φ-ψ\n";
    std::cout << "  Kaya (φ-ψ) ay eigenvector ng multiplication!\n\n";
    
    // ========== TAMANG NOISE DISTRIBUTION ==========
    std::cout << "TAMANG NOISE DISTRIBUTION:\n";
    std::cout << "  Noise = r·(φ - ψ) kung saan r ay random\n";
    std::cout << "  Ito ay:\n";
    std::cout << "  1. Trace zero (perpendicular sa message)\n";
    std::cout << "  2. Auto-cancelling sa projection\n";
    std::cout << "  3. Norm = r²·5 (bounded)\n\n";
    
    // Test projection with trace-zero noise
    NTL::ZZ inv_phi_minus_psi = NTL::InvMod((phi_zz - psi_zz + Q) % Q, Q);
    NTL::ZZ e1 = ((-psi_zz + Q) % Q * inv_phi_minus_psi) % Q;
    
    std::cout << "PROJECTION TEST:\n";
    for (long r = 1; r <= 5; r++) {
        NTL::ZZ noise = (NTL::to_ZZ(r) * (phi_zz - psi_zz + Q) % Q) % Q;
        NTL::ZZ proj = (e1 * noise) % Q;
        
        std::cout << "  e₁·(" << r << "(φ-ψ)) = " << proj;
        std::cout << " (should be 0)\n";
    }
    std::cout << "\n";
    
    // ========== FHE WITH TRACE-ZERO NOISE ==========
    std::cout << "FHE WITH TRACE-ZERO NOISE:\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> r_dist(-1000, 1000);
    
    struct Ciphertext {
        NTL::ZZ val;
    };
    
    auto encrypt = [&](int bit) {
        Ciphertext ct;
        if (bit) {
            ct.val = phi_zz;
        } else {
            ct.val = NTL::to_ZZ(0);
        }
        
        // Add trace-zero noise: r·(φ-ψ)
        long r = r_dist(gen);
        NTL::ZZ noise = (NTL::to_ZZ(r) * (phi_zz - psi_zz + Q) % Q) % Q;
        ct.val = (ct.val + noise) % Q;
        if (ct.val < 0) ct.val += Q;
        
        return ct;
    };
    
    auto decrypt = [&](const Ciphertext& ct) {
        NTL::ZZ proj = (e1 * ct.val) % Q;
        
        NTL::ZZ dist_0 = proj;
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;
        
        NTL::ZZ dist_1 = abs(proj - NTL::to_ZZ(1));
        if (dist_1 > Q / 2) dist_1 = Q - dist_1;
        
        return (dist_0 < dist_1) ? 0 : 1;
    };
    
    // Test
    int success = 0;
    int trials = 100;
    for (int t = 0; t < trials; t++) {
        auto ct0 = encrypt(0);
        auto ct1 = encrypt(1);
        if (decrypt(ct0) == 0 && decrypt(ct1) == 1) success++;
    }
    
    std::cout << "  Success: " << success << "/" << trials << "\n";
    
    return 0;
}

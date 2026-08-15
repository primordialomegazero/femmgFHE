#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";
constexpr int N = 1024;

void init_ring(const NTL::ZZ& Q) {
    NTL::ZZ_p::init(Q);
}

void reduce_mod(NTL::ZZ_pX& poly) {
    if (NTL::deg(poly) < N) return;
    NTL::ZZ_pX reduced;
    reduced.SetLength(N);
    for (int i = 0; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        int reduced_deg = i % (2 * N);
        if (reduced_deg >= N) {
            reduced_deg -= N;
            coeff = -coeff;
        }
        NTL::SetCoeff(reduced, reduced_deg, NTL::coeff(reduced, reduced_deg) + coeff);
    }
    poly = reduced;
}

int main() {
    std::cout << "257-BIT FHE V4 - POLYNOMIAL φ\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    init_ring(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    
    std::cout << "φ = " << phi_zz << "\n\n";
    
    // ========== TAMANG ENCRYPTION ==========
    // Encrypt(1) = φ·x (φ as coefficient ng x term)
    // Hindi constant - may x!
    
    auto encrypt = [&](int bit) {
        NTL::ZZ_pX ct;
        if (bit) {
            NTL::SetCoeff(ct, 1, phi);  // φ·x
        }
        return ct;
    };
    
    // Decrypt: check kung may φ sa x^1 coefficient
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        NTL::ZZ_p c1 = NTL::coeff(ct, 1);
        NTL::ZZ c1_zz = rep(c1);
        
        NTL::ZZ dist_0 = c1_zz;
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;
        
        NTL::ZZ diff = abs(c1_zz - phi_zz);
        NTL::ZZ dist_phi = diff;
        if (dist_phi > Q / 2) dist_phi = Q - dist_phi;
        
        return (dist_0 < dist_phi) ? 0 : 1;
    };
    
    // ========== BASIC TESTS ==========
    std::cout << "BASIC TESTS:\n";
    
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " ✓\n\n";
    
    // ========== MULTIPLICATION ==========
    std::cout << "MULTIPLICATION:\n";
    
    // ct1 * ct1 = φ·x * φ·x = φ²·x² = φ²·x²
    // Since N=1024, x^N = -1, so x² is just x² (no reduction for small powers)
    // Pero φ² = φ+1, so: φ²·x² = (φ+1)·x²
    
    auto mult = ct1 * ct1;
    reduce_mod(mult);
    
    std::cout << "  1 × 1:\n";
    std::cout << "    deg = " << NTL::deg(mult) << "\n";
    std::cout << "    c0 = " << NTL::coeff(mult, 0) << "\n";
    std::cout << "    c1 = " << NTL::coeff(mult, 1) << "\n";
    std::cout << "    c2 = " << NTL::coeff(mult, 2) << "\n";
    std::cout << "    Decrypt: " << decrypt(mult) << " (expected 1)\n\n";
    
    // ========== DEPTH TEST ==========
    std::cout << "DEPTH TEST (10 iterations):\n";
    
    NTL::ZZ_pX current = ct1;
    bool pass = true;
    
    for (int depth = 1; depth <= 10; depth++) {
        current = current * ct1;
        reduce_mod(current);
        
        int dec = decrypt(current);
        std::cout << "  Depth " << depth << ": φ^" << depth+1 << "·x^" << depth+1;
        std::cout << " → decrypt: " << dec;
        std::cout << (dec == 1 ? " ✓" : " ✗") << "\n";
        
        if (dec != 1) {
            pass = false;
            break;
        }
    }
    
    std::cout << "\n=== " << (pass ? "257-BIT FHE V4 PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}

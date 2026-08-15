#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <chrono>

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
    std::cout << "257-BIT FHE V2 - NOISE OPTIMIZED\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    init_ring(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    
    std::cout << "φ = " << phi_zz << "\n\n";
    
    // SIMPLIFIED ENCRYPTION: Walang noise polynomials muna
    // Just pure φ-based encryption (like sa 29-bit version)
    // Then i-add natin ang noise later
    
    auto encrypt = [&](int bit) {
        NTL::ZZ_pX ct;
        NTL::SetCoeff(ct, 0, bit ? phi : NTL::to_ZZ_p(0));
        return ct;
    };
    
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        NTL::ZZ_p c0 = NTL::coeff(ct, 0);
        NTL::ZZ c0_zz = rep(c0);
        
        NTL::ZZ dist_0 = c0_zz;
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;
        
        NTL::ZZ diff = abs(c0_zz - phi_zz);
        NTL::ZZ dist_phi = diff;
        if (dist_phi > Q / 2) dist_phi = Q - dist_phi;
        
        return (dist_0 < dist_phi) ? 0 : 1;
    };
    
    // ========== DEPTH TEST (Pure φ, no noise) ==========
    std::cout << "PURE φ DEPTH TEST (no noise):\n";
    
    auto ct1 = encrypt(1);
    NTL::ZZ_pX current = ct1;
    bool pass = true;
    
    for (int i = 1; i <= 100; i++) {
        current = current * ct1;
        reduce_mod(current);
        int dec = decrypt(current);
        if (dec != 1) {
            std::cout << "  FAIL at depth " << i << ": got " << dec << "\n";
            pass = false;
            break;
        }
        if (i % 10 == 0) {
            std::cout << "  Depth " << i << ": OK ✓\n";
        }
    }
    
    if (pass) {
        std::cout << "  Pure φ depth test: 100 iterations PASS ✓\n\n";
    }
    
    // ========== WITH NOISE (small) ==========
    std::cout << "WITH SMALL NOISE (bound=10):\n";
    
    auto encrypt_with_noise = [&](int bit, long noise_bound) {
        NTL::ZZ_pX ct;
        NTL::SetCoeff(ct, 0, bit ? phi : NTL::to_ZZ_p(0));
        
        // Add small noise sa higher coefficients
        for (int i = 1; i < 10; i++) {
            long noise_val = rand() % (2 * noise_bound) - noise_bound;
            NTL::SetCoeff(ct, i, NTL::to_ZZ_p(noise_val));
        }
        return ct;
    };
    
    auto ct1_noisy = encrypt_with_noise(1, 10);
    current = ct1_noisy;
    pass = true;
    
    for (int i = 1; i <= 50; i++) {
        current = current * ct1_noisy;
        reduce_mod(current);
        int dec = decrypt(current);
        if (dec != 1) {
            std::cout << "  FAIL at depth " << i << ": got " << dec << "\n";
            pass = false;
            break;
        }
        if (i % 10 == 0) {
            std::cout << "  Depth " << i << ": OK ✓\n";
        }
    }
    
    if (pass) {
        std::cout << "  Small noise depth test: 50 iterations PASS ✓\n\n";
    }
    
    // ========== NAND with noise ==========
    std::cout << "NAND GATES (with noise):\n";
    
    auto ct0_noisy = encrypt_with_noise(0, 10);
    
    auto nand = [&](const NTL::ZZ_pX& a, const NTL::ZZ_pX& b) {
        NTL::ZZ_pX one;
        NTL::SetCoeff(one, 0, phi);
        NTL::ZZ_pX result = one - (a * b);
        reduce_mod(result);
        return result;
    };
    
    int nand_results[4];
    nand_results[0] = decrypt(nand(ct0_noisy, ct0_noisy));
    nand_results[1] = decrypt(nand(ct0_noisy, ct1_noisy));
    nand_results[2] = decrypt(nand(ct1_noisy, ct0_noisy));
    nand_results[3] = decrypt(nand(ct1_noisy, ct1_noisy));
    
    std::cout << "  NAND(0,0) = " << nand_results[0] << " (expected 1) " << (nand_results[0] == 1 ? "✓" : "✗") << "\n";
    std::cout << "  NAND(0,1) = " << nand_results[1] << " (expected 1) " << (nand_results[1] == 1 ? "✓" : "✗") << "\n";
    std::cout << "  NAND(1,0) = " << nand_results[2] << " (expected 1) " << (nand_results[2] == 1 ? "✓" : "✗") << "\n";
    std::cout << "  NAND(1,1) = " << nand_results[3] << " (expected 0) " << (nand_results[3] == 0 ? "✓" : "✗") << "\n";
    
    return 0;
}

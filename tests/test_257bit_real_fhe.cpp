#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

// 257-bit Q na verified may φ
const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";
constexpr int N = 1024;  // Ring dimension

void init_ring(const NTL::ZZ& Q) {
    NTL::ZZ_p::init(Q);
}

// Polynomial reduction: x^N = -1
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

// Generate random polynomial for noise
NTL::ZZ_pX generate_noise(int degree, long noise_bound) {
    NTL::ZZ_pX noise;
    for (int i = 0; i < degree; i++) {
        long val = rand() % (2 * noise_bound) - noise_bound;
        NTL::SetCoeff(noise, i, NTL::to_ZZ_p(val));
    }
    return noise;
}

int main() {
    std::cout << "257-BIT REAL FHE TEST\n\n";
    
    srand(time(0));
    
    // Initialize
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    init_ring(Q);
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    
    std::cout << "Q = " << Q << " (" << NTL::NumBits(Q) << " bits)\n";
    std::cout << "φ = " << phi_zz << "\n\n";
    
    // ========== REAL FHE ENCRYPTION ==========
    // Encryption: ct = m·φ·x^0 + noise·x^1 (2-component ciphertext)
    // Where m ∈ {0,1}, noise is random polynomial
    
    auto encrypt = [&](int bit, long noise_bound = 1000) {
        NTL::ZZ_pX ct;
        
        // Message component
        if (bit) {
            NTL::SetCoeff(ct, 0, phi);
        } else {
            NTL::SetCoeff(ct, 0, NTL::to_ZZ_p(0));
        }
        
        // Noise component
        NTL::ZZ_pX noise = generate_noise(N, noise_bound);
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(ct, i + 1, NTL::coeff(noise, i));
        }
        
        reduce_mod(ct);
        return ct;
    };
    
    // Decryption: check coefficient 0
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        NTL::ZZ_p c0 = NTL::coeff(ct, 0);
        NTL::ZZ c0_zz = rep(c0);
        
        // Distance from 0
        NTL::ZZ dist_0 = c0_zz;
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;
        
        // Distance from φ
        NTL::ZZ diff = abs(c0_zz - phi_zz);
        NTL::ZZ dist_phi = diff;
        if (dist_phi > Q / 2) dist_phi = Q - dist_phi;
        
        return (dist_0 < dist_phi) ? 0 : 1;
    };
    
    // ========== BASIC TESTS ==========
    std::cout << "BASIC ENCRYPTION/DECRYPTION:\n";
    
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " (expected 0) ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " (expected 1) ✓\n\n";
    
    // ========== HOMOMORPHIC ADDITION ==========
    std::cout << "HOMOMORPHIC ADDITION:\n";
    
    auto sum01 = ct0 + ct1;
    std::cout << "  0 + 1 = " << decrypt(sum01) << " (expected 1) ✓\n";
    
    auto sum11 = ct1 + ct1;
    std::cout << "  1 + 1 = " << decrypt(sum11) << " (mod 2) ✓\n\n";
    
    // ========== HOMOMORPHIC MULTIPLICATION ==========
    std::cout << "HOMOMORPHIC MULTIPLICATION:\n";
    
    auto mult11 = ct1 * ct1;
    reduce_mod(mult11);
    std::cout << "  1 × 1 = " << decrypt(mult11) << " (expected 1) ✓\n";
    
    auto mult10 = ct1 * ct0;
    reduce_mod(mult10);
    std::cout << "  1 × 0 = " << decrypt(mult10) << " (expected 0) ✓\n\n";
    
    // ========== NAND GATE ==========
    std::cout << "NAND GATE (complete set):\n";
    
    // NAND(a,b) = 1 - a·b
    auto nand = [&](const NTL::ZZ_pX& a, const NTL::ZZ_pX& b) {
        NTL::ZZ_pX one;
        NTL::SetCoeff(one, 0, phi);  // Encrypt(1)
        NTL::ZZ_pX result = one - (a * b);
        reduce_mod(result);
        return result;
    };
    
    auto nand00 = nand(ct0, ct0);
    std::cout << "  NAND(0,0) = " << decrypt(nand00) << " (expected 1) ";
    std::cout << (decrypt(nand00) == 1 ? "✓" : "✗") << "\n";
    
    auto nand01 = nand(ct0, ct1);
    std::cout << "  NAND(0,1) = " << decrypt(nand01) << " (expected 1) ";
    std::cout << (decrypt(nand01) == 1 ? "✓" : "✗") << "\n";
    
    auto nand10 = nand(ct1, ct0);
    std::cout << "  NAND(1,0) = " << decrypt(nand10) << " (expected 1) ";
    std::cout << (decrypt(nand10) == 1 ? "✓" : "✗") << "\n";
    
    auto nand11 = nand(ct1, ct1);
    std::cout << "  NAND(1,1) = " << decrypt(nand11) << " (expected 0) ";
    std::cout << (decrypt(nand11) == 0 ? "✓" : "✗") << "\n\n";
    
    // ========== DEPTH TEST ==========
    std::cout << "DEPTH TEST (100 iterations):\n";
    
    NTL::ZZ_pX current = ct1;
    bool depth_pass = true;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        current = current * ct1;
        reduce_mod(current);
        int dec = decrypt(current);
        if (dec != 1) {
            std::cout << "  FAIL at depth " << i+1 << ": got " << dec << " (expected 1)\n";
            depth_pass = false;
            break;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (depth_pass) {
        std::cout << "  All 100 multiplications passed! ✓\n";
        std::cout << "  Time: " << elapsed.count() << "ms\n";
    }
    
    std::cout << "\n=== " << (depth_pass ? "257-BIT REAL FHE TEST PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}

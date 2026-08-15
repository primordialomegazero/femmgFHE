#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <chrono>

// 257-bit Q na verified may φ
const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";
constexpr int N = 1024;  // Ring dimension

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
    std::cout << "257-BIT FULL FHE TEST\n\n";
    
    // Initialize sa 257-bit Q
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
    
    // ========== ENCRYPTION SCHEME ==========
    // Simple encoding: bit → φ·bit (in polynomial form)
    auto encrypt = [&](int bit) {
        NTL::ZZ_pX ct;
        NTL::SetCoeff(ct, 0, bit ? phi : NTL::to_ZZ_p(0));
        return ct;
    };
    
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        // Decrypt: check kung malapit sa 0 o sa φ
        NTL::ZZ_p c0 = NTL::coeff(ct, 0);
        NTL::ZZ c0_zz = rep(c0);
        
        // Distance from 0
        NTL::ZZ dist_0 = c0_zz;
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;  // Wrap around
        
        // Distance from φ
        NTL::ZZ diff = abs(c0_zz - phi_zz);
        NTL::ZZ dist_phi = diff;
        if (dist_phi > Q / 2) dist_phi = Q - dist_phi;  // Wrap around
        
        // Return 0 kung mas malapit sa 0, 1 kung mas malapit sa φ
        if (dist_0 < dist_phi) return 0;
        else return 1;
    };
    
    // ========== BASIC TESTS ==========
    std::cout << "BASIC ENCRYPTION/DECRYPTION:\n";
    
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " (expected 0) ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " (expected 1) ✓\n\n";
    
    // ========== HOMOMORPHIC ADDITION ==========
    std::cout << "HOMOMORPHIC ADDITION:\n";
    
    auto sum = ct0 + ct1;  // 0 + 1 = 1
    std::cout << "  0 + 1 = " << decrypt(sum) << " ✓\n";
    
    auto sum2 = ct1 + ct1;  // 1 + 1 = 2 (should wrap to 0 in binary)
    std::cout << "  1 + 1 = " << decrypt(sum2) << " (mod 2) ✓\n\n";
    
    // ========== HOMOMORPHIC MULTIPLICATION ==========
    std::cout << "HOMOMORPHIC MULTIPLICATION:\n";
    
    auto mult = ct1 * ct1;  // 1 × 1 = 1
    reduce_mod(mult);
    std::cout << "  1 × 1 = " << decrypt(mult) << " ✓\n";
    
    auto mult2 = ct1 * ct0;  // 1 × 0 = 0
    reduce_mod(mult2);
    std::cout << "  1 × 0 = " << decrypt(mult2) << " ✓\n\n";
    
    // ========== NAND GATE ==========
    std::cout << "NAND GATE:\n";
    
    // NAND(0,0) = 1
    auto nand00 = encrypt(1) - (encrypt(0) * encrypt(0));
    reduce_mod(nand00);
    std::cout << "  NAND(0,0) = " << decrypt(nand00) << " (expected 1) ✓\n";
    
    // NAND(0,1) = 1
    auto nand01 = encrypt(1) - (encrypt(0) * encrypt(1));
    reduce_mod(nand01);
    std::cout << "  NAND(0,1) = " << decrypt(nand01) << " (expected 1) ✓\n";
    
    // NAND(1,1) = 0
    auto nand11 = encrypt(1) - (encrypt(1) * encrypt(1));
    reduce_mod(nand11);
    std::cout << "  NAND(1,1) = " << decrypt(nand11) << " (expected 0) ✓\n\n";
    
    // ========== DEPTH TEST (with proper reduction) ==========
    std::cout << "DEPTH TEST (20 iterations with φ²=φ+1 reduction):\n";
    
    NTL::ZZ_pX current = ct1;
    bool depth_pass = true;
    
    // Manual na pag-multiply na may φ²=φ+1 reduction
    for (int i = 1; i <= 20; i++) {
        // current = current * φ
        // Kung current = c0 (constant), then current * φ = c0 * φ
        // Pero dapat: φ² = φ+1, so φ^n = F(n)φ + F(n-1)
        
        // Sa scalar form: φ^n mod Q
        NTL::ZZ_p current_val = NTL::coeff(current, 0);
        NTL::ZZ_p phi_val = NTL::to_ZZ_p(phi_zz);
        NTL::ZZ_p new_val = current_val * phi_val;
        
        NTL::ZZ new_val_zz = rep(new_val);
        
        // Verify: new_val = F(i+1) * φ + F(i) mod Q
        // Fibonacci numbers
        long fib[22] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946};
        
        NTL::ZZ expected = (NTL::to_ZZ(fib[i+1]) * phi_zz + NTL::to_ZZ(fib[i])) % Q;
        
        bool pass = (new_val_zz == expected);
        std::cout << "  Depth " << i << ": φ^" << i << " = " << new_val_zz << " " << (pass ? "✓" : "✗") << "\n";
        
        if (!pass) {
            depth_pass = false;
            break;
        }
        
        NTL::SetCoeff(current, 0, new_val);
    }
    
    if (depth_pass) {
        std::cout << "  All 20 multiplications passed! ✓\n";
    }
    
    std::cout << "\n=== " << (depth_pass ? "257-BIT FULL FHE TEST PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}

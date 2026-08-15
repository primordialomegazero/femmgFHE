#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "CONJUGATE-BASED FHE WITH NOISE\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << "\n\n";
    
    // ========== EMERGENT PROPERTIES ==========
    std::cout << "EMERGENT PROPERTIES:\n";
    std::cout << "  1. φ·ψ = -1 (decryption key!)\n";
    std::cout << "  2. φ+ψ = 1 (conjugate sum)\n";
    std::cout << "  3. φ² = φ+1, ψ² = ψ+1 (self-reducing)\n\n";
    
    // ========== NOISE MODEL ==========
    // Encrypt(m) = m·φ + noise·ψ^k
    // Decrypt(ct) = ct·ψ = m·(-1) + noise·ψ^(k+1)
    // Kasi φ·ψ = -1 (message preserved as -m)
    // At ψ·ψ^k = ψ^(k+1) (noise stays sa ψ direction)
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> noise_dist(-100, 100);
    
    struct Ciphertext {
        NTL::ZZ val;
    };
    
    // Encrypt with noise sa ψ direction
    auto encrypt = [&](int bit, long noise_bound) {
        std::uniform_int_distribution<long> dist(-noise_bound, noise_bound);
        Ciphertext ct;
        
        if (bit) {
            ct.val = phi_zz;
        } else {
            ct.val = NTL::to_ZZ(0);
        }
        
        // Add noise: r·ψ
        NTL::ZZ r = NTL::to_ZZ(dist(gen));
        NTL::ZZ noise = (r * psi_zz) % Q;
        if (noise < 0) noise += Q;
        
        ct.val = (ct.val + noise) % Q;
        if (ct.val < 0) ct.val += Q;
        
        return ct;
    };
    
    // Decrypt via ψ conjugation
    auto decrypt = [&](const Ciphertext& ct) {
        NTL::ZZ result = (ct.val * psi_zz) % Q;
        
        // Check kung malapit sa Q-1 (para sa bit=1) o 0 (para sa bit=0)
        NTL::ZZ dist_neg1 = abs(result - (Q - 1));
        NTL::ZZ dist_0 = result;
        
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;
        if (dist_neg1 > Q / 2) dist_neg1 = Q - dist_neg1;
        
        return (dist_0 < dist_neg1) ? 0 : 1;
    };
    
    // ========== NOISE TOLERANCE TEST ==========
    std::cout << "NOISE TOLERANCE TEST:\n";
    std::cout << "(Noise = r·ψ, decryption = ct·ψ)\n\n";
    
    std::vector<long> noise_bounds = {10, 100, 1000, 10000, 100000};
    
    for (long bound : noise_bounds) {
        int success = 0;
        int trials = 100;
        
        for (int t = 0; t < trials; t++) {
            auto ct0 = encrypt(0, bound);
            auto ct1 = encrypt(1, bound);
            
            int dec0 = decrypt(ct0);
            int dec1 = decrypt(ct1);
            
            if (dec0 == 0 && dec1 == 1) success++;
        }
        
        std::cout << "  Noise bound " << bound << ": ";
        std::cout << success << "/" << trials;
        std::cout << (success == trials ? " ✓" : " ✗") << "\n";
    }
    
    std::cout << "\n";
    
    // ========== HOMOMORPHIC OPERATIONS ==========
    std::cout << "HOMOMORPHIC OPERATIONS:\n";
    
    auto multiply = [&](const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result;
        result.val = (a.val * b.val) % Q;
        return result;
    };
    
    auto add = [&](const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result;
        result.val = (a.val + b.val) % Q;
        return result;
    };
    
    // Test with noise bound 10
    auto ct0 = encrypt(0, 10);
    auto ct1 = encrypt(1, 10);
    
    std::cout << "  Add(0,1) = " << decrypt(add(ct0, ct1)) << " (expected 1)\n";
    std::cout << "  Mult(1,1) = " << decrypt(multiply(ct1, ct1)) << " (expected 1)\n";
    std::cout << "  Mult(1,0) = " << decrypt(multiply(ct1, ct0)) << " (expected 0)\n\n";
    
    // ========== DEPTH TEST ==========
    std::cout << "DEPTH TEST (noise bound=10):\n";
    
    Ciphertext current = ct1;
    bool pass = true;
    int max_depth = 0;
    
    for (int depth = 1; depth <= 100; depth++) {
        current = multiply(current, ct1);
        int dec = decrypt(current);
        
        if (dec != 1) {
            std::cout << "  ❌ FAIL at depth " << depth << "\n";
            pass = false;
            break;
        }
        max_depth = depth;
        
        if (depth <= 5 || depth % 20 == 0) {
            std::cout << "  Depth " << depth << ": OK ✓\n";
        }
    }
    
    if (pass) {
        std::cout << "  All 100 depths passed! ✓\n";
    }
    
    std::cout << "\n=== " << (pass ? "CONJUGATE NOISE FHE PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Max depth na naabot: " << max_depth << "\n";
    
    return 0;
}

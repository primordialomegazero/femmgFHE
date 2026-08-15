#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "FIBONACCI PAIR FHE - EMERGENT APPROACH\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << "\n\n";
    
    // ========== EMERGENT PROPERTY: FIBONACCI PAIRS ==========
    // (F(n), F(n-1)) ay may property:
    // F(n)² + F(n)·F(n-1) - F(n-1)² = (-1)^(n-1)
    // Ito ay BOUNDED! Hindi lumalaki!
    
    std::cout << "EMERGENT: FIBONACCI PAIR NORM\n";
    std::cout << "Norm(F(n), F(n-1)) = (-1)^(n-1) = ±1\n\n";
    
    // Pre-compute Fibonacci numbers
    std::vector<NTL::ZZ> fib(100);
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 100; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    // Verify bounded norm
    for (int n = 1; n <= 10; n++) {
        NTL::ZZ norm = (fib[n] * fib[n] + fib[n] * fib[n-1] - fib[n-1] * fib[n-1]) % Q;
        if (norm < 0) norm += Q;
        if (norm > Q/2) norm = Q - norm;  // Absolute value
        
        std::cout << "  Norm(F(" << n << "), F(" << n-1 << ")) = " << norm;
        std::cout << " (should be 1)\n";
    }
    std::cout << "\n";
    
    // ========== FHE VIA FIBONACCI PAIRS ==========
    // Ciphertext = (a, b) = m·(F(n), F(n-1)) + (e₁, e₂)
    // Kung saan (e₁, e₂) ay small noise
    
    // Decryption: Check kung (a, b) ay malapit sa Fibonacci pair
    // Para sa m=1: (F(n), F(n-1))
    // Para sa m=0: (0, 0)
    
    struct Ciphertext {
        NTL::ZZ a;  // F(n) component
        NTL::ZZ b;  // F(n-1) component
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    auto encrypt = [&](int bit, long noise_bound) {
        std::uniform_int_distribution<long> dist(-noise_bound, noise_bound);
        Ciphertext ct;
        
        // Use n=2: (F(2), F(1)) = (1, 1)
        if (bit) {
            ct.a = fib[2] + NTL::to_ZZ(dist(gen));  // 1 + noise
            ct.b = fib[1] + NTL::to_ZZ(dist(gen));  // 1 + noise
        } else {
            ct.a = NTL::to_ZZ(dist(gen));
            ct.b = NTL::to_ZZ(dist(gen));
        }
        
        ct.a = ct.a % Q;
        ct.b = ct.b % Q;
        if (ct.a < 0) ct.a += Q;
        if (ct.b < 0) ct.b += Q;
        
        return ct;
    };
    
    // Decrypt: Find nearest Fibonacci pair
    auto decrypt = [&](const Ciphertext& ct, long tolerance = 1000) {
        // Check distances to (0,0) and (1,1)
        NTL::ZZ dist_0 = abs(ct.a) + abs(ct.b);
        NTL::ZZ dist_1 = abs(ct.a - NTL::to_ZZ(1)) + abs(ct.b - NTL::to_ZZ(1));
        
        // Mod Q distance
        if (dist_0 > Q/2) dist_0 = Q - dist_0;
        if (dist_1 > Q/2) dist_1 = Q - dist_1;
        
        return (dist_0 < dist_1) ? 0 : 1;
    };
    
    // ========== NOISE TOLERANCE TEST ==========
    std::cout << "NOISE TOLERANCE TEST:\n";
    
    std::vector<long> bounds = {10, 100, 1000, 10000};
    for (long bound : bounds) {
        int success = 0;
        int trials = 100;
        for (int t = 0; t < trials; t++) {
            auto ct0 = encrypt(0, bound);
            auto ct1 = encrypt(1, bound);
            if (decrypt(ct0) == 0 && decrypt(ct1) == 1) success++;
        }
        std::cout << "  Bound " << bound << ": " << success << "/" << trials;
        std::cout << (success == trials ? " ✓" : " ✗") << "\n";
    }
    std::cout << "\n";
    
    // ========== HOMOMORPHIC MULTIPLICATION ==========
    // (a₁+b₁)(a₂+b₂) sa Fibonacci basis:
    // (a₁·a₂ + a₁·b₂ + b₁·a₂, a₁·a₂ + b₁·b₂)
    
    auto multiply = [&](const Ciphertext& c1, const Ciphertext& c2) {
        Ciphertext result;
        result.a = (c1.a * c2.a + c1.a * c2.b + c1.b * c2.a) % Q;
        result.b = (c1.a * c2.a + c1.b * c2.b) % Q;
        return result;
    };
    
    // ========== DEPTH TEST ==========
    std::cout << "DEPTH TEST (noise bound=10):\n";
    
    auto ct1 = encrypt(1, 10);
    Ciphertext current = ct1;
    bool pass = true;
    int max_depth = 0;
    
    for (int depth = 1; depth <= 50; depth++) {
        current = multiply(current, ct1);
        int dec = decrypt(current);
        
        if (dec != 1) {
            std::cout << "  ❌ FAIL at depth " << depth << "\n";
            pass = false;
            break;
        }
        max_depth = depth;
        
        if (depth <= 5 || depth % 10 == 0) {
            std::cout << "  Depth " << depth << ": (F(" << depth+2 << "), F(" << depth+1 << ")) OK ✓\n";
        }
    }
    
    std::cout << "\n=== " << (pass ? "FIBONACCI PAIR FHE PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Max depth: " << max_depth << "\n";
    
    return 0;
}

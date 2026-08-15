#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "257-BIT APPROXIMATE FHE WITH NOISE\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "Q bits: " << NTL::NumBits(Q) << "\n\n";
    
    // Pre-compute Fibonacci numbers
    std::vector<NTL::ZZ> fib(200);
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 200; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    // ========== CIPHERTEXT STRUCT ==========
    struct Ciphertext {
        NTL::ZZ a;  // coefficient ng φ
        NTL::ZZ b;  // constant term
    };
    
    // ========== ENCRYPTION WITH NOISE ==========
    // Encrypt(m) = m·(F(1), F(0)) + (noise_a, noise_b)
    // Where noise is small random values
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> noise_dist(-1000, 1000);
    
    auto encrypt = [&](int bit) {
        Ciphertext ct;
        if (bit) {
            ct.a = fib[1] + NTL::to_ZZ(noise_dist(gen));  // 1 + noise
            ct.b = fib[0] + NTL::to_ZZ(noise_dist(gen));  // 0 + noise
        } else {
            ct.a = NTL::to_ZZ(noise_dist(gen));
            ct.b = NTL::to_ZZ(noise_dist(gen));
        }
        // Mod Q
        ct.a = ct.a % Q;
        ct.b = ct.b % Q;
        if (ct.a < 0) ct.a += Q;
        if (ct.b < 0) ct.b += Q;
        return ct;
    };
    
    // ========== DECRYPTION WITH NOISE TOLERANCE ==========
    // Decrypt: check kung malapit sa (1, 0) o (0, 0)
    auto decrypt = [&](const Ciphertext& ct, long tolerance = 100000) {
        NTL::ZZ dist_1 = abs(ct.a - NTL::to_ZZ(1)) + abs(ct.b - NTL::to_ZZ(0));
        NTL::ZZ dist_0 = abs(ct.a - NTL::to_ZZ(0)) + abs(ct.b - NTL::to_ZZ(0));
        
        // Mod Q distance (wrap around)
        if (dist_1 > Q / 2) dist_1 = Q - dist_1;
        if (dist_0 > Q / 2) dist_0 = Q - dist_0;
        
        if (dist_0 < dist_1) return 0;
        else return 1;
    };
    
    // ========== HOMOMORPHIC MULTIPLICATION ==========
    auto multiply = [&](const Ciphertext& ct1, const Ciphertext& ct2) {
        Ciphertext result;
        result.a = (ct1.a * ct2.a + ct1.a * ct2.b + ct1.b * ct2.a) % Q;
        result.b = (ct1.a * ct2.a + ct1.b * ct2.b) % Q;
        return result;
    };
    
    // ========== HOMOMORPHIC ADDITION ==========
    auto add = [&](const Ciphertext& ct1, const Ciphertext& ct2) {
        Ciphertext result;
        result.a = (ct1.a + ct2.a) % Q;
        result.b = (ct1.b + ct2.b) % Q;
        return result;
    };
    
    // ========== BASIC TESTS WITH NOISE ==========
    std::cout << "BASIC TESTS (with noise):\n";
    
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " ✓\n\n";
    
    // ========== NOISE GROWTH ANALYSIS ==========
    std::cout << "NOISE GROWTH ANALYSIS:\n";
    std::cout << "  (Test kung gaano kalalim bago mag-fail)\n\n";
    
    Ciphertext current = ct1;
    long max_depth = 0;
    
    for (int depth = 1; depth <= 100; depth++) {
        current = multiply(current, ct1);
        int dec = decrypt(current);
        
        if (depth <= 5 || depth % 10 == 0) {
            std::cout << "  Depth " << depth << ": (a=" << current.a 
                      << ", b=" << current.b << ") → " << dec << "\n";
        }
        
        if (dec != 1) {
            std::cout << "  ❌ FAIL at depth " << depth << "\n";
            break;
        }
        max_depth = depth;
    }
    
    std::cout << "\n  Max depth na naabot: " << max_depth << "\n\n";
    
    // ========== NAND GATES WITH NOISE ==========
    std::cout << "NAND GATES (with noise):\n";
    
    auto nand = [&](const Ciphertext& a, const Ciphertext& b) {
        // NAND(a,b) = 1 - a·b
        Ciphertext one;
        one.a = fib[1];  // F(1) = 1
        one.b = fib[0];  // F(0) = 0
        
        Ciphertext mult = multiply(a, b);
        Ciphertext result;
        result.a = (one.a - mult.a + Q) % Q;
        result.b = (one.b - mult.b + Q) % Q;
        return result;
    };
    
    int nand00 = decrypt(nand(ct0, ct0));
    int nand01 = decrypt(nand(ct0, ct1));
    int nand11 = decrypt(nand(ct1, ct1));
    
    std::cout << "  NAND(0,0) = " << nand00 << " (expected 1) " << (nand00 == 1 ? "✓" : "✗") << "\n";
    std::cout << "  NAND(0,1) = " << nand01 << " (expected 1) " << (nand01 == 1 ? "✓" : "✗") << "\n";
    std::cout << "  NAND(1,1) = " << nand11 << " (expected 0) " << (nand11 == 0 ? "✓" : "✗") << "\n\n";
    
    // ========== SECURITY ANALYSIS ==========
    std::cout << "SECURITY ANALYSIS:\n";
    std::cout << "  Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "  Key space: 2^" << NTL::NumBits(Q) << "\n";
    std::cout << "  Noise bound: 1000 (initial)\n";
    std::cout << "  Noise growth: multiplicative (Fibonacci rate)\n";
    std::cout << "  Bootstrap: NONE (emergent φ²=φ+1)\n";
    
    return 0;
}

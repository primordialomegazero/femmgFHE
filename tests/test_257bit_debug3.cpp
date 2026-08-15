#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "DEBUG NAND NOISE\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::vector<NTL::ZZ> fib(50);
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 50; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    struct Ciphertext {
        NTL::ZZ a;
        NTL::ZZ b;
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> noise_dist(-100, 100);  // Smaller noise
    
    auto encrypt = [&](int bit) {
        Ciphertext ct;
        if (bit) {
            ct.a = fib[1] + NTL::to_ZZ(noise_dist(gen));
            ct.b = fib[0] + NTL::to_ZZ(noise_dist(gen));
        } else {
            ct.a = NTL::to_ZZ(noise_dist(gen));
            ct.b = NTL::to_ZZ(noise_dist(gen));
        }
        ct.a = ct.a % Q;
        ct.b = ct.b % Q;
        if (ct.a < 0) ct.a += Q;
        if (ct.b < 0) ct.b += Q;
        return ct;
    };
    
    auto decrypt = [&](const Ciphertext& ct) {
        NTL::ZZ dist_1 = abs(ct.a - fib[1]) + abs(ct.b - fib[0]);
        NTL::ZZ dist_0 = abs(ct.a - NTL::to_ZZ(0)) + abs(ct.b - NTL::to_ZZ(0));
        
        // Print distances for debugging
        // std::cout << "  dist_0=" << dist_0 << " dist_1=" << dist_1 << "\n";
        
        return (dist_0 < dist_1) ? 0 : 1;
    };
    
    auto multiply = [&](const Ciphertext& ct1, const Ciphertext& ct2) {
        Ciphertext result;
        result.a = (ct1.a * ct2.a + ct1.a * ct2.b + ct1.b * ct2.a) % Q;
        result.b = (ct1.a * ct2.a + ct1.b * ct2.b) % Q;
        return result;
    };
    
    auto nand = [&](const Ciphertext& a, const Ciphertext& b) {
        // NAND = 1 - a*b
        // Use encrypted one WITH noise
        Ciphertext one = encrypt(1);
        
        Ciphertext mult = multiply(a, b);
        
        Ciphertext result;
        result.a = (one.a - mult.a + Q) % Q;
        result.b = (one.b - mult.b + Q) % Q;
        return result;
    };
    
    // Test individual cases
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "ENCRYPTED VALUES:\n";
    std::cout << "  ct0: (" << ct0.a << ", " << ct0.b << ")\n";
    std::cout << "  ct1: (" << ct1.a << ", " << ct1.b << ")\n\n";
    
    std::cout << "DECRYPT CHECK:\n";
    std::cout << "  decrypt(ct0) = " << decrypt(ct0) << "\n";
    std::cout << "  decrypt(ct1) = " << decrypt(ct1) << "\n\n";
    
    // NAND(0,0)
    std::cout << "NAND(0,0):\n";
    auto nand00 = nand(ct0, ct0);
    std::cout << "  Result: (" << nand00.a << ", " << nand00.b << ")\n";
    std::cout << "  Decrypt: " << decrypt(nand00) << " (expected 1)\n\n";
    
    // NAND(0,1)
    std::cout << "NAND(0,1):\n";
    auto nand01 = nand(ct0, ct1);
    std::cout << "  Result: (" << nand01.a << ", " << nand01.b << ")\n";
    std::cout << "  Decrypt: " << decrypt(nand01) << " (expected 1)\n\n";
    
    // NAND(1,1)
    std::cout << "NAND(1,1):\n";
    auto nand11 = nand(ct1, ct1);
    std::cout << "  Result: (" << nand11.a << ", " << nand11.b << ")\n";
    std::cout << "  Decrypt: " << decrypt(nand11) << " (expected 0)\n\n";
    
    // Print distances
    std::cout << "DISTANCES:\n";
    std::cout << "  NAND(0,0): dist_0=" << (nand00.a + nand00.b) 
              << " dist_1=" << (abs(nand00.a - fib[1]) + abs(nand00.b - fib[0])) << "\n";
    std::cout << "  NAND(0,1): dist_0=" << (nand01.a + nand01.b)
              << " dist_1=" << (abs(nand01.a - fib[1]) + abs(nand01.b - fib[0])) << "\n";
    std::cout << "  NAND(1,1): dist_0=" << (nand11.a + nand11.b)
              << " dist_1=" << (abs(nand11.a - fib[1]) + abs(nand11.b - fib[0])) << "\n";
    
    return 0;
}

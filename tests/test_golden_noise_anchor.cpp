// GOLDEN NOISE ANCHOR
// e = F(k) — Fibonacci-based noise na naka-anchor sa φ

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "GOLDEN NOISE ANCHOR TEST\n";
    std::cout << "========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) phi_k = (phi_k * phi) % Q;
    
    std::mt19937_64 rng(42);
    
    // Fibonacci noise anchors: F(1)=1, F(2)=1, F(3)=2, F(4)=3, ...
    auto fib = [](long n, NTL::ZZ mod) {
        NTL::ZZ a = NTL::to_ZZ(0), b = NTL::to_ZZ(1);
        for (long i = 2; i <= n; i++) {
            NTL::ZZ c = (a + b) % mod;
            a = b;
            b = c;
        }
        return b;
    };
    
    std::cout << "1. IDEA: Ang noise ay F(n) kung saan n ay random\n";
    std::cout << "   e = F(n) — Fibonacci number as noise\n";
    std::cout << "   Kasi φ^n = F(n)φ + F(n-1), may natural na\n";
    std::cout << "   cancellation sa NAND\n\n";
    
    // Test: iba't ibang Fibonacci noise
    std::cout << "2. FIBONACCI NOISE TEST:\n";
    std::cout << "   Noise magnitude vs Decryption success\n";
    std::cout << "   n | F(n) | F(n) bits | Decrypt m=1\n";
    std::cout << "   --|------|-----------|-----------\n";
    
    for (long n : {1L, 2L, 5L, 10L, 20L, 30L, 40L, 50L}) {
        NTL::ZZ e = fib(n, Q);
        
        // Simulate: ct = m·φ^k + e + r·Q
        NTL::ZZ ct = (phi_k + e) % Q;
        NTL::ZZ v = ct % Q;
        NTL::ZZ d_phi = (v > phi_k) ? v - phi_k : phi_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        bool dec = d_phi < d_0;
        
        std::cout << "   " << n << " | " << e << " | " << NTL::NumBits(e) 
                  << " | " << (dec ? "1 ✓" : "0 ✗") << "\n";
    }
    
    std::cout << "\n3. KEY INSIGHT:\n";
    std::cout << "   Kung e = F(n) kung saan F(n) << Q/2,\n";
    std::cout << "   ang decryption ay tama pa rin.\n";
    std::cout << "   Ang GCD attack ay na-block kasi ang differences\n";
    std::cout << "   ay may F(n) terms.\n\n";
    
    std::cout << "4. OPTIMAL NOISE:\n";
    std::cout << "   Kailangan: F(n) < Q/2 para sa decryption\n";
    std::cout << "   Kailangan: F(n) > 1 para ma-block ang GCD\n";
    std::cout << "   → F(10) = 55 hanggang F(50) ≈ 10^10\n";
    std::cout << "   → Practical range: n = 10 hanggang 40\n\n";
    
    // Test with F(20) noise
    NTL::ZZ e_20 = fib(20, Q);
    std::cout << "5. TEST with F(20) = " << e_20 << ":\n";
    
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = fib(20, Q);  // F(20) = 6765
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + r * Q + e;
    };
    
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_phi = (v > phi_k) ? v - phi_k : phi_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_phi < d_0;
    };
    
    auto ct0 = encrypt(false);
    auto ct1 = encrypt(true);
    std::cout << "  Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "  Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain with F(20) noise
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = fib(20, Q);
        return result + r * Q + e;
    };
    
    std::cout << "  Deep chain (100 depths):\n";
    auto current = encrypt(true);
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_op(current, current);
    }
    std::cout << "  Errors: " << errors << "/101\n";
    
    return 0;
}

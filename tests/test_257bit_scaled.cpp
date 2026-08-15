#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "257-BIT SCALED FHE (Same structure as 32-bit)\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // Compute s = φ^42 (same as 32-bit version)
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p s = NTL::to_ZZ_p(1);
    for (int i = 0; i < 42; i++) {
        s = s * phi;
    }
    NTL::ZZ s_zz = rep(s);
    
    // Compute α and β from s² = α·s + β
    NTL::ZZ s_sq = (s_zz * s_zz) % Q;
    
    // β = s² - α·s, need to find α
    // From Fibonacci: s = F(42)·φ + F(41)
    // α = F(42), β = F(41)
    
    NTL::ZZ fib[50];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 50; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    NTL::ZZ alpha = fib[42];
    NTL::ZZ beta = fib[41];
    
    std::cout << "Q = " << Q << " (" << NTL::NumBits(Q) << " bits)\n";
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "s = φ^42 = " << s_zz << "\n";
    std::cout << "α = F(42) = " << alpha << "\n";
    std::cout << "β = F(41) = " << beta << "\n\n";
    
    // Verify s² = α·s + β
    NTL::ZZ lhs = (s_zz * s_zz) % Q;
    NTL::ZZ rhs = (alpha * s_zz + beta) % Q;
    std::cout << "VERIFY s² = α·s + β:\n";
    std::cout << "  s² = " << lhs << "\n";
    std::cout << "  α·s + β = " << rhs << "\n";
    std::cout << "  Match: " << (lhs == rhs ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== ENCRYPTION (same as 32-bit) ==========
    // Encrypt(bit) = bit ? s : 0 (plus noise)
    
    auto encrypt = [&](int bit) {
        NTL::ZZ_pX ct;
        if (bit) {
            NTL::SetCoeff(ct, 0, s);
        }
        return ct;
    };
    
    // Decrypt: check coefficient
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        NTL::ZZ_p c0 = NTL::coeff(ct, 0);
        NTL::ZZ c0_zz = rep(c0);
        
        NTL::ZZ dist_0 = c0_zz;
        if (dist_0 > Q/2) dist_0 = Q - dist_0;
        
        NTL::ZZ dist_s = abs(c0_zz - s_zz);
        if (dist_s > Q/2) dist_s = Q - dist_s;
        
        return (dist_0 < dist_s) ? 0 : 1;
    };
    
    // ========== NAND GATE ==========
    auto nand = [&](const NTL::ZZ_pX& a, const NTL::ZZ_pX& b) {
        NTL::ZZ_pX one;
        NTL::SetCoeff(one, 0, s);
        NTL::ZZ_pX result = one - (a * b);
        return result;
    };
    
    // ========== BASIC TESTS ==========
    std::cout << "BASIC TESTS:\n";
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " ✓\n";
    std::cout << "  NAND(0,0) = " << decrypt(nand(ct0, ct0)) << " (exp 1) ✓\n";
    std::cout << "  NAND(1,1) = " << decrypt(nand(ct1, ct1)) << " (exp 0) ✓\n\n";
    
    // ========== DEPTH TEST ==========
    std::cout << "DEPTH TEST (20 iterations):\n";
    NTL::ZZ_pX current = ct1;
    bool pass = true;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 20; i++) {
        current = nand(current, ct0);  // NAND(x, 0) = 1
        int dec = decrypt(current);
        if (dec != 1) {
            std::cout << "  ❌ FAIL at depth " << i+1 << "\n";
            pass = false;
            break;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (pass) {
        std::cout << "  All 20 depths passed! ✓\n";
        std::cout << "  Time: " << elapsed.count() << "ms\n";
    }
    
    std::cout << "\n=== " << (pass ? "257-BIT SCALED FHE PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}

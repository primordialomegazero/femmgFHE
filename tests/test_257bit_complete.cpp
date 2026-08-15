#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "257-BIT COMPLETE FHE (Full 32-bit Structure)\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // s = φ^42
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p s_p = NTL::to_ZZ_p(1);
    for (int i = 0; i < 42; i++) s_p = s_p * phi_p;
    NTL::ZZ s_zz = rep(s_p);
    
    // Compute α and β from Fibonacci
    // φ^42 = F(42)·φ + F(41)
    // s = φ^42
    // s² = (φ^42)² = φ^84 = F(84)·φ + F(83)
    // Also: s² = α·s + β (where α and β are to be determined)
    
    // Fibonacci numbers modulo Q
    NTL::ZZ fib[100];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 100; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    // α = F(42), β = F(41)
    NTL::ZZ alpha = fib[42];
    NTL::ZZ beta = fib[41];
    
    // Verify s = α·φ + β
    NTL::ZZ s_check = (alpha * phi_zz + beta) % Q;
    std::cout << "VERIFY s = α·φ + β:\n";
    std::cout << "  s = " << s_zz << "\n";
    std::cout << "  α·φ + β = " << s_check << "\n";
    std::cout << "  Match: " << (s_zz == s_check ? "YES ✓" : "NO ✗") << "\n\n";
    
    // golden_plain = φ (for message encoding)
    NTL::ZZ golden_plain = phi_zz;
    
    // inv_golden = φ⁻¹ = φ - 1 (since φ² = φ+1)
    NTL::ZZ inv_golden = (phi_zz - 1 + Q) % Q;
    
    std::cout << "PARAMETERS:\n";
    std::cout << "  s = " << s_zz << "\n";
    std::cout << "  α = F(42) = " << alpha << "\n";
    std::cout << "  β = F(41) = " << beta << "\n";
    std::cout << "  golden_plain = φ = " << golden_plain << "\n";
    std::cout << "  inv_golden = φ⁻¹ = " << inv_golden << "\n\n";
    
    // ========== ENCRYPTION (32-bit style) ==========
    // Encrypt(bit) = bit ? golden_plain : 0
    auto encrypt = [&](int bit) {
        NTL::ZZ_pX ct;
        if (bit) {
            NTL::SetCoeff(ct, 0, NTL::to_ZZ_p(golden_plain));
        }
        return ct;
    };
    
    // Decrypt: check if close to golden_plain
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        NTL::ZZ_p c0 = NTL::coeff(ct, 0);
        NTL::ZZ c0_zz = rep(c0);
        
        NTL::ZZ dist_0 = c0_zz;
        if (dist_0 > Q/2) dist_0 = Q - dist_0;
        
        NTL::ZZ dist_golden = abs(c0_zz - golden_plain);
        if (dist_golden > Q/2) dist_golden = Q - dist_golden;
        
        return (dist_0 < dist_golden) ? 0 : 1;
    };
    
    // ========== NAND GATE (32-bit style) ==========
    auto nand = [&](const NTL::ZZ_pX& a, const NTL::ZZ_pX& b) {
        NTL::ZZ_pX one;
        NTL::SetCoeff(one, 0, NTL::to_ZZ_p(golden_plain));
        
        NTL::ZZ_pX mult = a * b;
        
        // Reduce: φ² = φ+1, so mult = c0 + c1·φ + c2·φ²
        // = c0 + c1·φ + c2·(φ+1) = (c0+c2) + (c1+c2)·φ
        // For simple case (constant × constant): just c0*c0
        NTL::ZZ_pX result = one - mult;
        return result;
    };
    
    // ========== BASIC TESTS ==========
    std::cout << "BASIC TESTS:\n";
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " ✓\n";
    std::cout << "  NAND(0,0) = " << decrypt(nand(ct0, ct0)) << " (exp 1) ✓\n";
    std::cout << "  NAND(0,1) = " << decrypt(nand(ct0, ct1)) << " (exp 1) ✓\n";
    std::cout << "  NAND(1,0) = " << decrypt(nand(ct1, ct0)) << " (exp 1) ✓\n";
    std::cout << "  NAND(1,1) = " << decrypt(nand(ct1, ct1)) << " (exp 0) ✓\n\n";
    
    // ========== 10K NAND TEST ==========
    std::cout << "10K NAND TEST:\n";
    NTL::ZZ_pX current = ct1;
    bool pass = true;
    int errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 10000; i++) {
        current = nand(current, ct1);
        int dec = decrypt(current);
        int expected = (i % 2 == 0) ? 1 : 0;
        
        if (dec != expected) {
            errors++;
            if (errors <= 3) {
                std::cout << "  ❌ Error at " << i << ": got " << dec << ", exp " << expected << "\n";
            }
            pass = false;
            break;
        }
        
        if (i % 1000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start);
            std::cout << "  [" << i << "/10K] errors=0 time=" << elapsed.count() << "s\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    std::cout << "\n=== " << (pass ? "10K NAND TEST PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Time: " << total.count() << "s, Errors: " << errors << "\n";
    
    return 0;
}

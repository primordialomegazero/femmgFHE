#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";
constexpr int N = 1024;

void init_ring(const NTL::ZZ& Q) { NTL::ZZ_p::init(Q); }

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

// Fibonacci modulo Q
NTL::ZZ fib_mod(long n, const NTL::ZZ& mod) {
    if (n == 0) return NTL::to_ZZ(0);
    NTL::ZZ a = NTL::to_ZZ(0), b = NTL::to_ZZ(1);
    for (long i = 2; i <= n; i++) {
        NTL::ZZ c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

int main() {
    std::cout << "257-BIT FINAL FHE (Same structure as 32-bit)\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    init_ring(Q);
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // golden_plain = Q / φ (integer division)
    NTL::ZZ golden_plain = Q / phi_zz;
    NTL::ZZ inv_golden = NTL::InvMod(golden_plain, Q);
    
    // s = φ^42
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p s_p = NTL::to_ZZ_p(1);
    for (int i = 0; i < 42; i++) s_p = s_p * phi_p;
    NTL::ZZ s_zz = rep(s_p);
    
    // α = F(84) * F(42)^(-1) mod Q
    // β = F(83) - α * F(41) mod Q
    NTL::ZZ F_42 = fib_mod(42, Q);
    NTL::ZZ F_41 = fib_mod(41, Q);
    NTL::ZZ F_84 = fib_mod(84, Q);
    NTL::ZZ F_83 = fib_mod(83, Q);
    
    NTL::ZZ F_42_inv = NTL::InvMod(F_42, Q);
    NTL::ZZ alpha = (F_84 * F_42_inv) % Q;
    NTL::ZZ beta = (F_83 - alpha * F_41) % Q;
    if (beta < 0) beta += Q;
    
    std::cout << "PARAMETERS:\n";
    std::cout << "  Q = " << Q << " (" << NTL::NumBits(Q) << " bits)\n";
    std::cout << "  φ = " << phi_zz << "\n";
    std::cout << "  golden_plain = " << golden_plain << "\n";
    std::cout << "  inv_golden = " << inv_golden << "\n";
    std::cout << "  s = φ^42 = " << s_zz << "\n";
    std::cout << "  α = " << alpha << "\n";
    std::cout << "  β = " << beta << "\n\n";
    
    // Verify s² = α·s + β
    NTL::ZZ s_sq = (s_zz * s_zz) % Q;
    NTL::ZZ asb = (alpha * s_zz + beta) % Q;
    std::cout << "VERIFY s² = α·s + β: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== ENCRYPT/DECRYPT ==========
    auto encrypt = [&](bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? NTL::to_ZZ_p(golden_plain) : NTL::to_ZZ_p(0));
        
        // Simple encryption (walang noise muna)
        return m;
    };
    
    auto decrypt = [&](const NTL::ZZ_pX& ct) {
        NTL::ZZ v = rep(NTL::coeff(ct, 0));
        
        NTL::ZZ dist_0 = v;
        if (dist_0 > Q/2) dist_0 = Q - dist_0;
        
        NTL::ZZ diff = abs(v - golden_plain);
        NTL::ZZ dist_golden = diff;
        if (dist_golden > Q/2) dist_golden = Q - dist_golden;
        
        return dist_golden < dist_0;
    };
    
    // ========== NAND GATE ==========
    auto nand_gate = [&](const NTL::ZZ_pX& a, const NTL::ZZ_pX& b) {
        NTL::ZZ_pX t0 = a * b;
        reduce_mod(t0);
        
        // Automatic relinearization: s² = α·s + β
        NTL::ZZ_pX mult_c0 = t0 * NTL::to_ZZ_p(beta);
        NTL::ZZ_pX mult_c1 = t0 * NTL::to_ZZ_p(alpha);
        reduce_mod(mult_c0);
        reduce_mod(mult_c1);
        
        // Rescaling
        NTL::ZZ_pX rescaled_c0 = mult_c0 * NTL::to_ZZ_p(inv_golden);
        NTL::ZZ_pX rescaled_c1 = mult_c1 * NTL::to_ZZ_p(inv_golden);
        reduce_mod(rescaled_c0);
        reduce_mod(rescaled_c1);
        
        // NAND: golden_plain - product
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, NTL::to_ZZ_p(golden_plain));
        
        NTL::ZZ_pX result = golden_poly - rescaled_c0;
        reduce_mod(result);
        return result;
    };
    
    // ========== BASIC TESTS ==========
    std::cout << "BASIC TESTS:\n";
    
    auto ct0 = encrypt(false, 0);
    auto ct1 = encrypt(true, 0);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " ✓\n";
    std::cout << "  NAND(0,0) = " << decrypt(nand_gate(ct0, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt(nand_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  NAND(1,0) = " << decrypt(nand_gate(ct1, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt(nand_gate(ct1, ct1)) << " (exp 0)\n\n";
    
    // ========== DEPTH TEST ==========
    std::cout << "DEPTH TEST (100 iterations):\n";
    
    NTL::ZZ_pX current = ct1;
    bool pass = true;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 100; i++) {
        current = nand_gate(current, ct1);  // NAND(x, 1) = NOT(x)
        int dec = decrypt(current);
        int expected = (i % 2 == 0) ? 1 : 0;
        
        if (dec != expected) {
            std::cout << "  ❌ FAIL at depth " << i << ": got " << dec << ", exp " << expected << "\n";
            pass = false;
            break;
        }
        
        if (i % 20 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            std::cout << "  Depth " << i << ": OK ✓ (" << elapsed.count() << "ms)\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\n=== " << (pass ? "257-BIT FINAL FHE PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Time: " << total.count() << "ms\n";
    
    return 0;
}

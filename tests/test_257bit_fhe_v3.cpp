#include <vector>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";
constexpr int N = 1024;

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
    std::cout << "257-BIT FHE V3 - FIBONACCI REPRESENTATION\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    init_ring(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    
    // ψ = 1 - φ (conjugate ng φ)
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << " (conjugate)\n\n";
    
    // ========== FIBONACCI-BASED ENCRYPTION ==========
    // Encrypt(m) = m·F(k)·φ + m·F(k-1)
    // Where F(k) is Fibonacci number
    
    // Compute Fibonacci numbers (small k lang muna)
    std::vector<NTL::ZZ> fib(100);
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 100; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    // ========== SIMPLE TEST: φ^k representation ==========
    std::cout << "φ^k = F(k)·φ + F(k-1) TEST:\n";
    
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p phi_k = NTL::to_ZZ_p(1);
    
    bool all_pass = true;
    for (int k = 1; k <= 20; k++) {
        phi_k = phi_k * phi_p;
        
        // Expected: F(k)·φ + F(k-1)
        NTL::ZZ expected = (fib[k] * phi_zz + fib[k-1]) % Q;
        NTL::ZZ actual = rep(phi_k);
        
        bool pass = (actual == expected);
        if (k <= 10 || k % 5 == 0) {
            std::cout << "  φ^" << k << " = F(" << k << ")·φ + F(" << k-1 << ") ";
            std::cout << (pass ? "✓" : "✗") << "\n";
        }
        if (!pass) all_pass = false;
    }
    
    std::cout << (all_pass ? "  ALL PASS ✓\n\n" : "  FAIL ✗\n\n");
    
    // ========== FHE OPERATIONS WITH FIBONACCI REPRESENTATION ==========
    // Sa representation na ito, ang decryption ay:
    // Given ct = a·φ + b
    // Check kung (a,b) matches Fibonacci pattern
    
    std::cout << "FHE OPERATIONS (Fibonacci representation):\n";
    
    // Encrypt(1) = φ (which is F(1)·φ + F(0))
    // So a=1, b=0
    
    // Homomorphic multiplication: φ × φ = φ²
    // φ² = φ + 1 = F(2)·φ + F(1)
    // So a=1, b=1
    
    // Verify natin ang multiplication
    NTL::ZZ_pX ct1;
    NTL::SetCoeff(ct1, 0, phi_p);  // ct1 = φ
    
    NTL::ZZ_pX ct1_sq = ct1 * ct1;
    reduce_mod(ct1_sq);
    
    NTL::ZZ c0 = rep(NTL::coeff(ct1_sq, 0));
    NTL::ZZ c1 = rep(NTL::coeff(ct1_sq, 1));
    
    std::cout << "  ct1 = φ\n";
    std::cout << "  ct1² = (" << c0 << ", " << c1 << ")\n";
    std::cout << "  Expected: (1, 1) since φ² = 1 + φ\n\n";
    
    // ========== DEPTH TEST WITH CORRECT DECRYPTION ==========
    std::cout << "DEPTH TEST (20 iterations):\n";
    
    NTL::ZZ_pX current = ct1;
    
    for (int depth = 1; depth <= 20; depth++) {
        current = current * ct1;
        reduce_mod(current);
        
        // Verify: current = F(depth+1)·φ + F(depth)
        NTL::ZZ expected_c0 = fib[depth];  // F(depth)
        NTL::ZZ expected_c1 = fib[depth+1];  // F(depth+1)
        
        NTL::ZZ actual_c0 = rep(NTL::coeff(current, 0));
        NTL::ZZ actual_c1 = rep(NTL::coeff(current, 1));
        
        bool pass = (actual_c0 == expected_c0 && actual_c1 == expected_c1);
        
        if (depth <= 5 || depth % 5 == 0) {
            std::cout << "  Depth " << depth << ": φ^" << depth+1 << " = ";
            std::cout << "F(" << depth << ")·φ + F(" << depth+1 << ") ";
            std::cout << (pass ? "✓" : "✗") << "\n";
        }
        
        if (!pass) {
            std::cout << "    Expected: (" << expected_c0 << ", " << expected_c1 << ")\n";
            std::cout << "    Got: (" << actual_c0 << ", " << actual_c1 << ")\n";
            break;
        }
    }
    
    return 0;
}

#include <vector>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <chrono>

void init_ring(long Q) { NTL::ZZ_p::init(NTL::ZZ(Q)); }

void reduce_mod(NTL::ZZ_pX& poly, long N, long Q) {
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

// Maghanap ng φ sa Z_Q na may φ² = φ+1
long find_phi(long Q) {
    // Kailangan: 5 ay quadratic residue mod Q
    // φ = (1 + √5) / 2 mod Q
    
    // Hanapin ang √5 mod Q
    for (long i = 1; i < Q && i < 1000000; i++) {
        if ((i * i) % Q == 5 % Q) {
            long sqrt5 = i;
            long inv2 = (Q + 1) / 2;
            long phi = ((1 + sqrt5) * inv2) % Q;
            return phi;
        }
    }
    return -1;
}

int main() {
    std::cout << "LARGE Q TEST\n\n";
    
    // Test sa iba't ibang Q sizes
    std::vector<long> q_values = {
        536870909,           // 29 bits (current)
        2147483647,          // 31 bits (max int)
        4294967291,          // 32 bits
        1000000007,          // 30 bits (common prime)
        999999937            // 30 bits
    };
    
    for (long Q : q_values) {
        std::cout << "Q = " << Q << " (" << (int)std::log2(Q) << " bits)\n";
        
        long phi = find_phi(Q);
        
        if (phi > 0) {
            std::cout << "  φ = " << phi << "\n";
            std::cout << "  φ² mod Q = " << (phi * phi) % Q << "\n";
            std::cout << "  φ+1 mod Q = " << (phi + 1) % Q << "\n";
            std::cout << "  Match: " << (((phi * phi) % Q == (phi + 1) % Q) ? "YES ✓" : "NO ✗") << "\n";
        } else {
            std::cout << "  Walang φ (5 ay hindi QR o hindi mahanap sa limit)\n";
        }
        std::cout << "\n";
    }
    
    // Pinakamalaking testable Q
    std::cout << "=== PINAKAMALAKING TESTABLE Q ===\n";
    
    // Subukan ang 2^61 - 1 (Mersenne prime)
    long large_q = (1LL << 61) - 1;
    std::cout << "2^61 - 1 = " << large_q << " (61 bits)\n";
    
    long phi_large = find_phi(large_q);
    if (phi_large > 0) {
        std::cout << "  φ = " << phi_large << "\n";
    } else {
        std::cout << "  Hindi mahanap sa brute force (kailangan ng Tonelli-Shanks)\n";
    }
    
    return 0;
}

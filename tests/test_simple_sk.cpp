#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr int N = 1024;

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

// Reduce: X^N = -1
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
    init_ring();
    
    std::cout << "SIMPLE SECRET KEY TEST\n\n";
    
    // Approach: s = X (simplest polynomial na may inverse)
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 1, 1);  // s = X
    
    // Compute s² = X²
    NTL::ZZ_pX s_sq = s * s;
    reduce_mod(s_sq);
    
    std::cout << "s = X\n";
    std::cout << "s² = X² (degree " << NTL::deg(s_sq) << ")\n\n";
    
    // Compute s³ = X³
    NTL::ZZ_pX s_cube = s_sq * s;
    reduce_mod(s_cube);
    
    std::cout << "s³ = X³ (degree " << NTL::deg(s_cube) << ")\n\n";
    
    // Sa traditional RLWE, kailangan i-reduce ang s² at s³
    // Pero kung s = X, ang s² at s³ ay simple polynomials
    
    // Ang multiplication:
    // (c0 + c1·X) × (d0 + d1·X) = c0·d0 + (c0·d1 + c1·d0)·X + c1·d1·X²
    // Ang X² ay degree 2 lang - HINDI ito malaking problema!
    
    std::cout << "=== KEY INSIGHT ===\n";
    std::cout << "Kung s = X (o ibang low-degree polynomial):\n";
    std::cout << "  - s² ay low-degree\n";
    std::cout << "  - Hindi lumalaki ang ciphertext\n";
    std::cout << "  - Walang relinearization na kailangan!\n\n";
    
    std::cout << "Ang trade-off:\n";
    std::cout << "  - Mas simple ang s = less entropy = less secure\n";
    std::cout << "  - Pero kung ang φ foundation ay nagbibigay ng security...\n";
    
    return 0;
}

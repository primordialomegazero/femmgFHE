#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr int N = 1024;

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

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
    
    std::cout << "2-COMPONENT NA WALANG RELINEARIZATION\n\n";
    
    // IDEA: I-absorb ang s² term sa c1
    // Sa halip na (c0, c1, c2) na may hiwalay na s²
    // Gumamit ng (c0, c1) kung saan c1 ay may kasamang s
    
    // Decryption: c0 + c1·s = m
    // Multiplication: (c0 + c1·s)(d0 + d1·s)
    // = c0·d0 + (c0·d1 + c1·d0)·s + c1·d1·s²
    
    // Kung s² ay may KNOWN na relationship sa s at 1:
    // s² = a·s + b (para sa constants a, b)
    // Kaya: c1·d1·s² = c1·d1·(a·s + b) = (a·c1·d1)·s + (b·c1·d1)
    
    // Result: 
    // c0' = c0·d0 + b·c1·d1
    // c1' = c0·d1 + c1·d0 + a·c1·d1
    
    // Kung may s na may property na s² = s + 1 (φ-like):
    // a = 1, b = 1
    // c0' = c0·d0 + c1·d1
    // c1' = c0·d1 + c1·d0 + c1·d1
    
    std::cout << "Kung s² = s + 1 (φ property):\n";
    std::cout << "  c0' = c0·d0 + c1·d1\n";
    std::cout << "  c1' = c0·d1 + c1·d0 + c1·d1\n\n";
    
    std::cout << "Walang relinearization key!\n";
    std::cout << "Walang dimension growth!\n";
    std::cout << "2 components lang!\n\n";
    
    // Test: Hanapin kung may s sa ring na may s² = s + 1
    std::cout << "Kailangan: s² - s - 1 = 0 (sa ring)\n";
    std::cout << "Ito ay katulad ng φ sa scalar case\n";
    std::cout << "Sa polynomial ring, kailangan ng s na polynomial\n";
    std::cout << "na nagse-satisfy ng s² - s - 1 = 0\n\n";
    
    // Simple test: s = φ (constant polynomial)
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, 386640388);  // φ mod Q
    
    NTL::ZZ_pX s_sq = s * s;
    reduce_mod(s_sq);
    NTL::ZZ_pX s_plus_1 = s + NTL::ZZ_pX(1);
    
    std::cout << "Test s = φ (constant):\n";
    std::cout << "  s² coeff[0] = " << NTL::coeff(s_sq, 0) << "\n";
    std::cout << "  s+1 coeff[0] = " << NTL::coeff(s_plus_1, 0) << "\n";
    std::cout << "  Match: " << (NTL::coeff(s_sq, 0) == NTL::coeff(s_plus_1, 0) ? "YES" : "NO") << "\n";
    
    return 0;
}

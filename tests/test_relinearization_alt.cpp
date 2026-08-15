#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr long PHI_MOD_Q = 386640388;
constexpr long PSI_MOD_Q = 150230522;

int main() {
    std::cout << "RELINEARIZATION VIA φ·ψ = -1 (ALTERNATIVE)\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // Ang relinearization ay kailangan para i-reduce ang s³ at s⁴ terms
    // pabalik sa s², s, at constant.
    
    // Traditional: Kailangan ng relinearization key (encrypted s²)
    
    // Alternative: Gamitin ang φ·ψ = -1 para sa cancellation
    
    std::cout << "Observation:\n";
    std::cout << "  φ·ψ = -1\n";
    std::cout << "  φ·ψ + 1 = 0\n";
    std::cout << "  Ito ay identity na pwedeng gamitin para sa reduction!\n\n";
    
    std::cout << "Sa ring, kung may element r na may property:\n";
    std::cout << "  r² - r - 1 = 0\n";
    std::cout << "  r² = r + 1\n\n";
    
    std::cout << "Ang φ = " << PHI_MOD_Q << " ay may ganitong property.\n";
    std::cout << "Ang ψ = " << PSI_MOD_Q << " ay may ganitong property din.\n\n";
    
    // Test: Pwede bang gamitin ang φ para sa reduction?
    NTL::ZZ_p phi_p;
    phi_p = PHI_MOD_Q;
    
    NTL::ZZ_p phi_sq = phi_p * phi_p;
    NTL::ZZ_p phi_plus_1 = phi_p + NTL::ZZ_p(1);
    
    std::cout << "φ² mod Q = " << phi_sq << "\n";
    std::cout << "φ+1 mod Q = " << phi_plus_1 << "\n";
    std::cout << "Match: " << (phi_sq == phi_plus_1 ? "YES" : "NO") << "\n\n";
    
    // Ang ideya: I-multiply ang ciphertext sa φ para ma-reduce
    // (c0 + c1·s + c2·s²) · φ = c0·φ + c1·φ·s + c2·φ·s²
    // Kung s² = s+1 (φ-like), ang result ay:
    // c0·φ + c1·φ·s + c2·φ·(s+1) = (c0·φ + c2·φ) + (c1·φ + c2·φ)·s
    
    std::cout << "POTENTIAL APPROACH:\n";
    std::cout << "1. I-multiply ang ciphertext sa φ\n";
    std::cout << "2. Ang s² term ay nagre-reduce sa s at constant\n";
    std::cout << "3. Walang relinearization key na kailangan\n\n";
    
    std::cout << "Kailangan i-verify:\n";
    std::cout << "1. Totoo ba na s² = s+1 para sa NTT ring?\n";
    std::cout << "2. Ang multiplication ba sa φ ay homomorphic?\n";
    std::cout << "3. Ang noise ba ay nape-preserve?\n";
    
    return 0;
}

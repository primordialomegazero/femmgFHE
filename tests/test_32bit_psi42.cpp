#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ_p::init(NTL::ZZ(4294967291));
    
    std::cout << "ψ^42 VERIFICATION\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(4294967291);
    NTL::ZZ phi = NTL::to_ZZ(2147516414);
    NTL::ZZ s = NTL::to_ZZ(402045813);
    NTL::ZZ alpha = NTL::to_ZZ(599074578);  // L(42)
    NTL::ZZ golden_plain = NTL::to_ZZ(2654435766);
    NTL::ZZ inv_golden = NTL::to_ZZ(2775463862);
    
    // Compute ψ = 1 - φ
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    std::cout << "ψ = 1 - φ = " << psi << "\n\n";
    
    // Compute ψ^42
    NTL::ZZ_p psi_p = NTL::to_ZZ_p(psi);
    NTL::ZZ_p psi_42 = NTL::to_ZZ_p(1);
    for (int i = 0; i < 42; i++) psi_42 = psi_42 * psi_p;
    NTL::ZZ psi_42_zz = rep(psi_42);
    
    std::cout << "ψ^42 = " << psi_42_zz << "\n";
    std::cout << "golden_plain = " << golden_plain << "\n";
    std::cout << "Match: " << (psi_42_zz == golden_plain ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify: s + ψ^42 = L(42)
    NTL::ZZ sum = (s + psi_42_zz) % Q;
    std::cout << "s + ψ^42 = " << sum << "\n";
    std::cout << "L(42) = " << alpha << "\n";
    std::cout << "Match: " << (sum == alpha ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify: s × ψ^42 = 1
    NTL::ZZ prod = (s * psi_42_zz) % Q;
    std::cout << "s × ψ^42 = " << prod << "\n";
    std::cout << "Match: " << (prod == 1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Check inv_golden
    std::cout << "inv_golden = " << inv_golden << "\n";
    std::cout << "ψ^42 × inv_golden = " << (psi_42_zz * inv_golden) % Q << "\n";
    std::cout << "golden_plain × inv_golden = " << (golden_plain * inv_golden) % Q << "\n\n";
    
    // Check ψ^41
    NTL::ZZ_p psi_41 = NTL::to_ZZ_p(1);
    for (int i = 0; i < 41; i++) psi_41 = psi_41 * psi_p;
    NTL::ZZ psi_41_zz = rep(psi_41);
    
    std::cout << "ψ^41 = " << psi_41_zz << "\n";
    std::cout << "inv_golden = " << inv_golden << "\n";
    std::cout << "Match: " << (psi_41_zz == inv_golden ? "YES ✓" : "NO ✗") << "\n";
    
    return 0;
}

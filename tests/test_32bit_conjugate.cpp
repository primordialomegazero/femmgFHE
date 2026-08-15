#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ_p::init(NTL::ZZ(4294967291));
    
    std::cout << "CONJUGATE ROOT VERIFICATION\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(4294967291);
    NTL::ZZ s = NTL::to_ZZ(402045813);
    NTL::ZZ alpha = NTL::to_ZZ(599074578);
    NTL::ZZ beta = NTL::to_ZZ(4294967290);
    NTL::ZZ golden_plain = NTL::to_ZZ(2654435766);
    
    std::cout << "s = " << s << "\n";
    std::cout << "golden_plain = " << golden_plain << "\n";
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = " << beta << "\n\n";
    
    // Verify s² = α·s + β
    NTL::ZZ s_sq = (s * s) % Q;
    NTL::ZZ asb = (alpha * s + beta) % Q;
    std::cout << "s² = " << s_sq << "\n";
    std::cout << "α·s + β = " << asb << "\n";
    std::cout << "Match: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify golden_plain² = α·golden_plain + β
    NTL::ZZ gp_sq = (golden_plain * golden_plain) % Q;
    NTL::ZZ agb = (alpha * golden_plain + beta) % Q;
    std::cout << "golden_plain² = " << gp_sq << "\n";
    std::cout << "α·golden_plain + β = " << agb << "\n";
    std::cout << "Match: " << (gp_sq == agb ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify s + golden_plain = α?
    NTL::ZZ sum = (s + golden_plain) % Q;
    std::cout << "s + golden_plain = " << sum << "\n";
    std::cout << "α = " << alpha << "\n";
    std::cout << "Match: " << (sum == alpha ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify s * golden_plain = -β?
    NTL::ZZ prod = (s * golden_plain) % Q;
    NTL::ZZ neg_beta = (Q - beta) % Q;
    std::cout << "s * golden_plain = " << prod << "\n";
    std::cout << "-β = " << neg_beta << "\n";
    std::cout << "Match: " << (prod == neg_beta ? "YES ✓" : "NO ✗") << "\n";
    
    return 0;
}

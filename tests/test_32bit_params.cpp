#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ_p::init(NTL::ZZ(4294967291));
    
    std::cout << "32-BIT PARAMETERS ANALYSIS\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(4294967291);
    NTL::ZZ phi = NTL::to_ZZ(2147516414);
    NTL::ZZ s = NTL::to_ZZ(402045813);
    NTL::ZZ alpha = NTL::to_ZZ(599074578);
    NTL::ZZ beta = NTL::to_ZZ(4294967290);
    NTL::ZZ golden_plain = NTL::to_ZZ(2654435766);
    NTL::ZZ inv_golden = NTL::to_ZZ(2775463862);
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "φ = " << phi << "\n";
    std::cout << "s = φ^42 = " << s << "\n";
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = " << beta << "\n";
    std::cout << "golden_plain = " << golden_plain << "\n";
    std::cout << "inv_golden = " << inv_golden << "\n\n";
    
    // ========== RELATIONS ==========
    std::cout << "RELATIONS:\n\n";
    
    // 1. φ² = φ+1
    NTL::ZZ phi_sq = (phi * phi) % Q;
    std::cout << "1. φ² = " << phi_sq << ", φ+1 = " << (phi+1)%Q;
    std::cout << (phi_sq == (phi+1)%Q ? " ✓" : " ✗") << "\n";
    
    // 2. s = φ^42
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi);
    NTL::ZZ_p s_check = NTL::to_ZZ_p(1);
    for (int i = 0; i < 42; i++) s_check = s_check * phi_p;
    std::cout << "2. s = φ^42: " << (rep(s_check) == s ? "✓" : "✗") << "\n";
    
    // 3. α = F(42)?
    NTL::ZZ fib[50];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 50; i++) fib[i] = (fib[i-1] + fib[i-2]) % Q;
    std::cout << "3. α = F(42) = " << fib[42] << ": " << (alpha == fib[42] ? "✓" : "✗") << "\n";
    std::cout << "   β = F(41) = " << fib[41] << ": " << (beta == fib[41] ? "✓" : "✗") << "\n";
    
    // 4. s² = α·s + β?
    NTL::ZZ s_sq = (s * s) % Q;
    NTL::ZZ alpha_s_plus_beta = (alpha * s + beta) % Q;
    std::cout << "4. s² = α·s + β: " << (s_sq == alpha_s_plus_beta ? "✓" : "✗") << "\n";
    
    // 5. golden_plain = ?
    std::cout << "\n5. golden_plain analysis:\n";
    std::cout << "   golden_plain = " << golden_plain << "\n";
    std::cout << "   φ = " << phi << "\n";
    std::cout << "   s = " << s << "\n";
    std::cout << "   golden_plain - φ = " << (golden_plain - phi + Q) % Q << "\n";
    std::cout << "   golden_plain - s = " << (golden_plain - s + Q) % Q << "\n";
    std::cout << "   golden_plain * inv_golden % Q = " << (golden_plain * inv_golden) % Q << "\n";
    
    // 6. inv_golden analysis
    std::cout << "\n6. inv_golden analysis:\n";
    std::cout << "   inv_golden = " << inv_golden << "\n";
    std::cout << "   inv_golden * φ % Q = " << (inv_golden * phi) % Q << "\n";
    std::cout << "   inv_golden * s % Q = " << (inv_golden * s) % Q << "\n";
    
    // 7. golden_plain^2?
    NTL::ZZ gp_sq = (golden_plain * golden_plain) % Q;
    std::cout << "\n7. golden_plain² = " << gp_sq << "\n";
    std::cout << "   golden_plain + 1 = " << (golden_plain + 1) % Q << "\n";
    std::cout << "   Match: " << (gp_sq == (golden_plain+1)%Q ? "✓" : "✗") << "\n";
    
    return 0;
}

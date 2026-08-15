#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ_p::init(NTL::ZZ(4294967291));
    
    std::cout << "REVERSE ENGINEER 32-BIT PARAMETERS\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(4294967291);
    NTL::ZZ phi = NTL::to_ZZ(2147516414);
    NTL::ZZ s = NTL::to_ZZ(402045813);
    NTL::ZZ alpha = NTL::to_ZZ(599074578);
    NTL::ZZ beta = NTL::to_ZZ(4294967290);
    NTL::ZZ golden_plain = NTL::to_ZZ(2654435766);
    NTL::ZZ inv_golden = NTL::to_ZZ(2775463862);
    
    std::cout << "KNOWN VALUES:\n";
    std::cout << "  Q = " << Q << "\n";
    std::cout << "  φ = " << phi << "\n";
    std::cout << "  s = " << s << "\n";
    std::cout << "  α = " << alpha << "\n";
    std::cout << "  β = " << beta << "\n";
    std::cout << "  golden_plain = " << golden_plain << "\n";
    std::cout << "  inv_golden = " << inv_golden << "\n\n";
    
    // ========== RELATIONSHIPS ==========
    std::cout << "RELATIONSHIPS:\n\n";
    
    // 1. s = φ^42?
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi);
    NTL::ZZ_p s_check = NTL::to_ZZ_p(1);
    for (int i = 0; i < 42; i++) s_check = s_check * phi_p;
    std::cout << "1. s = φ^42: " << (rep(s_check) == s ? "YES ✓" : "NO ✗") << "\n";
    
    // 2. α at β relationship
    NTL::ZZ fib[50];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 50; i++) fib[i] = (fib[i-1] + fib[i-2]) % Q;
    
    std::cout << "2. α = F(42)? " << (alpha == fib[42] ? "YES" : "NO") << "\n";
    std::cout << "   β = F(41)? " << (beta == fib[41] ? "YES" : "NO") << "\n";
    std::cout << "   F(42) = " << fib[42] << "\n";
    std::cout << "   F(41) = " << fib[41] << "\n\n";
    
    // 3. s² = α·s + β?
    NTL::ZZ s_sq = (s * s) % Q;
    NTL::ZZ asb = (alpha * s + beta) % Q;
    std::cout << "3. s² = α·s + β: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n";
    std::cout << "   s² = " << s_sq << "\n";
    std::cout << "   α·s+β = " << asb << "\n\n";
    
    // 4. golden_plain analysis
    std::cout << "4. golden_plain = " << golden_plain << "\n";
    std::cout << "   golden_plain mod φ = " << (golden_plain % phi) << "\n";
    std::cout << "   golden_plain / φ = " << (golden_plain / phi) << "\n";
    std::cout << "   golden_plain - s = " << (golden_plain - s + Q) % Q << "\n";
    std::cout << "   golden_plain * inv_golden % Q = " << (golden_plain * inv_golden) % Q << "\n\n";
    
    // 5. inv_golden analysis
    std::cout << "5. inv_golden = " << inv_golden << "\n";
    std::cout << "   inv_golden * φ % Q = " << (inv_golden * phi) % Q << "\n";
    std::cout << "   inv_golden * s % Q = " << (inv_golden * s) % Q << "\n";
    std::cout << "   inv_golden * golden_plain % Q = " << (inv_golden * golden_plain) % Q << "\n\n";
    
    // 6. s and golden_plain relation
    std::cout << "6. s vs golden_plain:\n";
    std::cout << "   s = " << s << "\n";
    std::cout << "   golden_plain = " << golden_plain << "\n";
    std::cout << "   s² % Q = " << (s * s) % Q << "\n";
    std::cout << "   golden_plain² % Q = " << (golden_plain * golden_plain) % Q << "\n";
    std::cout << "   s * golden_plain % Q = " << (s * golden_plain) % Q << "\n\n";
    
    // 7. Try: golden_plain = φ^k?
    std::cout << "7. golden_plain = φ^k?\n";
    NTL::ZZ_p gp_check = NTL::to_ZZ_p(1);
    for (int k = 1; k <= 100; k++) {
        gp_check = gp_check * phi_p;
        if (rep(gp_check) == golden_plain) {
            std::cout << "   golden_plain = φ^" << k << " ✓\n";
            break;
        }
    }
    
    return 0;
}

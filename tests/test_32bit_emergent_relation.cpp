#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    NTL::ZZ_p::init(NTL::ZZ(4294967291));
    
    std::cout << "EMERGENT RELATION DISCOVERY\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(4294967291);
    NTL::ZZ phi = NTL::to_ZZ(2147516414);
    NTL::ZZ s = NTL::to_ZZ(402045813);
    NTL::ZZ alpha = NTL::to_ZZ(599074578);
    NTL::ZZ beta = NTL::to_ZZ(4294967290);
    NTL::ZZ golden_plain = NTL::to_ZZ(2654435766);
    NTL::ZZ inv_golden = NTL::to_ZZ(2775463862);
    
    std::cout << "VALUES:\n";
    std::cout << "  φ = " << phi << "\n";
    std::cout << "  s = φ^42 = " << s << "\n";
    std::cout << "  golden_plain = " << golden_plain << "\n";
    std::cout << "  inv_golden = " << inv_golden << "\n\n";
    
    // ========== EMERGENT PATTERNS ==========
    std::cout << "PATTERN HUNTING:\n\n";
    
    // 1. golden_plain = ?
    std::cout << "1. golden_plain relation:\n";
    std::cout << "   golden_plain = " << golden_plain << "\n";
    std::cout << "   φ + s = " << (phi + s) % Q << "\n";
    std::cout << "   φ - s = " << (phi - s + Q) % Q << "\n";
    std::cout << "   φ * s % Q = " << (phi * s) % Q << "\n";
    std::cout << "   φ^43 = " << rep(NTL::to_ZZ_p(phi) * NTL::to_ZZ_p(s)) << "\n\n";
    
    // 2. Check kung golden_plain = φ^k para sa ibang k
    std::cout << "2. golden_plain = φ^k search:\n";
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi);
    NTL::ZZ_p power = NTL::to_ZZ_p(1);
    bool found = false;
    for (int k = 0; k < 100; k++) {
        if (rep(power) == golden_plain) {
            std::cout << "   golden_plain = φ^" << k << " ✓\n";
            found = true;
            break;
        }
        power = power * phi_p;
    }
    if (!found) std::cout << "   Hindi φ^k\n\n";
    
    // 3. inv_golden = φ^k?
    std::cout << "3. inv_golden = φ^k search:\n";
    power = NTL::to_ZZ_p(1);
    found = false;
    for (int k = 0; k < 100; k++) {
        if (rep(power) == inv_golden) {
            std::cout << "   inv_golden = φ^" << k << " ✓\n";
            found = true;
            break;
        }
        power = power * phi_p;
    }
    if (!found) std::cout << "   Hindi φ^k\n\n";
    
    // 4. golden_plain = s^k?
    std::cout << "4. golden_plain = s^k search:\n";
    NTL::ZZ_p s_p = NTL::to_ZZ_p(s);
    power = NTL::to_ZZ_p(1);
    found = false;
    for (int k = 0; k < 100; k++) {
        if (rep(power) == golden_plain) {
            std::cout << "   golden_plain = s^" << k << " ✓\n";
            found = true;
            break;
        }
        power = power * s_p;
    }
    if (!found) std::cout << "   Hindi s^k\n\n";
    
    // 5. Fibonacci relation
    std::cout << "5. Fibonacci relation:\n";
    NTL::ZZ fib[100];
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 100; i++) fib[i] = (fib[i-1] + fib[i-2]) % Q;
    
    std::cout << "   F(42) = " << fib[42] << "\n";
    std::cout << "   F(41) = " << fib[41] << "\n";
    std::cout << "   α = " << alpha << "\n";
    std::cout << "   β = " << beta << "\n";
    std::cout << "   α - F(42) = " << (alpha - fib[42] + Q) % Q << "\n";
    std::cout << "   β - F(41) = " << (beta - fib[41] + Q) % Q << "\n\n";
    
    // 6. golden_plain = F(k)?
    std::cout << "6. golden_plain = F(k) search:\n";
    found = false;
    for (int k = 0; k < 100; k++) {
        if (fib[k] == golden_plain) {
            std::cout << "   golden_plain = F(" << k << ") ✓\n";
            found = true;
            break;
        }
    }
    if (!found) std::cout << "   Hindi F(k)\n\n";
    
    // 7. inv_golden = F(k)?
    std::cout << "7. inv_golden = F(k) search:\n";
    found = false;
    for (int k = 0; k < 100; k++) {
        if (fib[k] == inv_golden) {
            std::cout << "   inv_golden = F(" << k << ") ✓\n";
            found = true;
            break;
        }
    }
    if (!found) std::cout << "   Hindi F(k)\n\n";
    
    // 8. Lucas numbers
    std::cout << "8. Lucas relation:\n";
    // L(n) = F(n-1) + F(n+1)
    std::cout << "   L(42) = " << (fib[41] + fib[43]) % Q << "\n";
    std::cout << "   L(41) = " << (fib[40] + fib[42]) % Q << "\n";
    std::cout << "   α = " << alpha << " (L(42)? " << (alpha == (fib[41]+fib[43])%Q ? "YES" : "NO") << ")\n";
    std::cout << "   β = " << beta << " (L(41)? " << (beta == (fib[40]+fib[42])%Q ? "YES" : "NO") << ")\n\n";
    
    // 9. golden_plain = L(k)?
    std::cout << "9. golden_plain = L(k) search:\n";
    found = false;
    for (int k = 2; k < 99; k++) {
        NTL::ZZ L = (fib[k-1] + fib[k+1]) % Q;
        if (L == golden_plain) {
            std::cout << "   golden_plain = L(" << k << ") ✓\n";
            found = true;
            break;
        }
    }
    if (!found) std::cout << "   Hindi L(k)\n\n";
    
    // 10. Direct relation
    std::cout << "10. Direct computation:\n";
    std::cout << "    s² = " << (s*s)%Q << "\n";
    std::cout << "    s² - s = " << ((s*s - s + Q)%Q) << "\n";
    std::cout << "    s² + s = " << ((s*s + s)%Q) << "\n";
    std::cout << "    golden_plain - s² = " << ((golden_plain - (s*s)%Q + Q)%Q) << "\n";
    std::cout << "    golden_plain + s² = " << ((golden_plain + (s*s)%Q)%Q) << "\n";
    std::cout << "    φ^84 = " << rep(power) << " (φ^84 = (φ^42)² = s²)\n";
    
    return 0;
}

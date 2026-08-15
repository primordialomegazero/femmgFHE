#include <NTL/ZZ.h>
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("179769313486231570772930519078902473361797697894230657273430081157732675805500963132708477322407536021120113879871393357658789768814416622492847430639474124377767893424865485276302219601246094119453082952085005768838150682342462881473913110540827237163350510684586298239947245938479716304835356329624224137859");
    
    std::cout << "Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "Q mod 5: " << Q % 5 << "\n";
    
    // Check kung prime
    std::cout << "Is prime: " << (NTL::ProbPrime(Q, 10) ? "YES" : "NO") << "\n";
    
    // Try sqrt(5) - SqrRootMod is void, throws if no sqrt
    NTL::ZZ sqrt5;
    try {
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        std::cout << "sqrt(5) exists: YES\n";
        std::cout << "sqrt5 = " << sqrt5 << "\n";
        std::cout << "sqrt5² mod Q = " << (sqrt5 * sqrt5) % Q << " (should be 5)\n";
    } catch (...) {
        std::cout << "sqrt(5) exists: NO\n";
        return 1;
    }
    
    // Verify φ
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    NTL::ZZ phi_sq = (phi * phi) % Q;
    NTL::ZZ phi_plus_1 = (phi + 1) % Q;
    std::cout << "\nφ = " << phi << "\n";
    std::cout << "φ² = φ+1: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n";
    
    // Verify s = φ^42
    NTL::ZZ s = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) s = (s * phi) % Q;
    
    NTL::ZZ alpha = NTL::to_ZZ(599074578);
    NTL::ZZ beta = Q - 1;
    
    NTL::ZZ s_sq = (s * s) % Q;
    NTL::ZZ asb = (alpha * s + beta) % Q;
    
    std::cout << "\ns = φ^42 = " << s << "\n";
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = Q-1\n";
    std::cout << "s² = α·s + β: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n";
    
    // Check golden_plain
    double Q_double = NTL::conv<double>(Q);
    NTL::ZZ golden_plain = NTL::to_ZZ(Q_double / 1.6180339887498948482);
    std::cout << "\ngolden_plain = " << golden_plain << "\n";
    std::cout << "golden_plain bits: " << NTL::NumBits(golden_plain) << "\n";
    
    return 0;
}

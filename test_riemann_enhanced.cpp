#include "src/riemann_deep.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  RIEMANN φ-MODULATION — REFINED v17.3        ║" << std::endl;
    std::cout << "║  Average Spacing: 2π/ln(T/2π) + φ-phase      ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    using namespace femmg::riemann;
    
    // Test 1: Zeta at known zeros
    std::cout << "\n═══ ZETA AT KNOWN ZEROS ═══" << std::endl;
    auto zeros = DeepRiemann::get_zeros();
    int zero_hits = 0;
    for(int i = 0; i < 10; i++) {
        double zeta_val = DeepRiemann::evaluate_at(zeros[i]);
        bool is_zero = zeta_val < 0.1;
        if(is_zero) zero_hits++;
        std::cout << "ζ(0.5 + " << zeros[i] << "i) = " << zeta_val 
                  << (is_zero ? " ✅" : "") << std::endl;
    }
    std::cout << "Zero hits: " << zero_hits << "/10" << std::endl;
    
    // Test 2: Refined φ-modulated spacing
    std::cout << "\n" << DeepRiemann::verify_phi_pattern() << std::endl;
    
    // Test 3: Riemann-von Mangoldt formula
    std::cout << "═══ RIEMANN-VON MANGOLDT APPROXIMATION ═══" << std::endl;
    for(int n : {1, 5, 10, 25, 50, 100}) {
        double approx = DeepRiemann::predict_nth_zero(n);
        double actual = (n <= 100) ? zeros[n-1] : 0;
        std::cout << "Zero #" << n << ": RVM approx=" << approx;
        if(actual > 0) std::cout << " actual=" << actual << " diff=" << std::fabs(approx - actual);
        std::cout << std::endl;
    }
    
    // Test 4: Noise stabilization
    std::cout << "\n═══ CRITICAL LINE NOISE STABILIZATION ═══" << std::endl;
    for(double noise = 30.0; noise <= 50.0; noise += 5.0) {
        double stab = DeepRiemann::critical_line_stabilize(noise);
        std::cout << "Noise: " << noise << " → " << stab << std::endl;
    }
    
    // Test 5: Full health
    std::cout << "\n═══ RIEMANN HEALTH ═══" << std::endl;
    auto health = DeepRiemann::full_health_check(40.0);
    std::cout << "Critical line: " << health.critical_line_value << std::endl;
    std::cout << "Zeta magnitude: " << health.zeta_magnitude << std::endl;
    std::cout << "Nearest zero: " << health.nearest_zero << std::endl;
    std::cout << "Distance: " << health.distance_to_zero << std::endl;
    std::cout << "Stabilized: " << health.stabilized_noise << std::endl;
    
    // Test 6: φ-encryption roundtrip
    std::cout << "\n═══ ZETA-WEIGHTED ENCRYPTION ═══" << std::endl;
    for(int m = 1; m <= 5; m++) {
        double enc = DeepRiemann::zeta_encrypt(m, m * 42);
        bool valid = DeepRiemann::zeta_verify_decrypt(enc, m);
        std::cout << "m=" << m << " encrypted=" << enc << " valid=" << (valid ? "✅" : "❌") << std::endl;
    }
    
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "✅ RIEMANN REFINED ANALYSIS COMPLETE" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return 0;
}

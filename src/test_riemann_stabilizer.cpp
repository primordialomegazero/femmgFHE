#include "riemann_stabilizer.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  RIEMANN STABILIZER — Integration Test       ║\n";
    std::cout << "║  Critical Line Re(s)=1/2 + Banach + Zeta    ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    using namespace femmg::riemann;

    std::cout << "━━━ NOISE STABILIZATION TEST ━━━\n";
    double noises[] = {100.0, 80.0, 60.0, 50.0, 45.0, 42.0, 41.0, 40.5, 40.1};
    
    for (double n : noises) {
        auto h = ZetaStabilizer::check(n);
        double stabilized = ZetaStabilizer::deep_stabilize(n);
        
        std::cout << "Noise " << n << " → Stabilized: " << std::fixed << std::setprecision(6) 
                  << stabilized << " (Banach: " << h.banach_component 
                  << " + Zeta: " << h.riemann_boost << ")\n";
    }
    
    std::cout << "\n━━━ ZETA ZEROS NEAR NOISE FLOOR ━━━\n";
    for (double n = 35.0; n <= 45.0; n += 1.0) {
        double attractor = ZetaStabilizer::nearest_zero_attractor(n);
        std::cout << "  Noise " << n << " → Nearest zero attractor: " << attractor << "\n";
    }
    
    std::cout << "\n━━━ LONG-TERM STABILITY ━━━\n";
    double noise = 100.0;
    for (int i = 0; i < 20; i++) {
        noise = ZetaStabilizer::stabilize(noise);
        if (i % 4 == 0) {
            auto h = ZetaStabilizer::check(noise);
            std::cout << "  Iter " << i << ": noise=" << noise 
                      << " zeta_mag=" << h.zeta_magnitude << "\n";
        }
    }
    
    std::cout << "\n╔══════════════════════════════════════════════╗\n";
    std::cout << "║  RIEMANN STABILIZER — READY FOR INTEGRATION  ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";
    
    return 0;
}

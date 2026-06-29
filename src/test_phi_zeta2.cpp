#include "phi_zeta_spacing.h"
#include <iostream>
#include <iomanip>

int main() {
    using namespace femmg::phizeta;
    auto p = PhiZetaSpacing::analyze();
    
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  φ-ZETA SPACING v2 — RETWEAKED               ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";
    
    std::cout << "Mean gap: " << p.mean_gap << "\n";
    std::cout << "φ/mean ratio: " << p.phi_mean_ratio << "\n";
    std::cout << "φ-resonance: " << p.resonance_score << "\n";
    std::cout << "Next zero: " << p.predicted_next_zero << "\n\n";
    
    std::cout << "━━━ STABILIZATION ━━━\n";
    for (double n : {30.0, 35.0, 40.0, 42.0, 45.0, 50.0, 55.0, 60.0}) {
        double s = PhiZetaSpacing::phi_zeta_stabilize(n, 40.0);
        std::cout << "  " << n << " → " << s << " (pull: " << (s - n) << ")\n";
    }
    
    std::cout << "\n╔══════════════════════════════════════════════╗\n";
    std::cout << "║  φ-Zeta Spacing — Ready for FHE Integration  ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";
}

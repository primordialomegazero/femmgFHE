#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double mirror_collapse(double value, bool was_phi_path) {
    double collapsed = was_phi_path ? value * PSI : value * PHI;
    return std::abs(collapsed);
}

int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    std::uniform_int_distribution<int> bit(0, 1);
    
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "=== SPIRAL REVERSIBILITY DEBUG ===\n\n";
    
    for (int i = 0; i < 10; i++) {
        double v = val(gen);
        bool phi_path = (bit(gen) == 1);
        
        // Simulate gate output
        double gate_out = phi_path ? v*PHI : v*PSI;
        
        // Layer 1: Mirror collapse → canonical
        double canonical = mirror_collapse(gate_out, phi_path);
        
        // Layer 2: Spiral outward (×φ) + shuffle simulation
        double spiral_out = canonical * PHI;
        
        // Layer 3: Mirror collapse AGAIN
        // spiral_out came from canonical×φ, so was_phi_path = true
        double back_to_canonical = mirror_collapse(spiral_out, true);
        
        // Layer 4: Spiral outward again
        double spiral_out2 = back_to_canonical * PHI;
        
        // Layer 5: Mirror collapse again
        double back_again = mirror_collapse(spiral_out2, true);
        
        std::cout << "v=" << v 
                  << " gate=" << gate_out
                  << " can1=" << canonical
                  << " spir1=" << spiral_out
                  << " can2=" << back_to_canonical
                  << " spir2=" << spiral_out2
                  << " can3=" << back_again
                  << "\n";
        
        // Check: is canonical preserved?
        double diff1 = std::abs(canonical - back_to_canonical);
        double diff2 = std::abs(canonical - back_again);
        
        std::cout << "  can1==can2? diff=" << diff1;
        if (diff1 > 1e-10) std::cout << " ❌ DIVERGENCE!";
        std::cout << "\n  can1==can3? diff=" << diff2;
        if (diff2 > 1e-10) std::cout << " ❌ DIVERGENCE!";
        std::cout << "\n\n";
        
        // Also check: what should spiral_out mirror to?
        double expected = std::abs(spiral_out * PSI);  // spiral_out is φ-weighted, so mirror=×ψ
        std::cout << "  spiral_out=" << spiral_out 
                  << " spiral_out×PSI=" << (spiral_out*PSI)
                  << " |spiral_out×PSI|=" << expected
                  << " expected canonical=" << canonical
                  << " match=" << (std::abs(expected-canonical) < 1e-10 ? "YES" : "NO")
                  << "\n\n";
    }
    
    return 0;
}

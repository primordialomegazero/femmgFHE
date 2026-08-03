#include <iostream>
#include <iomanip>
#include <cmath>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double mirror_collapse(double value, bool was_phi_path) {
    double collapsed = was_phi_path ? value * PSI : value * PHI;
    return std::abs(collapsed);
}

int main() {
    std::cout << std::fixed << std::setprecision(15);
    
    // N=8: layers 0-7
    // layer 0: even → PHI
    // layer 1: odd  → PSI
    // layer 2: even → PHI
    // layer 3: odd  → PSI
    // layer 4: even → PHI
    // layer 5: odd  → PSI
    // layer 6: even → PHI
    // layer 7: odd  → PSI
    // Total: PHI^4 * PSI^4 = (PHI*PSI)^4 = (-1)^4 = 1.0
    
    // Last layer = layer 7 = odd → PSI
    // last_is_phi = (N_layers-1)%2==0 = 7%2==0 = false
    // mirror_collapse(v, false) = v * PHI
    
    double total = 1.0;
    for (int i = 0; i < 8; i++) {
        total *= (i % 2 == 0) ? PHI : PSI;
    }
    std::cout << "Total product (PHI^4 * PSI^4): " << total << "\n";
    std::cout << "Expected: (-1)^4 = " << std::pow(-1.0, 4) << "\n\n";
    
    double v = 0.5;
    double scaled = v * total;  // = 0.5 * 1.0 = 0.5
    std::cout << "v = " << v << "\n";
    std::cout << "scaled = v * total = " << scaled << "\n\n";
    
    // Mirror bridge: last_is_phi = false (layer 7 is odd)
    bool last_is_phi = false;
    double collapsed = mirror_collapse(scaled, last_is_phi);
    std::cout << "mirror_collapse(" << scaled << ", false) = " << collapsed << "\n";
    std::cout << "  = scaled * PHI = " << (scaled * PHI) << "\n";
    std::cout << "  = abs(" << (scaled * PHI) << ") = " << collapsed << "\n\n";
    
    std::cout << "Expected: " << v << "\n";
    std::cout << "Got: " << collapsed << "\n";
    std::cout << "Match: " << (std::abs(collapsed - v) < 1e-10 ? "YES" : "NO") << "\n\n";
    
    // THE PROBLEM:
    // Total product = 1.0 (even number of φ,ψ pairs)
    // Mirror multiplies by PHI → 1.0 * PHI = PHI ≠ 1.0
    // We need: mirror should multiply by PSI to get 1.0 * PSI = PSI → abs = 0.618 ≠ 1.0
    // OR: mirror should divide by total, not multiply by conjugate
    
    std::cout << "=== THE FIX FOR EVEN N ===\n";
    std::cout << "For even N: total = (PHI*PSI)^(N/2) = (-1)^(N/2)\n";
    std::cout << "  N=2: total = -1\n";
    std::cout << "  N=4: total = 1\n";
    std::cout << "  N=6: total = -1\n";
    std::cout << "  N=8: total = 1\n\n";
    
    std::cout << "When total = 1 (N=4,8,12...):\n";
    std::cout << "  Mirror bridge has NOTHING to undo!\n";
    std::cout << "  scaled = original * 1 = original\n";
    std::cout << "  mirror_collapse(original, false) = original * PHI ≠ original\n";
    std::cout << "  → Mirror Bridge BREAKS for N=4,8,12...\n\n";
    
    std::cout << "THE REAL FIX:\n";
    std::cout << "  Compute total_product. If |total_product - 1| < 1e-10:\n";
    std::cout << "    SKIP mirror bridge (nothing to undo)\n";
    std::cout << "  Else:\n";
    std::cout << "    Apply mirror bridge normally\n";
    
    return 0;
}

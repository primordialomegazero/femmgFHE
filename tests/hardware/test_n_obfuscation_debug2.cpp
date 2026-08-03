#include <iostream>
#include <iomanip>
#include <cmath>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    std::cout << std::fixed << std::setprecision(15);
    
    std::cout << "=== THE REAL PROBLEM ===\n\n";
    std::cout << "PHI = " << PHI << "\n";
    std::cout << "PSI = " << PSI << "\n";
    std::cout << "|PSI| = " << std::abs(PSI) << "\n";
    std::cout << "PHI * PSI = " << (PHI * PSI) << " (should be -1)\n";
    std::cout << "PHI * |PSI| = " << (PHI * std::abs(PSI)) << " (should be 1, but NOT -1)\n\n";
    
    // The obfuscate_round uses |PSI|, not PSI
    // Even layer: scale = PHI
    // Odd layer: scale = |PSI| = 0.618...
    
    std::cout << "=== N=2 (even) ===\n";
    double v = 0.5;
    std::cout << "Original v = " << v << "\n";
    
    // Layer 0 (even): scale = PHI
    v *= PHI;
    std::cout << "After layer 0 (×PHI): " << v << "\n";
    
    // Layer 1 (odd): scale = |PSI|
    v *= std::abs(PSI);
    std::cout << "After layer 1 (×|PSI|): " << v << "\n";
    
    // Now mirror collapse: last_layer_is_phi = (N_layers-1)%2==0 = 1%2==0 = false
    // mirror_collapse(v, false) = v * PHI (since not phi_path)
    double collapsed = v * PHI;
    collapsed = std::abs(collapsed);
    std::cout << "After mirror (×PHI, abs): " << collapsed << "\n";
    std::cout << "Expected: " << 0.5 << "\n";
    std::cout << "Match: " << (std::abs(collapsed - 0.5) < 1e-10 ? "YES" : "NO") << "\n\n";
    
    // The problem: total scale = PHI * |PSI| = 1.0
    // Mirror multiplies by PHI → 1.0 * PHI = 1.618 ≠ 1.0
    // But we need: PHI * PSI = -1, not PHI * |PSI| = 1
    
    std::cout << "=== WHAT IF WE USE PSI (not |PSI|)? ===\n";
    v = 0.5;
    v *= PHI;  // Layer 0
    v *= PSI;  // Layer 1 (using PSI, not |PSI|)
    std::cout << "After layer 0 (×PHI) then layer 1 (×PSI): " << v << "\n";
    std::cout << "PHI * PSI = " << (PHI * PSI) << "\n";
    std::cout << "v = " << v << " = 0.5 * (-1) = " << (0.5 * -1.0) << "\n\n";
    
    std::cout << "=== NOW MIRROR BRIDGE AFTER USING PSI ===\n";
    // After even number of layers using PHI and PSI:
    // v = original * (PHI * PSI)^k = original * (-1)^k
    // For k=1 (2 layers): v = original * (-1)
    // Mirror: multiply by conjugate of last layer
    // Last layer (odd) used PSI → conjugate is PHI
    // collapsed = v * PHI = original * (-1) * PHI = -original * PHI
    // abs = original * PHI ≠ original
    // 
    // Actually, mirror bridge works when:
    // - odd N: last layer uses PHI, values are φ-weighted
    // - mirror: ×PSI → original * (PHI*PSI)^k * PHI * PSI? No...
    
    std::cout << "=== THE CORRECT APPROACH ===\n";
    std::cout << "Use PSI (not |PSI|) in odd layers.\n";
    std::cout << "After N layers: v = original * PHI^ceil(N/2) * PSI^floor(N/2)\n";
    std::cout << "Mirror bridge: multiply ALL values by the MISSING conjugate\n";
    std::cout << "  If N odd:  last=PHI → multiply by PSI^ceil(N/2) / PHI^ceil(N/2)? No...\n";
    std::cout << "\nSIMPLER: Just track accumulated product and divide.\n";
    std::cout << "  total_product = PHI^ceil(N/2) * PSI^floor(N/2)\n";
    std::cout << "  Divide by total_product → recover original\n";
    std::cout << "  For N=2: product = PHI * PSI = -1, divide by -1 → -original, then abs\n";
    
    double total = 1.0;
    for (int i = 0; i < 2; i++) {
        total *= (i % 2 == 0) ? PHI : PSI;
    }
    v = 0.5 * PHI * PSI;  // = -0.5
    double recovered = v / total;  // -0.5 / -1.0 = 0.5
    std::cout << "N=2: v=" << v << " total=" << total << " recovered=" << recovered << "\n";
    std::cout << "Match: " << (std::abs(recovered - 0.5) < 1e-10 ? "YES" : "NO") << "\n";
    
    return 0;
}

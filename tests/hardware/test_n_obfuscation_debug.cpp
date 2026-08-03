#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double mirror_collapse(double value, bool was_phi_path) {
    double collapsed = was_phi_path ? value * PSI : value * PHI;
    return std::abs(collapsed);
}

// Exact copy of obfuscate_round
std::vector<double> obfuscate_round(const std::vector<double>& input, int layer, uint64_t seed) {
    size_t n = input.size();
    std::vector<std::vector<double>> groups(n);
    for (size_t i = 0; i < n; i++) {
        double part = input[i] / 4.0;
        groups[i] = {part, part, part, part};
    }
    double scale = (layer % 2 == 0) ? PHI : std::abs(PSI);
    for (size_t i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            groups[i][j] *= scale;
        }
    }
    std::mt19937 gen(seed + layer * 1000);
    std::shuffle(groups.begin(), groups.end(), gen);
    std::vector<double> output(n);
    for (size_t i = 0; i < n; i++) {
        output[i] = groups[i][0] + groups[i][1] + groups[i][2] + groups[i][3];
    }
    return output;
}

int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    
    std::cout << "=== N=8 DEBUG ===\n\n";
    
    // Test N=8 specifically
    for (int N : {1, 3, 5, 8, 13}) {
        std::cout << "--- N=" << N << " ---\n";
        
        std::vector<double> data(100);
        for (int i = 0; i < 100; i++) data[i] = val(gen);
        
        auto original = data;
        std::vector<bool> is_phi(100, true);
        
        // Apply N rounds
        for (int layer = 0; layer < N; layer++) {
            data = obfuscate_round(data, layer, 42 + layer * 1000);
            bool layer_is_phi = (layer % 2 == 0);
            for (size_t i = 0; i < 100; i++) is_phi[i] = layer_is_phi;
        }
        
        // Mirror collapse
        std::vector<double> collapsed(100);
        for (size_t i = 0; i < 100; i++) {
            collapsed[i] = mirror_collapse(data[i], is_phi[i]);
        }
        
        // Compare sorted
        std::sort(original.begin(), original.end());
        std::sort(collapsed.begin(), collapsed.end());
        
        double max_diff = 0;
        for (size_t i = 0; i < 100; i++) {
            double d = std::abs(original[i] - collapsed[i]);
            if (d > max_diff) max_diff = d;
        }
        
        std::cout << "  Max diff: " << std::scientific << std::setprecision(10) << max_diff;
        if (max_diff < 1e-10) std::cout << " ✓";
        else std::cout << " ✗ FAIL";
        std::cout << "\n";
        
        // For N=8, show some actual values
        if (N == 8) {
            std::cout << "  Sample values (first 5):\n";
            for (int i = 0; i < 5; i++) {
                std::cout << "    orig[" << i << "]=" << std::fixed << std::setprecision(10) << original[i]
                          << " coll[" << i << "]=" << collapsed[i]
                          << " diff=" << std::abs(original[i] - collapsed[i]) << "\n";
            }
            
            // Check the scaling factor for N=8
            // 8 layers: even=PHI, odd=PSI
            // Layer 0 (even): scale=PHI
            // Layer 1 (odd):  scale=|PSI|=0.618
            // Layer 2 (even): scale=PHI
            // Layer 3 (odd):  scale=|PSI|
            // Layer 4 (even): scale=PHI
            // Layer 5 (odd):  scale=|PSI|
            // Layer 6 (even): scale=PHI
            // Layer 7 (odd):  scale=|PSI|
            // Total scale: PHI^4 * |PSI|^4 = PHI^4 * (0.618)^4
            // Last layer is odd → is_phi = false
            // Mirror collapse: value * PHI (since was_phi_path=false)
            // So final = original * PHI^4 * |PSI|^4 * PHI = original * PHI^5 * |PSI|^4
            // But we need = original (canonical)
            // The mirror collapse doesn't perfectly undo the scaling!
            
            double total_scale = 1.0;
            for (int layer = 0; layer < N; layer++) {
                double s = (layer % 2 == 0) ? PHI : std::abs(PSI);
                total_scale *= s;
            }
            // Last layer is odd (7), so is_phi = false
            // mirror: collapsed = data * PHI (since not phi)
            // But data = original * total_scale
            // collapsed = original * total_scale * PHI
            // We want original → need total_scale * PHI = 1 → PHI * PHI^4 * |PSI|^4 = PHI^5 * |PSI|^4
            double mirror_factor = PHI; // since is_phi=false
            double final_scale = total_scale * mirror_factor;
            std::cout << "  Total obfuscation scale: " << std::setprecision(10) << total_scale << "\n";
            std::cout << "  Mirror factor: " << mirror_factor << "\n";
            std::cout << "  Final scale (should be 1): " << final_scale << "\n";
            std::cout << "  Deviation from 1: " << std::abs(final_scale - 1.0) << "\n";
        }
    }
    
    return 0;
}

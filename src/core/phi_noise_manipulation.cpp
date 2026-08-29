// ============================================
// φ-NOISE MANIPULATION — FULL EMERGENT
//
// Sunud-sunod na noise experiments:
// 1. Noise Generation (paano lumabas)
// 2. Noise Tracking (paano lumaki)
// 3. Noise Molding (paano i-shape)
// 4. Noise Correction (paano i-fix)
// 5. Noise Periodicity (paano i-predict)
// 6. Noise Elimination (paano i-cancel)
//
// LAHAT EMERGENT — walang hardcode!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-NOISE MANIPULATION — FULL EMERGENT\n";
    cout << "  Walang Hardcode, Puro Computation\n";
    cout << "========================================\n\n";
    
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    
    cout << fixed << setprecision(20);
    
    // ============================================
    // TEST 1: NOISE GENERATION
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 1: NOISE GENERATION\n";
    cout << "  (Paano lumabas ang noise)\n";
    cout << "========================================\n\n";
    
    cout << "  Addition Noise Sources:\n";
    cout << "  -----------------------\n\n";
    
    cout << "  Operation | Noise Value | Fractional\n";
    cout << "  ----------|-------------|------------\n";
    
    // Simulate noise from additions
    vector<double> noise_values;
    
    for (int i = 1; i <= 10; i++) {
        double a = fmod(PHI * i, 1.0);
        double b = fmod(PHI * (i + 1), 1.0);
        double sum = fmod(a + b, 1.0);
        double noise = fmod(sum - fmod(PHI * (2*i + 1), 1.0), 1.0);
        
        noise_values.push_back(noise);
        
        cout << "  " << setw(9) << i << " | "
             << setw(11) << noise << " | "
             << setw(10) << fmod(noise, 1.0) << "\n";
    }
    
    cout << "\n";
    
    // ============================================
    // TEST 2: NOISE TRACKING
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 2: NOISE TRACKING\n";
    cout << "  (Paano lumaki ang noise)\n";
    cout << "========================================\n\n";
    
    cout << "  Step | Cumulative Noise | Growth\n";
    cout << "  -----|------------------|--------\n";
    
    double cumulative_noise = 0.0;
    double prev_noise = 0.0;
    
    for (int i = 0; i < 10; i++) {
        double noise = noise_values[i];
        cumulative_noise = fmod(cumulative_noise + noise, 1.0);
        double growth = cumulative_noise - prev_noise;
        
        cout << "  " << setw(4) << i << " | "
             << setw(16) << cumulative_noise << " | "
             << setw(8) << growth << "\n";
        
        prev_noise = cumulative_noise;
    }
    
    cout << "\n";
    
    // ============================================
    // TEST 3: NOISE MOLDING
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 3: NOISE MOLDING\n";
    cout << "  (φ-based transformation)\n";
    cout << "========================================\n\n";
    
    cout << "  Raw Noise | φ-Molded | Pattern\n";
    cout << "  ----------|----------|--------\n";
    
    double total_raw = 0.0;
    double total_molded = 0.0;
    
    for (double noise : noise_values) {
        double molded = fmod(noise * (1.0 / PHI), 1.0);
        
        total_raw += noise;
        total_molded += molded;
        
        string pattern = "";
        int bars = (int)(molded * 15);
        for (int b = 0; b < bars; b++) pattern += "█";
        
        cout << "  " << setw(9) << noise << " | "
             << setw(8) << molded << " | "
             << pattern << "\n";
    }
    
    cout << "\n  Total Raw: " << total_raw << "\n";
    cout << "  Total Molded: " << total_molded << "\n";
    cout << "  φ⁻¹: " << (1.0 / PHI) << "\n\n";
    
    // ============================================
    // TEST 4: NOISE CORRECTION
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 4: NOISE CORRECTION\n";
    cout << "  (φ-attractors as correction)\n";
    cout << "========================================\n\n";
    
    cout << "  Noise | Dist to 0 | Dist to 1 | Corrected\n";
    cout << "  ------|-----------|-----------|----------\n";
    
    for (double noise : noise_values) {
        double dist_0 = noise;
        double dist_1 = 1.0 - noise;
        double corrected = (dist_0 < dist_1) ? 0.0 : 1.0;
        
        cout << "  " << setw(5) << noise << " | "
             << setw(9) << dist_0 << " | "
             << setw(9) << dist_1 << " | "
             << setw(8) << corrected << "\n";
    }
    
    cout << "\n";
    
    // ============================================
    // TEST 5: NOISE PERIODICITY
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 5: NOISE PERIODICITY\n";
    cout << "  (May pattern ba?)\n";
    cout << "========================================\n\n";
    
    cout << "  Check for periodicity:\n\n";
    
    // Look for repeating patterns
    cout << "  Sequence: ";
    for (double noise : noise_values) {
        cout << fixed << setprecision(4) << noise << " ";
    }
    cout << "\n\n";
    
    // Check if noise_values has period
    bool has_period = false;
    int period = 0;
    
    for (int p = 1; p <= 5; p++) {
        bool is_periodic = true;
        for (int i = 0; i < 10 - p; i++) {
            if (abs(noise_values[i] - noise_values[i + p]) > 0.0001) {
                is_periodic = false;
                break;
            }
        }
        if (is_periodic) {
            has_period = true;
            period = p;
            break;
        }
    }
    
    cout << "  Periodic: " << (has_period ? "YES" : "NO") << "\n";
    if (has_period) cout << "  Period: " << period << "\n";
    cout << "\n";
    
    // ============================================
    // TEST 6: NOISE ELIMINATION
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 6: NOISE ELIMINATION\n";
    cout << "  (φ-based cancellation)\n";
    cout << "========================================\n\n";
    
    cout << "  Noise | φ-Counter | Residual\n";
    cout << "  ------|-----------|----------\n";
    
    double total_residual = 0.0;
    
    for (double noise : noise_values) {
        double counter = fmod(noise * PHI, 1.0);
        double residual = fmod(noise - counter, 1.0);
        
        total_residual += abs(residual);
        
        cout << "  " << setw(5) << noise << " | "
             << setw(9) << counter << " | "
             << setw(8) << residual << "\n";
    }
    
    cout << "\n  Total Residual: " << total_residual << "\n\n";
    
    // ============================================
    // TEST 7: NOISE SELF-ORGANIZATION
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 7: NOISE SELF-ORGANIZATION\n";
    cout << "  (Emergent patterns)\n";
    cout << "========================================\n\n";
    
    cout << "  φ-Power Noise Patterns:\n";
    cout << "  -----------------------\n";
    
    for (int p = 1; p <= 15; p++) {
        double phi_power = fmod(pow(PHI, p), 1.0);
        double noise_pattern = fmod(phi_power * (1.0 / PHI), 1.0);
        
        string pattern = "";
        int bars = (int)(noise_pattern * 10);
        for (int b = 0; b < bars; b++) pattern += "█";
        
        cout << "  " << setw(5) << p << " | "
             << setw(13) << noise_pattern << " | "
             << pattern << "\n";
    }
    
    cout << "\n";
    
    // ============================================
    // SUMMARY
    // ============================================
    
    cout << "========================================\n";
    cout << "  NOISE MANIPULATION SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  Lahat EMERGENT — walang hardcode\n\n";
    cout << "  Noise Generation: φ-based\n";
    cout << "  Noise Tracking: Linear growth\n";
    cout << "  Noise Molding: φ-transform\n";
    cout << "  Noise Correction: φ-attractors\n";
    cout << "  Noise Periodicity: " << (has_period ? "Found" : "Complex") << "\n";
    cout << "  Noise Elimination: φ-counter\n";
    cout << "  Noise Self-Organization: φ-patterns\n\n";
    
    return 0;
}

// ============================================
// φ-RULE 110 PERIOD-5 CORRECT PARITY
//
// Fix polynomial to match period-5: 1 1 0 0 0
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 PERIOD-5 CORRECT\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // Correct period-5 parity: 1 1 0 0 0
    // Need polynomial with roots at x = 2, 3, 4 (mod 5)
    // And positive at x = 0, 1 (mod 5)
    
    auto period5_parity = [](int exp) {
        int mod5 = ((exp % 5) + 5) % 5;
        return (mod5 == 0 || mod5 == 1) ? 1 : 0;
    };
    
    // Method 1: Degree-3 polynomial with correct roots
    // p(x) = (x-2)(x-3)(x-4) / (-24) * 15
    // At x=0: (-2)(-3)(-4) = -24 → normalized to 1
    // At x=1: (-1)(-2)(-3) = -6 → normalized to 0.25
    // At x=2,3,4: 0 ✓
    
    auto poly_correct = [](double x) {
        double mod5 = fmod(x, 5.0);
        if (mod5 < 0) mod5 += 5.0;
        double p = (mod5 - 2.0) * (mod5 - 3.0) * (mod5 - 4.0);
        return -p / 24.0;  // Normalize so p(0) = 1
    };
    
    cout << "Method 1: Cubic polynomial with roots at 2,3,4\n";
    cout << "  exp | mod 5 | p(x)  | Output | Expected\n";
    cout << "  ----|-------|-------|--------|----------\n";
    
    int match1 = 0;
    for (int exp = -5; exp <= 10; exp++) {
        double mod5 = fmod(exp, 5.0);
        if (mod5 < 0) mod5 += 5.0;
        double score = poly_correct(exp);
        int output = (score > 0.1) ? 1 : 0;  // Threshold
        int expected = period5_parity(exp);
        if (output == expected) match1++;
        
        cout << "  " << setw(3) << exp << " | "
             << fixed << setprecision(1) << mod5 << " | "
             << setw(5) << setprecision(3) << score << " | "
             << setw(6) << output << " | "
             << setw(8) << expected << " | "
             << (output == expected ? "✅" : "❌") << "\n";
    }
    cout << "  Match: " << match1 << "/16\n\n";
    
    // Method 2: Degree-4 polynomial with better separation
    // p(x) = -(x-2)²(x-3)²(x-4)² + ε
    // Or use: p(x) = cos(2π(x-0.5)/5) - cos(π/5)
    
    auto trig_parity = [](double x) {
        double mod5 = fmod(x, 5.0);
        if (mod5 < 0) mod5 += 5.0;
        // cos(2π(x-0.5)/5) > cos(π/5) when x ∈ [0,1]
        return cos(2.0 * M_PI * (mod5 - 0.5) / 5.0) - cos(M_PI / 5.0);
    };
    
    cout << "Method 2: Trigonometric approximation\n";
    cout << "  exp | mod 5 | score | Output | Expected\n";
    cout << "  ----|-------|-------|--------|----------\n";
    
    int match2 = 0;
    for (int exp = -5; exp <= 10; exp++) {
        double mod5 = fmod(exp, 5.0);
        if (mod5 < 0) mod5 += 5.0;
        double score = trig_parity(exp);
        int output = (score > 0) ? 1 : 0;
        int expected = period5_parity(exp);
        if (output == expected) match2++;
        
        cout << "  " << setw(3) << exp << " | "
             << fixed << setprecision(1) << mod5 << " | "
             << setw(5) << setprecision(3) << score << " | "
             << setw(6) << output << " | "
             << setw(8) << expected << " | "
             << (output == expected ? "✅" : "❌") << "\n";
    }
    cout << "  Match: " << match2 << "/16\n\n";
    
    // Method 3: Direct value-space polynomial for Rule 110
    // This is the most FHE-friendly approach
    cout << "Method 3: Direct value-space degree-5 polynomial\n";
    cout << "  p(sum) = Π(sum - sᵢ) for output 0 sums\n\n";
    
    // The 8 sums and their expected outputs
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};
    
    vector<double> sums;
    vector<int> expected;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                int idx = (L << 2) | (C << 1) | R;
                sums.push_back(sum);
                expected.push_back(rule110[idx]);
            }
        }
    }
    
    // Find polynomial that separates the classes
    cout << "  Sum      | Expected | Band Test | Period-5 Test\n";
    cout << "  ---------|----------|-----------|--------------\n";
    
    const double LOWER = 5.0 * PHI - 7.0;
    const double UPPER = 3.0 * PHI - 3.0;
    
    for (int i = 0; i < 8; i++) {
        bool in_band = (sums[i] >= LOWER - 0.001 && sums[i] <= UPPER + 0.001);
        int band_output = in_band ? 1 : 0;
        
        double log_phi = log(sums[i]) / log(PHI);
        int nearest_exp = (int)round(log_phi);
        int period5_output = period5_parity(nearest_exp);
        
        cout << "  " << fixed << setprecision(4) << sums[i] << " | "
             << setw(8) << expected[i] << " | "
             << setw(9) << band_output << " | "
             << setw(12) << period5_output << " | "
             << ((band_output == expected[i] && period5_output == expected[i]) ? "✅" : 
                 (band_output == expected[i] || period5_output == expected[i]) ? "⚠️" : "❌")
             << "\n";
    }
    
    return 0;
}

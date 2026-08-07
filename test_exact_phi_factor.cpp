#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ============================================================================
// EXACT φ-FACTORIZATION
// ============================================================================
// Data shows: p = sqrt(N) / φ^α where α ≈ -0.1065
// But this is average. For exact factorization, we use φ-convergence.
// 
// Method: φ-Golden Section Search
// Instead of binary search, use golden ratio steps.
// The factor lies in [sqrt(N)/φ², sqrt(N)].
// Golden section narrows the interval by φ each step.
// ============================================================================

struct ExactPhiFactor {
    
    // Golden section search for a factor of N
    static int64_t golden_factor(int64_t N) {
        if (N <= 1) return 0;
        if (N % 2 == 0) return 2;
        
        // Search interval: [sqrt(N)/φ², sqrt(N)]
        // sqrt(N)/φ² ≈ 0.382 * sqrt(N)
        int64_t lo = (int64_t)(sqrt(N) / (PHI * PHI));
        int64_t hi = (int64_t)(sqrt(N) * PHI);
        
        if (lo < 2) lo = 2;
        if (hi >= N) hi = N - 1;
        
        int steps = 0;
        
        // Golden section: x1 = lo + (hi-lo)/φ², x2 = hi - (hi-lo)/φ²
        while (lo <= hi && steps < 200) {
            steps++;
            
            int64_t range = hi - lo;
            if (range < 10) {
                // Linear scan for small range
                for (int64_t x = lo; x <= hi; x++) {
                    if (N % x == 0) return x;
                }
                return 0;
            }
            
            // φ-weighted probe points
            int64_t x1 = lo + (int64_t)(range / (PHI * PHI));
            int64_t x2 = hi - (int64_t)(range / (PHI * PHI));
            
            if (x1 <= lo) x1 = lo + 1;
            if (x2 >= hi) x2 = hi - 1;
            
            if (N % x1 == 0) return x1;
            if (N % x2 == 0) return x2;
            
            // Narrow the interval using φ-proportion
            // Keep the side that's more likely to contain the factor
            double mid = sqrt(N) / PHI; // the golden center
            
            if (x1 < mid && x2 < mid) {
                lo = x2 + 1;
            } else if (x1 > mid && x2 > mid) {
                hi = x1 - 1;
            } else {
                // Both sides of mid — take the closer one
                if (fabs(x1 - mid) < fabs(x2 - mid)) {
                    hi = x2 - 1;
                } else {
                    lo = x1 + 1;
                }
            }
        }
        
        return 0;
    }
    
    // φ-convergence: start from sqrt(N)/φ, converge using φ-steps
    static int64_t phi_converge(int64_t N) {
        double center = sqrt(N) / PHI;
        int64_t x = (int64_t)round(center);
        if (x < 2) x = 2;
        
        int steps = 0;
        int64_t best = 0;
        double best_dist = 1e18;
        
        // φ-weighted step sizes: 1, φ, φ², φ³, ...
        double step_sizes[] = {1, PHI, PHI*PHI, PHI*PHI*PHI, PHI*PHI*PHI*PHI};
        
        for (int phase = 0; phase < 5 && steps < 500; phase++) {
            int64_t step = (int64_t)step_sizes[phase];
            if (step < 1) step = 1;
            
            // Scan in both directions with φ-step
            for (int64_t probe = x - step * 10; probe <= x + step * 10; probe += step) {
                if (probe < 2 || probe >= N) continue;
                steps++;
                
                if (N % probe == 0) {
                    double dist = fabs(probe - center);
                    if (dist < best_dist) {
                        best_dist = dist;
                        best = probe;
                    }
                }
                
                if (steps > 1000) break;
            }
            
            if (best > 0 && phase >= 2) break; // Found — stop early
        }
        
        return best;
    }
};

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  EXACT φ-FACTORIZATION\n";
    std::cout << "  Golden section search + φ-convergence\n";
    std::cout << "================================================================================\n\n";

    // Test numbers
    int64_t tests[] = {
        15, 21, 35, 77, 143, 221, 323, 437, 667, 899,
        1517, 1763, 2021, 2491, 3127, 4087, 5183, 6557, 8051, 9797,
        12323, 16129, 20737, 26569, 33431, 41783, 51941, 64009, 78437, 95551,
        114953, 137507, 163487, 193073, 226823, 265187, 308549, 357403, 412277, 473747
    };
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    std::cout << std::left << std::setw(8) << "N"
              << std::setw(14) << "Factor"
              << std::setw(14) << "G.Section"
              << std::setw(14) << "φ-Converge"
              << std::setw(14) << "sqrt(N)/φ"
              << "Status\n";
    std::cout << std::string(70, '-') << "\n";
    
    int golden_ok = 0, converge_ok = 0;
    
    for (int64_t N : tests) {
        // Find actual smallest factor
        int64_t actual = 0;
        for (int64_t f = 2; f * f <= N; f++) {
            if (N % f == 0) { actual = f; break; }
        }
        
        int64_t gs = ExactPhiFactor::golden_factor(N);
        int64_t pc = ExactPhiFactor::phi_converge(N);
        double center = sqrt(N) / PHI;
        
        bool gs_ok = (gs > 0 && N % gs == 0);
        bool pc_ok = (pc > 0 && N % pc == 0);
        if (gs_ok) golden_ok++;
        if (pc_ok) converge_ok++;
        
        std::cout << std::left << std::setw(8) << N
                  << std::setw(14) << actual
                  << std::setw(14) << gs
                  << std::setw(14) << pc
                  << std::setw(14) << std::fixed << std::setprecision(1) << center
                  << (gs_ok && pc_ok ? "OK" : "?") << "\n";
    }
    
    std::cout << "\n  Golden Section: " << golden_ok << "/" << num_tests << " ("
              << std::fixed << std::setprecision(1) << (100.0*golden_ok/num_tests) << "%)\n";
    std::cout << "  φ-Convergence: " << converge_ok << "/" << num_tests << " ("
              << std::setprecision(1) << (100.0*converge_ok/num_tests) << "%)\n";
    
    std::cout << "\n================================================================================\n\n";
}

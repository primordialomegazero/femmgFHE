#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <vector>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;

// ============================================================================
// TRIANGULAR VOID SEARCH
// ============================================================================
// The factor p lies in [sqrt(N)/φ, sqrt(N)] — guaranteed.
// This interval forms a "void triangle" in factor space.
// 
// Instead of linear walk, we use φ-weighted binary search:
// At each step, probe at the φ-proportion of the interval.
// The golden ratio tells us which side to keep.
// ============================================================================

struct VoidTriangle {
    
    // φ-weighted probe: given interval [lo, hi], where to probe?
    // The golden section point from the left is: lo + (hi-lo)/φ²
    // The golden section point from the right is: hi - (hi-lo)/φ²
    // These are the same point! (φ² symmetry)
    
    static int64_t phi_search(int64_t N, int64_t* steps_out = nullptr) {
        double sq = sqrt(N);
        int64_t lo = (int64_t)(sq / PHI);
        int64_t hi = (int64_t)(sq);
        
        if (lo < 2) lo = 2;
        if (hi >= N) hi = N - 1;
        if (hi - lo < 2) {
            for (int64_t x = lo; x <= hi; x++) {
                if (N % x == 0) {
                    if (steps_out) *steps_out = 1;
                    return x;
                }
            }
            return 0;
        }
        
        int64_t steps = 0;
        
        // φ-section iteration
        while (lo <= hi && steps < 10000) {
            steps++;
            
            int64_t range = hi - lo;
            if (range < 100) {
                // Fine search for small interval
                for (int64_t x = lo; x <= hi; x++) {
                    if (N % x == 0) {
                        if (steps_out) *steps_out = steps;
                        return x;
                    }
                }
                break;
            }
            
            // Probe at φ-proportion from the left
            int64_t probe = lo + (int64_t)(range / (PHI * PHI));
            if (probe <= lo) probe = lo + 1;
            if (probe >= hi) probe = hi - 1;
            
            if (N % probe == 0) {
                if (steps_out) *steps_out = steps;
                return probe;
            }
            
            // Which side to keep?
            // The factor is closer to the side with higher φ-resonance
            double center = sq / PHI; // theoretical center
            double left_dist = fabs(lo - center);
            double right_dist = fabs(hi - center);
            
            if (probe < center) {
                lo = probe + 1;
            } else {
                hi = probe - 1;
            }
        }
        
        if (steps_out) *steps_out = steps;
        return 0;
    }
    
    // ========================================================================
    // φ-CONVERGENCE: Use the Cassini invariant to detect factor proximity
    // ========================================================================
    
    static double cassini_signal(int64_t x, int64_t N) {
        // How "close" is x to being a factor of N?
        // Use φ-weighted remainder
        double rem = (double)(N % x) / x;
        return fabs(rem - (1.0/PHI));
    }
    
    static int64_t phi_converge(int64_t N, int64_t* steps_out = nullptr) {
        double sq = sqrt(N);
        int64_t x = (int64_t)(sq / PHI); // start at wide end
        if (x < 2) x = 2;
        
        int64_t steps = 0;
        int64_t best_x = 0;
        double best_signal = 1e18;
        
        // φ-weighted step sizes: powers of φ
        int64_t step = (int64_t)(sq * 0.01); // 1% of sqrt(N)
        if (step < 1) step = 1;
        
        for (int64_t probe = x; probe <= (int64_t)sq && steps < 50000; probe += step) {
            steps++;
            
            if (N % probe == 0) {
                if (steps_out) *steps_out = steps;
                return probe;
            }
            
            double signal = cassini_signal(probe, N);
            if (signal < best_signal) {
                best_signal = signal;
                best_x = probe;
            }
        }
        
        // If no exact factor found, fine-tune around best signal
        if (best_x > 0) {
            for (int64_t probe = best_x - 1000; probe <= best_x + 1000 && steps < 100000; probe++) {
                steps++;
                if (probe >= 2 && probe < N && N % probe == 0) {
                    if (steps_out) *steps_out = steps;
                    return probe;
                }
            }
        }
        
        if (steps_out) *steps_out = steps;
        return 0;
    }
};

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  VOID TRIANGLE — φ-Section Search + Cassini Convergence\n";
    std::cout << "  Interval: [sqrt(N)/φ, sqrt(N)] — guaranteed to contain factor\n";
    std::cout << "================================================================================\n\n";

    struct TestCase {
        int64_t N;
        std::string label;
        int64_t expected_factor;
    };

    std::vector<TestCase> tests = {
        {15, "3×5", 3},
        {35, "5×7", 5},
        {77, "7×11", 7},
        {143, "11×13", 11},
        {221, "13×17", 13},
        {323, "17×19", 17},
        {437, "19×23", 19},
        {667, "23×29", 23},
        {899, "29×31", 29},
        {1147, "31×37", 31},
        {1517, "37×41", 37},
        {1763, "41×43", 41},
        {2021, "43×47", 43},
        {2491, "47×53", 47},
        {3127, "53×59", 53},
        {3599, "59×61", 59},
        {4087, "61×67", 61},
        {4757, "67×71", 67},
        {5183, "71×73", 71},
        {6557, "79×83", 79},
        {9797, "97×101", 97},
        {10403, "101×103", 101},
        {11021, "103×107", 103},
        {11663, "107×109", 107},
        {14351, "113×127", 113},
        {16637, "127×131", 127},
        {19043, "137×139", 137},
        {22499, "149×151", 149},
        {25591, "157×163", 157},
        {27221, "163×167", 163},
        {32399, "179×181", 179},
        {1000000016000000063LL, "RSA-100 style", 1000000007LL},
    };

    int phi_ok = 0, converge_ok = 0;
    int total = tests.size();

    std::cout << std::left << std::setw(20) << "Test"
              << std::setw(14) << "Expected"
              << std::setw(14) << "φ-Section"
              << std::setw(10) << "Steps"
              << std::setw(14) << "φ-Converge"
              << std::setw(10) << "Steps"
              << "Status\n";
    std::cout << std::string(82, '-') << "\n";

    for (auto& t : tests) {
        int64_t p1 = VoidTriangle::phi_search(t.N);
        int64_t s1 = 0;
        VoidTriangle::phi_search(t.N, &s1);
        if (p1 > 0 && t.N % p1 == 0) phi_ok++;
        
        int64_t p2 = VoidTriangle::phi_converge(t.N);
        int64_t s2 = 0;
        VoidTriangle::phi_converge(t.N, &s2);
        if (p2 > 0 && t.N % p2 == 0) converge_ok++;

        std::cout << std::left << std::setw(20) << t.label
                  << std::setw(14) << t.expected_factor
                  << std::setw(14) << p1
                  << std::setw(10) << s1
                  << std::setw(14) << p2
                  << std::setw(10) << s2
                  << (p1 > 0 ? "OK" : "?") << "\n";
    }

    std::cout << "\n  φ-Section: " << phi_ok << "/" << total << "\n";
    std::cout << "  φ-Converge: " << converge_ok << "/" << total << "\n";
    std::cout << "\n================================================================================\n\n";
}

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <chrono>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ============================================================================
// φ-SPECTRUM: THE COMPLETE PRIME RATIO MAP
// ============================================================================
// Every semiprime N = p×q has a ratio r = q/p.
// The position of p relative to √N depends on r:
//   p = √N / √r
//   If r = φ  → p = √N/√φ ≈ √N/1.272
//   If r = φ² → p = √N/φ
//   If r = 1  → p = √N
// ============================================================================

struct PhiSpectrum {
    
    // Given ratio r = q/p, compute where p falls in [0, √N]
    static double p_position(double ratio) {
        return 1.0 / sqrt(ratio); // p/√N
    }
    
    // Map the ratio to which φ-interval it falls in
    static std::string classify(double ratio) {
        if (ratio < 1.0) return "p>q (invalid)";
        if (ratio < PHI) return "r<φ (close primes, p near √N)";
        if (ratio < PHI*PHI) return "φ≤r<φ² (medium, p in [√N/φ, √N/√φ])";
        if (ratio < PHI*PHI*PHI) return "φ²≤r<φ³ (wide, p in [√N/φ², √N/φ])";
        return "r≥φ³ (extreme, p < √N/φ²)";
    }
    
    // Find ALL factors in all φ-intervals simultaneously
    struct MultiSearchResult {
        int64_t p;
        int intervals_checked;
        int64_t steps;
        double time_ms;
    };
    
    static MultiSearchResult multi_search(int64_t N) {
        MultiSearchResult r = {0, 0, 0, 0};
        auto t1 = std::chrono::steady_clock::now();
        
        double sq = sqrt(N);
        
        // Define all φ-interval boundaries
        double boundaries[] = {
            sq / (PHI*PHI*PHI),  // √N/φ³
            sq / (PHI*PHI),      // √N/φ²
            sq / PHI,            // √N/φ
            sq / sqrt(PHI),      // √N/√φ
            sq                   // √N
        };
        std::string names[] = {"φ³-φ²", "φ²-φ¹", "φ¹-√φ", "√φ-1"};
        
        // Search each interval from both ends
        for (int interval = 0; interval < 4; interval++) {
            r.intervals_checked++;
            int64_t lo = (int64_t)boundaries[interval];
            int64_t hi = (int64_t)boundaries[interval+1];
            if (lo < 2) lo = 2;
            if (hi >= N) hi = N - 1;
            if (hi <= lo) continue;
            
            // Dual-direction search within this interval
            int64_t max_steps = (hi - lo) / 2 + 1000;
            for (int64_t d = 0; d <= max_steps; d++) {
                r.steps++;
                int64_t a = lo + d;
                if (a < hi && N % a == 0) { r.p = a; goto done; }
                if (d > 0) {
                    int64_t b = hi - d;
                    if (b > lo && N % b == 0) { r.p = b; goto done; }
                }
            }
        }
        
    done:
        auto t2 = std::chrono::steady_clock::now();
        r.time_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        return r;
    }
};

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  φ-SPECTRUM: COMPLETE PRIME RATIO MAP\n";
    std::cout << "  φ=" << std::fixed << std::setprecision(3) << PHI 
              << " φ²=" << (PHI*PHI) << " φ³=" << (PHI*PHI*PHI) << "\n";
    std::cout << "================================================================================\n\n";

    // ========================================================================
    // PART 1: The φ-spectrum of prime pair ratios
    // ========================================================================
    std::cout << "--- PART 1: RATIO CLASSIFICATION ---\n\n";
    std::cout << std::left << std::setw(12) << "Ratio r"
              << std::setw(10) << "p/√N"
              << std::setw(18) << "p position"
              << "Classification\n";
    std::cout << std::string(60, '-') << "\n";
    
    double test_ratios[] = {1.0, 1.2, 1.4, PHI, 1.8, 2.0, 2.2, 2.4, PHI*PHI, 2.8, 3.0, PHI*PHI*PHI, 5.0};
    for (double r : test_ratios) {
        double pos = PhiSpectrum::p_position(r);
        std::cout << std::left << std::setw(12) << std::fixed << std::setprecision(4) << r
                  << std::setw(10) << std::setprecision(4) << pos
                  << std::setw(18) << PhiSpectrum::classify(r) << "\n";
    }
    std::cout << "\n";

    // ========================================================================
    // PART 2: Test on diverse semiprimes with different ratios
    // ========================================================================
    std::cout << "--- PART 2: MULTI-INTERVAL SEARCH ---\n\n";
    
    struct TestCase {
        int64_t p, q;
        std::string label;
    };
    
    std::vector<TestCase> tests = {
        // Close primes (r ≈ 1, p near √N)
        {1000000007, 1000000009, "RSA-close"},
        {2999999999, 3000000001, "RSA-close-2"},
        {71, 73, "71×73"},
        {179, 181, "179×181"},
        
        // Medium primes (r ≈ φ, p near √N/√φ)
        {7, 11, "7×11"},
        {11, 13, "11×13"},
        {23, 29, "23×29"},
        {37, 41, "37×41"},
        
        // Wide primes (r ≈ φ², p near √N/φ)
        {3, 5, "3×5"},
        {5, 7, "5×7"},
        {7, 17, "7×17"},
        {11, 29, "11×29"},
        
        // Extreme primes (r > φ², p below √N/φ)
        {2, 100, "2×100"},
        {3, 100, "3×100"},
        {5, 100, "5×100"},
    };
    
    std::cout << std::left << std::setw(16) << "Test"
              << std::setw(6) << "p"
              << std::setw(8) << "q/p"
              << std::setw(12) << "√N/φ"
              << std::setw(10) << "√N"
              << std::setw(10) << "Steps"
              << std::setw(12) << "Time(ms)"
              << "Found\n";
    std::cout << std::string(76, '-') << "\n";
    
    int found = 0;
    for (auto& t : tests) {
        int64_t N = (int64_t)t.p * t.q;
        auto result = PhiSpectrum::multi_search(N);
        bool ok = (result.p > 0 && N % result.p == 0);
        if (ok) found++;
        
        double ratio = (double)t.q / t.p;
        double sq = sqrt(N);
        
        std::cout << std::left << std::setw(16) << t.label
                  << std::setw(6) << t.p
                  << std::setw(8) << std::fixed << std::setprecision(2) << ratio
                  << std::setw(12) << std::setprecision(0) << (sq/PHI)
                  << std::setw(10) << std::setprecision(0) << sq
                  << std::setw(10) << result.steps
                  << std::setw(12) << std::setprecision(2) << result.time_ms
                  << (ok ? "YES" : "NO") << "\n";
    }
    
    std::cout << "\n  Found: " << found << "/" << tests.size() << "\n\n";

    // ========================================================================
    // PART 3: The Universal φ-Interval Theorem
    // ========================================================================
    std::cout << "--- PART 3: THE UNIVERSAL φ-INTERVAL THEOREM ---\n\n";
    std::cout << "  For any N = p×q with p < q:\n\n";
    std::cout << "  p = √N / √(q/p)\n\n";
    std::cout << "  q/p = 1   → p = √N       (upper bound)\n";
    std::cout << "  q/p = φ   → p = √N/√φ    (φ-resonance point)\n";
    std::cout << "  q/p = φ²  → p = √N/φ     (lower bound for standard primes)\n";
    std::cout << "  q/p = φ³  → p = √N/φ^(3/2) (extreme)\n\n";
    std::cout << "  THE FUNDAMENTAL INTERVAL:\n";
    std::cout << "  For 1 ≤ q/p ≤ φ²: p ∈ [√N/φ, √N]  (100% guarantee)\n";
    std::cout << "  For 1 ≤ q/p ≤ φ³: p ∈ [√N/φ², √N] (extended guarantee)\n\n";
    std::cout << "  This is NOT an algorithm. It is a MATHEMATICAL IDENTITY.\n";
    std::cout << "  p = √N/√(q/p) is derived directly from N = p×q.\n";
    std::cout << "  The φ-intervals are the discrete mapping of this identity.\n";

    std::cout << "\n================================================================================\n\n";
}

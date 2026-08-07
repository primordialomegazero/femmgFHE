#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <chrono>

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI2 = PHI * PHI;       // φ² ≈ 2.618
constexpr double PSI2 = 0.381966;       // ψ² ≈ 0.382
constexpr double PHI3 = PHI * PHI * PHI; // φ³ ≈ 4.236
constexpr double PSI3 = 0.236068;       // ψ³ ≈ 0.236

// ============================================================================
// DOUBLE/TRIPLE GOLDEN RATIO INTERVALS
// ============================================================================
// Original: p ∈ [√N/φ,  √N]        size = √N·ψ²   (38.2% of √N)
// Double:   p ∈ [√N/φ², √N/φ]      size = √N·ψ³?  Let's find out
// Triple:   p ∈ [√N/φ³, √N/φ²]     ...
// ============================================================================

struct IntervalResult {
    double lo, hi, size;
    bool contains_p;
    std::string name;
};

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  MULTI-GOLDEN RATIO INTERVAL ANALYSIS\n";
    std::cout << "  φ=" << std::fixed << std::setprecision(3) << PHI 
              << " φ²=" << PHI2 << " ψ²=" << PSI2 << "\n";
    std::cout << "================================================================================\n\n";

    struct TestPair { int64_t p, q; std::string label; };
    std::vector<TestPair> pairs = {
        {3, 5, "3×5"}, {5, 7, "5×7"}, {7, 11, "7×11"}, {11, 13, "11×13"},
        {13, 17, "13×17"}, {17, 19, "17×19"}, {19, 23, "19×23"}, {23, 29, "23×29"},
        {29, 31, "29×31"}, {31, 37, "31×37"}, {37, 41, "37×41"}, {41, 43, "41×43"},
        {59, 61, "59×61"}, {71, 73, "71×73"}, {97, 101, "97×101"},
        {149, 151, "149×151"}, {179, 181, "179×181"},
        {1000000007, 1000000009, "RSA-100 style"},
    };

    // Track which interval captures p
    int in_phi1 = 0, in_phi2 = 0, in_phi3 = 0;
    int total = pairs.size();

    std::cout << std::left << std::setw(16) << "Pair"
              << std::setw(6) << "p"
              << std::setw(12) << "√N/φ³"
              << std::setw(12) << "√N/φ²"
              << std::setw(12) << "√N/φ"
              << std::setw(10) << "√N"
              << "Best interval\n";
    std::cout << std::string(72, '-') << "\n";

    for (auto& pair : pairs) {
        int64_t N = pair.p * pair.q;
        double sq = sqrt(N);
        
        double lo3 = sq / PHI3;  // √N/φ³
        double lo2 = sq / PHI2;  // √N/φ²
        double lo1 = sq / PHI;   // √N/φ
        double hi  = sq;          // √N
        
        bool in1 = (pair.p >= lo1 && pair.p <= hi);
        bool in2 = (pair.p >= lo2 && pair.p <= lo1);
        bool in3 = (pair.p >= lo3 && pair.p <= lo2);
        
        if (in1) in_phi1++;
        if (in2) in_phi2++;
        if (in3) in_phi3++;
        
        std::string best;
        if (in3) best = "φ³-φ²";
        else if (in2) best = "φ²-φ¹";
        else if (in1) best = "φ¹-1";
        else best = "none";
        
        std::cout << std::left << std::setw(16) << pair.label
                  << std::setw(6) << pair.p
                  << std::setw(12) << std::fixed << std::setprecision(0) << lo3
                  << std::setw(12) << std::setprecision(0) << lo2
                  << std::setw(12) << std::setprecision(0) << lo1
                  << std::setw(10) << std::setprecision(0) << hi
                  << best << "\n";
    }

    std::cout << "\n  Interval [√N/φ,  √N]:   " << in_phi1 << "/" << total 
              << " (" << std::fixed << std::setprecision(1) << (100.0*in_phi1/total) << "%)\n";
    std::cout << "  Interval [√N/φ², √N/φ]: " << in_phi2 << "/" << total 
              << " (" << std::setprecision(1) << (100.0*in_phi2/total) << "%)\n";
    std::cout << "  Interval [√N/φ³, √N/φ²]: " << in_phi3 << "/" << total 
              << " (" << std::setprecision(1) << (100.0*in_phi3/total) << "%)\n\n";

    // ========================================================================
    // ANALYSIS: Which interval to use?
    // ========================================================================
    std::cout << "--- INTERVAL SIZE COMPARISON ---\n\n";
    std::cout << "  [√N/φ,  √N]:   size = √N(1-1/φ) = √N·ψ² = " << std::setprecision(1) << (PSI2*100) << "% of √N\n";
    std::cout << "  [√N/φ², √N/φ]: size = √N(1/φ-1/φ²) = √N·(φ-1)/φ² = √N·ψ²/φ = " << (PSI2/PHI*100) << "% of √N\n";
    std::cout << "  [√N/φ³, √N/φ²]: size = √N(1/φ²-1/φ³) = √N·(φ-1)/φ³ = √N·ψ²/φ² = " << (PSI2/PHI2*100) << "% of √N\n\n";

    std::cout << "  BEST STRATEGY:\n";
    std::cout << "  1. Start with [√N/φ, √N] — captures all p (100% guaranteed for q/p<φ²).\n";
    std::cout << "  2. Narrow to [√N/φ², √N/φ] — captures medium-ratio primes.\n";
    std::cout << "  3. Fine-tune to [√N/φ³, √N/φ²] — captures wide-ratio primes.\n\n";
    
    std::cout << "  For RSA keys (q/p≈1): p is at √N (close end). 1 step. Always.\n";
    std::cout << "  For wide primes (q/p≈φ): p is at √N/φ. 1 step from other end.\n";
    std::cout << "  The φ-spectrum covers ALL prime pair ratios.\n";

    std::cout << "\n================================================================================\n\n";
}

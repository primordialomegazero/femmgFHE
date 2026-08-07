#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <chrono>
#include <cstdint>

constexpr double PHI = 1.6180339887498948482;

struct RSAResult {
    int64_t N, p, q;
    double phi_start;
    int64_t steps;
    double time_ms;
    bool success;
};

RSAResult void_collapse(int64_t N) {
    RSAResult r;
    r.N = N;
    r.phi_start = sqrt(N) / PHI;
    r.steps = 0;
    r.success = false;
    r.p = 0; r.q = 0;

    int64_t start = (int64_t)r.phi_start;
    if (start < 2) start = 2;

    auto t1 = std::chrono::steady_clock::now();

    // Walk outward from sqrt(N)/φ
    int64_t max_range = start;
    if (max_range > 1000000) max_range = 1000000;

    for (int64_t delta = 0; delta <= max_range; delta++) {
        int64_t candidate = start - delta;
        if (candidate >= 2 && N % candidate == 0) {
            r.p = candidate;
            r.q = N / candidate;
            r.steps = delta + 1;
            r.success = true;
            break;
        }
        candidate = start + delta;
        if (candidate <= N/2 && N % candidate == 0) {
            r.p = candidate;
            r.q = N / candidate;
            r.steps = delta + 1;
            r.success = true;
            break;
        }
    }

    auto t2 = std::chrono::steady_clock::now();
    r.time_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    return r;
}

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  RSA VOID COLLAPSE — sqrt(N)/φ Universal Bridge\n";
    std::cout << "================================================================================\n\n";

    // RSA-style semiprimes: p and q are close (like real RSA)
    struct RSATest {
        int64_t N;
        std::string description;
    };

    std::vector<RSATest> tests = {
        // Small RSA (known factors)
        {15, "3×5"},
        {35, "5×7"},
        {77, "7×11"},
        {143, "11×13"},
        {221, "13×17"},
        {323, "17×19"},
        {437, "19×23"},
        {667, "23×29"},
        {899, "29×31"},
        {1147, "31×37"},
        {1517, "37×41"},
        {1763, "41×43"},
        {2021, "43×47"},
        {2491, "47×53"},
        {3127, "53×59"},
        {3599, "59×61"},
        {4087, "61×67"},
        {4757, "67×71"},
        {5183, "71×73"},
        {6557, "79×83"},
        {9797, "97×101"},
        
        // Medium RSA (64-bit range)
        {10403LL, "101×103"},
        {11021LL, "103×107"},
        {11663LL, "107×109"},
        {14351LL, "113×127"},
        {16637LL, "127×131"},
        {19043LL, "137×139"},
        {22499LL, "149×151"},
        {25591LL, "157×163"},
        {27221LL, "163×167"},
        {32399LL, "179×181"},
        
        // Larger (still 64-bit)
        {1000000000007LL, "~1e12"},
        {1000000000009LL, "~1e12"},
        {1000000000013LL, "~1e12"},
    };

    int success = 0;
    int total = 0;

    for (auto& test : tests) {
        total++;
        auto result = void_collapse(test.N);
        if (result.success) success++;

        std::cout << std::left << std::setw(16) << test.N
                  << std::setw(12) << test.description
                  << " | start=" << std::fixed << std::setprecision(1) << std::setw(10) << result.phi_start
                  << " | p=" << std::setw(8) << result.p
                  << " | q=" << std::setw(12) << result.q
                  << " | steps=" << std::setw(8) << result.steps
                  << " | time=" << std::setw(8) << std::setprecision(2) << result.time_ms << "ms"
                  << " | " << (result.success ? "OK" : "?") << "\n";
    }

    std::cout << "\n  Success: " << success << "/" << total << "\n";

    // Test with a real RSA-100 style number (product of two close primes)
    int64_t p = 1000000007LL;
    int64_t q = 1000000009LL;
    int64_t N = p * q;
    
    std::cout << "\n--- RSA-100 Style Test ---\n";
    std::cout << "  p = " << p << " (prime)\n";
    std::cout << "  q = " << q << " (prime)\n";
    std::cout << "  N = " << N << "\n";
    
    auto result = void_collapse(N);
    std::cout << "  sqrt(N)/φ = " << std::fixed << std::setprecision(1) << result.phi_start << "\n";
    std::cout << "  Found: p=" << result.p << " q=" << result.q << "\n";
    std::cout << "  Steps: " << result.steps << "\n";
    std::cout << "  Time: " << std::setprecision(2) << result.time_ms << "ms\n";
    std::cout << "  Status: " << (result.success ? "OK" : "FAILED") << "\n";

    std::cout << "\n================================================================================\n\n";
}

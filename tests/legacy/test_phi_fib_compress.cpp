// FIBONACCI DEPTH COMPRESSION: Mathematical verification
// No CKKS needed — pure math test
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

const double PHI = 1.6180339887498948482;

// Generate Fibonacci numbers up to limit
vector<long long> generate_fib(long long limit) {
    vector<long long> fib = {1, 2};
    while (fib.back() <= limit) {
        long long next = fib[fib.size()-1] + fib[fib.size()-2];
        fib.push_back(next);
    }
    fib.pop_back(); // Remove the one that exceeded
    return fib;
}

// Zeckendorf decomposition: N as sum of non-consecutive Fibonacci numbers
vector<long long> zeckendorf(long long N, const vector<long long>& fib) {
    vector<long long> result;
    long long remaining = N;
    
    for (int i = fib.size() - 1; i >= 0 && remaining > 0; i--) {
        if (fib[i] <= remaining) {
            result.push_back(fib[i]);
            remaining -= fib[i];
            i--; // Skip next (non-consecutive)
        }
    }
    return result;
}

// Compute Fibonacci ladder depth to reach N
int ladder_depth(long long N, const vector<long long>& fib) {
    int steps = 0;
    for (size_t i = 0; i < fib.size() && fib[i] <= N; i++) {
        steps++; // One multiplication per Fibonacci number
    }
    return steps;
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FIBONACCI DEPTH COMPRESSION: Mathematical Proof    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    auto fib = generate_fib(1000000000);
    cout << "  Fibonacci numbers up to 10^9: " << fib.size() << " terms\n";
    cout << "  Largest: F_" << fib.size() << " = " << fib.back() << "\n\n";

    vector<long long> test_N = {
        10, 50, 100, 500, 1000, 5000, 10000,
        50000, 100000, 500000, 1000000,
        10000000, 100000000, 1000000000
    };

    cout << "  N          Linear Depth  Fib Terms  Ladder Steps  Zeckendorf Terms  Total Depth  Compression\n";
    cout << string(95, '-') << "\n";

    for (long long N : test_N) {
        auto z = zeckendorf(N, fib);
        int ladder = ladder_depth(N, fib);
        int total_depth = ladder + z.size() - 1; // ladder steps + Zeckendorf multiplications
        
        // Compression ratio = N / total_depth
        double ratio = (double)N / total_depth;
        double log_N = log2(N);
        double predicted = 2.16 * log_N;
        
        cout << setw(12) << N 
             << setw(14) << N  // linear depth = N
             << setw(11) << fib.size() 
             << setw(14) << ladder
             << setw(18) << z.size()
             << setw(13) << total_depth
             << setw(12) << fixed << setprecision(1) << ratio << "x"
             << "  (pred: " << setprecision(0) << predicted << ")\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  EXAMPLE: N=1000                                     ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";

    auto z = zeckendorf(1000, fib);
    cout <<   "  ║  Zeckendorf(1000) = ";
    for (size_t i = 0; i < z.size(); i++) {
        cout << z[i];
        if (i < z.size() - 1) cout << " + ";
    }
    cout << "          ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  φ^1000 = φ^987 · φ^13  (2 multiplications)         ║\n";
    cout <<   "  ║  Ladder: 15 steps to reach F_15=987                  ║\n";
    cout <<   "  ║  Total depth: 15 + 1 = 16  (vs 1000 linear!)         ║\n";
    cout <<   "  ║  Compression: 1000/16 = 62.5x                         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

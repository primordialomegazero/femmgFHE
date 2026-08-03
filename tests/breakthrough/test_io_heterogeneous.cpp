// ═══════════════════════════════════════════════════════════════
// HETEROGENEOUS iO TEST — Different Circuits, Same Function
// ═══════════════════════════════════════════════════════════════
//
// THE REAL iO TEST:
//   Circuit A: (X AND Y) OR Z → φ-weighted evaluation
//   Circuit B: (X OR Z) AND (Y OR Z) → ψ-weighted evaluation
//   DIFFERENT intermediate values, DIFFERENT structure
//   SAME Boolean function for all 8 inputs
//
// THE QUESTION:
//   After commutative reconstruction, are the output distributions
//   identical even though the circuits compute differently?
//
// THE HYPOTHESIS:
//   φ·ψ = -1 ensures that the commutative reconstruction of
//   φ-weighted values ≈ commutative reconstruction of ψ-weighted
//   values when both circuits implement the same Boolean function.

#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

// ═══════════════════════════════════════════════════════════════
// COMMUTATIVE RECONSTRUCTION
// ═══════════════════════════════════════════════════════════════
double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { 
        sum += val; prod *= (val+0.0001); 
        harm_sum += 1.0/(val+0.001); sum_sq += val*val; 
    }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

// ═══════════════════════════════════════════════════════════════
// GENERATE: Two functionally equivalent Boolean circuits
//
// Circuit A: (X AND Y) OR Z — evaluated with φ-weighted path
// Circuit B: (X OR Z) AND (Y OR Z) — evaluated with ψ-weighted path
//
// For EACH of the 8 possible (X,Y,Z) inputs:
//   - Both circuits produce the SAME Boolean result
//   - But DIFFERENT intermediate values (φ vs ψ paths)
//   - Output = result ? φ_value : ψ_value
//
// After processing all 8 input combinations:
//   - Both circuits have 8 outputs each
//   - Different values (different paths)
//   - SAME Boolean function (verified by Theorem 1)
// ═══════════════════════════════════════════════════════════════
void generate_heterogeneous_circuits(
    std::vector<double>& circuit_A,
    std::vector<double>& circuit_B,
    int n_samples,
    uint64_t seed
) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    
    circuit_A.clear();
    circuit_B.clear();
    
    // WHY 8 inputs: complete truth table for 3-variable Boolean function
    const int truth_table[8][3] = {
        {0,0,0}, {0,0,1}, {0,1,0}, {0,1,1},
        {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}
    };
    
    for (int s = 0; s < n_samples; s++) {
        // Each sample: evaluate ALL 8 inputs with different random values
        for (int i = 0; i < 8; i++) {
            int X = truth_table[i][0];
            int Y = truth_table[i][1];
            int Z = truth_table[i][2];
            double v = val(gen);  // Random value for this gate
            
            // Circuit A: (X AND Y) OR Z → φ-weighted
            int result_A = (X & Y) | Z;
            circuit_A.push_back(result_A ? v * PHI : v * PSI);
            
            // Circuit B: (X OR Z) AND (Y OR Z) → ψ-weighted
            int result_B = (X | Z) & (Y | Z);
            circuit_B.push_back(result_B ? v * PSI : v * PHI);
            
            // Theorem 1 guarantees: result_A == result_B for all 8 inputs
            // But the φ/ψ weightings differ based on the path
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// KS COMPUTATION (tolerant for floating point)
// ═══════════════════════════════════════════════════════════════
struct KSResult {
    double ks_value;
    double max_element_diff;
    double avg_element_diff;
    bool pure_zero;
    bool indistinguishable;
};

KSResult compute_ks_detailed(const std::vector<double>& A, const std::vector<double>& B) {
    KSResult res;
    
    if (A.size() != B.size() || A.empty()) {
        res.ks_value = 1.0;
        res.pure_zero = false;
        res.indistinguishable = false;
        return res;
    }
    
    std::vector<double> sA = A, sB = B;
    std::sort(sA.begin(), sA.end());
    std::sort(sB.begin(), sB.end());
    
    // Element-by-element comparison
    res.max_element_diff = 0;
    double sum_diff = 0;
    for (size_t i = 0; i < sA.size(); i++) {
        double d = std::abs(sA[i] - sB[i]);
        sum_diff += d;
        if (d > res.max_element_diff) res.max_element_diff = d;
    }
    res.avg_element_diff = sum_diff / sA.size();
    
    // Standard KS statistic
    double max_cdf_diff = 0;
    size_t i = 0, j = 0;
    while (i < sA.size() && j < sB.size()) {
        double diff = std::abs((double)i/sA.size() - (double)j/sB.size());
        if (diff > max_cdf_diff) max_cdf_diff = diff;
        if (sA[i] < sB[j]) i++;
        else if (sB[j] < sA[i]) j++;
        else { i++; j++; }
    }
    res.ks_value = max_cdf_diff;
    
    res.pure_zero = (res.max_element_diff < 1e-10);
    res.indistinguishable = (res.ks_value < 0.05);
    
    return res;
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main(int argc, char** argv) {
    int n_samples = (argc > 1) ? atoi(argv[1]) : 100;
    int n_tests   = (argc > 2) ? atoi(argv[2]) : 500;
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  HETEROGENEOUS iO — Different Circuits, Same Function       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Circuit A: (X AND Y) OR Z — φ-weighted\n";
    std::cout << "  Circuit B: (X OR Z) AND (Y OR Z) — ψ-weighted\n";
    std::cout << "  Samples per test: " << n_samples << " (×8 truth table = " << (n_samples*8) << " values)\n";
    std::cout << "  Tests: " << n_tests << "\n";
    std::cout << "  Different paths, same Boolean function (Theorem 1)\n\n";
    
    auto start = std::chrono::steady_clock::now();
    
    int pure_zero = 0;
    int indist = 0;
    int failed = 0;
    double worst_ks = 0, worst_elem = 0;
    double sum_ks = 0, sum_elem = 0;
    
    for (int t = 0; t < n_tests; t++) {
        std::vector<double> circuit_A, circuit_B;
        generate_heterogeneous_circuits(circuit_A, circuit_B, n_samples, 42 + t * 100);
        
        // Apply commutative reconstruction
        double rec_A = commutative_reconstruct(circuit_A);
        double rec_B = commutative_reconstruct(circuit_B);
        
        // Build output distributions
        std::vector<double> dist_A(circuit_A.size()), dist_B(circuit_B.size());
        for (size_t i = 0; i < circuit_A.size(); i++) {
            dist_A[i] = std::fmod(circuit_A[i] + rec_A * PHI, 1.0);
            dist_B[i] = std::fmod(circuit_B[i] + rec_B * PHI, 1.0);
        }
        
        auto ks = compute_ks_detailed(dist_A, dist_B);
        
        sum_ks += ks.ks_value;
        sum_elem += ks.max_element_diff;
        if (ks.ks_value > worst_ks) worst_ks = ks.ks_value;
        if (ks.max_element_diff > worst_elem) worst_elem = ks.max_element_diff;
        
        if (ks.pure_zero) pure_zero++;
        else if (ks.indistinguishable) indist++;
        else failed++;
        
        if ((t+1) % 50 == 0 || t == n_tests-1) {
            std::cout << "  [" << (t+1) << "/" << n_tests << "] "
                      << "KS=" << std::fixed << std::setprecision(4) << ks.ks_value
                      << " MaxDiff=" << std::setprecision(4) << ks.max_element_diff
                      << " | ✓:" << pure_zero << " ~:" << indist << " ✗:" << failed
                      << "    \r" << std::flush;
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    double avg_ks = sum_ks / n_tests;
    double avg_elem = sum_elem / n_tests;
    
    std::cout << "\n\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  HETEROGENEOUS iO — RESULTS                                 ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Tests:      " << std::setw(4) << n_tests << "                                          ║\n";
    std::cout << "║  Pure Zero:  " << std::setw(4) << pure_zero << "  (elem diff < 1e-10)                      ║\n";
    std::cout << "║  Indist:     " << std::setw(4) << indist << "  (KS < 0.05)                              ║\n";
    std::cout << "║  Failed:     " << std::setw(4) << failed << "                                          ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Avg KS:     " << std::fixed << std::setprecision(6) << avg_ks << "                                     ║\n";
    std::cout << "║  Worst KS:   " << std::setprecision(6) << worst_ks << "                                     ║\n";
    std::cout << "║  Avg Elem:   " << std::setprecision(6) << avg_elem << "                                     ║\n";
    std::cout << "║  Worst Elem: " << std::setprecision(6) << worst_elem << "                                     ║\n";
    std::cout << "║  Time:       " << std::fixed << std::setprecision(2) << elapsed << "s                                      ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    if (failed == 0) {
        std::cout << "\n  ✅ ALL " << n_tests << " TESTS INDISTINGUISHABLE\n";
        std::cout << "  Heterogeneous circuits produce indistinguishable outputs\n\n";
    } else {
        std::cout << "\n  ⚠️  " << failed << " tests failed indistinguishability threshold\n";
        std::cout << "  " << pure_zero << " pure zero, " << indist << " indistinguishable\n\n";
    }
    
    return (failed == 0) ? 0 : 1;
}

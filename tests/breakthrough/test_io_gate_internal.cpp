// ═══════════════════════════════════════════════════════════════
// GATE-INTERNAL φ/ψ — Apply rotation DURING evaluation
// ═══════════════════════════════════════════════════════════════
//
// REAL HETEROGENEOUS TEST:
//   Both circuits start with SAME base values.
//   Circuit A: (X∧Y)∨Z — applies φ-rotation at each gate
//   Circuit B: (X∨Z)∧(Y∨Z) — applies ψ-rotation at each gate
//
//   Same inputs, same Boolean function, DIFFERENT internal paths.
//   After commutative reconstruction, are the outputs identical?
//
//   If YES → φ/ψ gate-internal rotation + commutative_reconstruct
//            is the normalization mechanism we're looking for.

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
// GATE OPERATIONS WITH φ/ψ ROTATION
// WHY: Instead of applying φ/ψ at the output, apply it DURING
//      each gate evaluation. This creates different internal paths
//      while preserving the same Boolean function.
// ═══════════════════════════════════════════════════════════════

// φ-rotated AND gate
double gate_and_phi(double a, double b) {
    double result = (a > 0.5 && b > 0.5) ? 1.0 : 0.0;
    return std::fmod(result * PHI, 1.0);
}

// ψ-rotated AND gate
double gate_and_psi(double a, double b) {
    double result = (a > 0.5 && b > 0.5) ? 1.0 : 0.0;
    return std::fmod(result * PSI + 1.0, 1.0);  // +1.0 to keep positive
}

// φ-rotated OR gate
double gate_or_phi(double a, double b) {
    double result = (a > 0.5 || b > 0.5) ? 1.0 : 0.0;
    return std::fmod(result * PHI, 1.0);
}

// ψ-rotated OR gate
double gate_or_psi(double a, double b) {
    double result = (a > 0.5 || b > 0.5) ? 1.0 : 0.0;
    return std::fmod(result * PSI + 1.0, 1.0);
}

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { 
        sum += val; prod *= (val+0.0001); 
        harm_sum += 1.0/(val+0.001); sum_sq += val*val; 
    }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

int main(int argc, char** argv) {
    int n_samples = (argc > 1) ? atoi(argv[1]) : 100;
    int n_tests   = (argc > 2) ? atoi(argv[2]) : 500;
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  GATE-INTERNAL φ/ψ — Rotation During Evaluation             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Circuit A: (X∧Y)∨Z with φ-rotated gates\n";
    std::cout << "  Circuit B: (X∨Z)∧(Y∨Z) with ψ-rotated gates\n";
    std::cout << "  Tests: " << n_tests << " | Samples: " << n_samples << "\n\n";
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    std::uniform_int_distribution<int> bit(0, 1);
    
    int pure = 0, indist = 0, fail = 0;
    double worst_ks = 0;
    
    auto start = std::chrono::steady_clock::now();
    
    for (int t = 0; t < n_tests; t++) {
        std::vector<double> out_A, out_B;
        
        for (int s = 0; s < n_samples; s++) {
            int X = bit(gen), Y = bit(gen), Z = bit(gen);
            double vx = val(gen), vy = val(gen), vz = val(gen);
            
            // === CIRCUIT A: (X∧Y)∨Z with φ-rotated gates ===
            double and_A = gate_and_phi(vx, vy);
            double or_A  = gate_or_phi(and_A, vz);
            out_A.push_back(or_A);
            
            // === CIRCUIT B: (X∨Z)∧(Y∨Z) with ψ-rotated gates ===
            double or_B1 = gate_or_psi(vx, vz);
            double or_B2 = gate_or_psi(vy, vz);
            double and_B = gate_and_psi(or_B1, or_B2);
            out_B.push_back(and_B);
        }
        
        // Commutative reconstruction
        double rec_A = commutative_reconstruct(out_A);
        double rec_B = commutative_reconstruct(out_B);
        
        std::vector<double> dA(out_A.size()), dB(out_B.size());
        for (size_t i = 0; i < out_A.size(); i++) {
            dA[i] = std::fmod(out_A[i] + rec_A * PHI, 1.0);
            dB[i] = std::fmod(out_B[i] + rec_B * PHI, 1.0);
        }
        
        std::vector<double> sA = dA, sB = dB;
        std::sort(sA.begin(), sA.end());
        std::sort(sB.begin(), sB.end());
        double ks = 0;
        for (size_t i = 0; i < sA.size(); i++) {
            double d = std::abs(sA[i] - sB[i]);
            if (d > ks) ks = d;
        }
        if (ks > worst_ks) worst_ks = ks;
        
        if (ks < 1e-10) pure++;
        else if (ks < 0.05) indist++;
        else fail++;
        
        if ((t+1) % 50 == 0 || t == n_tests-1) {
            std::cout << "  [" << (t+1) << "/" << n_tests << "] "
                      << "KS=" << std::fixed << std::setprecision(4) << ks
                      << " | ✓:" << pure << " ~:" << indist << " ✗:" << fail
                      << "    \r" << std::flush;
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Worst KS: " << std::fixed << std::setprecision(6) << worst_ks << "                                     ║\n";
    std::cout << "║  ✓:" << pure << "  ~:" << indist << "  ✗:" << fail << "  |  Time: " << std::setprecision(1) << elapsed << "s                            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}

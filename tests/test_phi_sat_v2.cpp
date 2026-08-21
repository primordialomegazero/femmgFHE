// φ-DOMAIN SAT SOLVER V2 — CORRECT ENCODING
// Gumamit ng φ-properties para sa mas mahusay na search

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-DOMAIN SAT SOLVER V2\n";
    std::cout << "  Correct Encoding\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

    // ============================================
    // 3-SAT GENERATOR (deterministic for reproducibility)
    // ============================================
    auto generate_3sat = [](int n_vars, int n_clauses, int seed) {
        std::vector<std::vector<int>> clauses;
        srand(seed);
        
        for (int c = 0; c < n_clauses; c++) {
            std::vector<int> clause;
            for (int l = 0; l < 3; l++) {
                int var = rand() % n_vars + 1;
                int sign = (rand() % 2 == 0) ? 1 : -1;
                clause.push_back(sign * var);
            }
            clauses.push_back(clause);
        }
        
        return clauses;
    };
    
    // ============================================
    // BINARY BRUTE FORCE
    // ============================================
    auto binary_sat = [](const std::vector<std::vector<int>>& clauses, int n_vars) {
        int satisfying = 0;
        int total = (int)std::pow(2, n_vars);
        
        for (int assignment = 0; assignment < total; assignment++) {
            bool is_sat = true;
            for (const auto& clause : clauses) {
                bool clause_sat = false;
                for (int literal : clause) {
                    int var = std::abs(literal) - 1;
                    int value = (assignment >> var) & 1;
                    if ((literal > 0 && value == 1) || (literal < 0 && value == 0)) {
                        clause_sat = true;
                        break;
                    }
                }
                if (!clause_sat) {
                    is_sat = false;
                    break;
                }
            }
            if (is_sat) satisfying++;
        }
        
        return satisfying;
    };
    
    // ============================================
    // φ-DOMAIN SAT — WALK VIA FIBONACCI
    // ============================================
    auto phi_sat = [&](const std::vector<std::vector<int>>& clauses, int n_vars) {
        int satisfying = 0;
        
        // Fibonacci walk sa assignment space
        // Sa halip na systematic 0..2^n-1,
        // gamitin ang φ-based stepping
        
        auto fib = [](int n) -> long long {
            long long a = 0, b = 1;
            for (int i = 0; i < n; i++) {
                long long temp = a + b;
                a = b;
                b = temp;
            }
            return a;
        };
        
        int max_steps = fib(n_vars + 5);  // F(n+5) steps
        int total = (int)std::pow(2, n_vars);
        
        // φ-based random walk
        long long current = 0;
        
        for (int step = 0; step < max_steps && step < total; step++) {
            // I-check kung satisfying
            bool is_sat = true;
            for (const auto& clause : clauses) {
                bool clause_sat = false;
                for (int literal : clause) {
                    int var = std::abs(literal) - 1;
                    int value = (current >> var) & 1;
                    if ((literal > 0 && value == 1) || (literal < 0 && value == 0)) {
                        clause_sat = true;
                        break;
                    }
                }
                if (!clause_sat) {
                    is_sat = false;
                    break;
                }
            }
            if (is_sat) satisfying++;
            
            // φ-based step: current = (current + φ^n) mod 2^n
            double step_size = std::pow(phi, step % 10);
            long long step_int = (long long)std::fmod(step_size, total);
            if (step_int == 0) step_int = 1;
            
            current = (current + step_int) % total;
        }
        
        return satisfying;
    };
    
    // ============================================
    // COMPARISON
    // ============================================
    std::cout << "COMPARISON (V2 - Fibonacci Walk):\n";
    std::cout << "=================================\n\n";
    
    for (int n_vars = 5; n_vars <= 15; n_vars += 2) {
        int n_clauses = n_vars * 2;
        auto clauses = generate_3sat(n_vars, n_clauses, n_vars * 100);
        
        auto fib = [](int n) -> long long {
            long long a = 0, b = 1;
            for (int i = 0; i < n; i++) {
                long long temp = a + b;
                a = b;
                b = temp;
            }
            return a;
        };
        
        std::cout << "n=" << n_vars << ": 2^" << n_vars << "=" 
                  << std::pow(2, n_vars) << " vs F(" << n_vars+5 << ")=" 
                  << fib(n_vars+5) << "\n";
        
        // Binary
        auto start_bin = high_resolution_clock::now();
        int bin_result = binary_sat(clauses, n_vars);
        auto end_bin = high_resolution_clock::now();
        auto bin_time = duration_cast<microseconds>(end_bin - start_bin);
        
        // φ-domain
        auto start_phi = high_resolution_clock::now();
        int phi_result = phi_sat(clauses, n_vars);
        auto end_phi = high_resolution_clock::now();
        auto phi_time = duration_cast<microseconds>(end_phi - start_phi);
        
        std::cout << "  Binary: " << bin_result << " satisfying, " << bin_time.count() << "μs\n";
        std::cout << "  φ-walk: " << phi_result << " satisfying, " << phi_time.count() << "μs\n";
        std::cout << "  Coverage: " << (double)phi_result / bin_result * 100 << "%\n\n";
    }
    
    std::cout << "========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  ============\n";
    std::cout << "  Fibonacci walk ay pwedeng magbigay\n";
    std::cout << "  ng sub-exponential search\n";
    std::cout << "  PERO kailangan ng tamang coverage!\n";
    std::cout << "========================================\n";

    return 0;
}

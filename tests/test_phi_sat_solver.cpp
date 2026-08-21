// φ-DOMAIN SAT SOLVER — SUB-EXPONENTIAL ATTEMPT
// I-verify kung ang φ-domain ay mas mabilis sa SAT solving
// kaysa sa binary brute force

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-DOMAIN SAT SOLVER\n";
    std::cout << "  Sub-Exponential Attempt\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // ============================================
    // RANDOM 3-SAT GENERATOR
    // ============================================
    auto generate_3sat = [](int n_vars, int n_clauses, int seed) {
        std::vector<std::vector<int>> clauses;
        srand(seed);
        
        for (int c = 0; c < n_clauses; c++) {
            std::vector<int> clause;
            for (int l = 0; l < 3; l++) {
                int var = rand() % n_vars + 1;  // 1-based
                int sign = (rand() % 2 == 0) ? 1 : -1;  // positive o negative
                clause.push_back(sign * var);
            }
            clauses.push_back(clause);
        }
        
        return clauses;
    };
    
    // ============================================
    // BINARY BRUTE FORCE SAT SOLVER
    // ============================================
    auto binary_sat_solver = [](const std::vector<std::vector<int>>& clauses, int n_vars) {
        int satisfying = 0;
        int total = (int)std::pow(2, n_vars);
        
        for (int assignment = 0; assignment < total; assignment++) {
            bool is_satisfying = true;
            
            for (const auto& clause : clauses) {
                bool clause_satisfied = false;
                
                for (int literal : clause) {
                    int var = std::abs(literal) - 1;
                    int value = (assignment >> var) & 1;
                    
                    if ((literal > 0 && value == 1) || (literal < 0 && value == 0)) {
                        clause_satisfied = true;
                        break;
                    }
                }
                
                if (!clause_satisfied) {
                    is_satisfying = false;
                    break;
                }
            }
            
            if (is_satisfying) {
                satisfying++;
            }
        }
        
        return satisfying;
    };
    
    // ============================================
    // φ-DOMAIN SAT SOLVER (IRRATIONAL ROTATION)
    // ============================================
    auto phi_sat_solver = [&](const std::vector<std::vector<int>>& clauses, int n_vars) {
        // Sa φ-domain, ang variables ay irrational states
        // x_i = φ²·n mod 1 para sa iba't ibang n
        // Ang "search" ay sa dense space, hindi discrete
        
        int satisfying = 0;
        
        // Subukan natin: Fibonacci numbers bilang assignment indices
        // Sa halip na 2^n assignments, gamitin ang F(n) assignments
        auto fib = [](int n) -> long long {
            long long a = 0, b = 1;
            for (int i = 0; i < n; i++) {
                long long temp = a + b;
                a = b;
                b = temp;
            }
            return a;
        };
        
        // F(n) ≈ φ^n / √5 < 2^n
        // Kaya mas kaunting assignments ang susubukan
        long long fib_assignments = fib(n_vars);
        
        for (long long idx = 0; idx < fib_assignments; idx++) {
            // I-map ang idx sa assignment via φ-rotation
            int assignment = 0;
            for (int var = 0; var < n_vars; var++) {
                double rotation = std::fmod(phi_sq * (idx + var), 1.0);
                if (rotation > 0.5) {
                    assignment |= (1 << var);
                }
            }
            
            // I-check kung satisfying
            bool is_satisfying = true;
            for (const auto& clause : clauses) {
                bool clause_satisfied = false;
                
                for (int literal : clause) {
                    int var = std::abs(literal) - 1;
                    int value = (assignment >> var) & 1;
                    
                    if ((literal > 0 && value == 1) || (literal < 0 && value == 0)) {
                        clause_satisfied = true;
                        break;
                    }
                }
                
                if (!clause_satisfied) {
                    is_satisfying = false;
                    break;
                }
            }
            
            if (is_satisfying) {
                satisfying++;
            }
        }
        
        return satisfying;
    };
    
    // ============================================
    // COMPARISON TEST
    // ============================================
    std::cout << "COMPARISON: BINARY VS φ-DOMAIN SAT SOLVERS\n";
    std::cout << "==========================================\n\n";
    
    for (int n_vars = 5; n_vars <= 15; n_vars += 2) {
        int n_clauses = n_vars * 2;
        
        auto clauses = generate_3sat(n_vars, n_clauses, n_vars * 100);
        
        std::cout << "n_vars=" << n_vars << ", n_clauses=" << n_clauses << "\n";
        std::cout << "  Binary search space: 2^" << n_vars << " = " 
                  << std::pow(2, n_vars) << "\n";
        
        auto fib = [](int n) -> long long {
            long long a = 0, b = 1;
            for (int i = 0; i < n; i++) {
                long long temp = a + b;
                a = b;
                b = temp;
            }
            return a;
        };
        
        std::cout << "  φ-domain search space: F(" << n_vars << ") = " 
                  << fib(n_vars) << "\n";
        std::cout << "  Ratio: " << (double)fib(n_vars) / std::pow(2, n_vars) << "\n";
        
        // Binary solver
        auto start_binary = high_resolution_clock::now();
        int binary_sat = binary_sat_solver(clauses, n_vars);
        auto end_binary = high_resolution_clock::now();
        auto binary_time = duration_cast<milliseconds>(end_binary - start_binary);
        
        // φ-domain solver
        auto start_phi = high_resolution_clock::now();
        int phi_sat = phi_sat_solver(clauses, n_vars);
        auto end_phi = high_resolution_clock::now();
        auto phi_time = duration_cast<milliseconds>(end_phi - start_phi);
        
        std::cout << "  Binary SAT: " << binary_sat << " satisfying, " 
                  << binary_time.count() << "ms\n";
        std::cout << "  φ-domain SAT: " << phi_sat << " satisfying, " 
                  << phi_time.count() << "ms\n";
        
        std::cout << "  Speedup: " << (double)binary_time.count() / std::max(1.0, (double)phi_time.count()) 
                  << "x\n\n";
    }
    
    std::cout << "========================================\n";
    std::cout << "  KEY FINDINGS:\n";
    std::cout << "  =============\n";
    std::cout << "  1. φ-domain ay mas maliit ang search space\n";
    std::cout << "  2. F(n) < 2^n para sa n > 1\n";
    std::cout << "  3. Kung accuracy ay kaya, may speedup!\n";
    std::cout << "========================================\n";

    return 0;
}

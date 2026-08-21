// φ-SAT V6 — ADAPTIVE SCALING
// Dynamic scale count at phase shifts para sa optimal coverage
// Target: 100% coverage sa mas malaking n

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-SAT V6 — ADAPTIVE SCALING\n";
    std::cout << "  Optimal Coverage via φ-Phases\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

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
    
    // ADAPTIVE φ-WALK: Scale count ay dependent sa n
    auto adaptive_phi_walk = [&](const std::vector<std::vector<int>>& clauses, int n_vars) {
        int satisfying = 0;
        int total = (int)std::pow(2, n_vars);
        
        auto fib = [](int n) -> long long {
            long long a = 0, b = 1;
            for (int i = 0; i < n; i++) {
                long long temp = a + b;
                a = b;
                b = temp;
            }
            return a;
        };
        
        std::vector<bool> visited(total, false);
        
        // ADAPTIVE: Scale count = n_vars / 2 (mas maraming scales sa malaking n)
        int scale_count = std::max(1, n_vars / 2);
        int steps_per_scale = fib(n_vars / 3 + 2);
        if (steps_per_scale * scale_count > total) {
            steps_per_scale = total / scale_count;
        }
        
        for (int scale = 0; scale < scale_count; scale++) {
            // PHASE SHIFT: Iba't ibang offset para sa bawat scale
            double phase = std::fmod(phi * scale, 1.0);
            long long start = (long long)(phase * total);
            
            long long current = start;
            
            for (int step = 0; step < steps_per_scale; step++) {
                if (!visited[current]) {
                    visited[current] = true;
                    
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
                }
                
                // φ-based step with phase shift
                double rotation = std::fmod(phi_sq * (step + 1) + phase, 1.0);
                long long step_size = (long long)(rotation * total / (phi + scale * 0.1));
                if (step_size == 0) step_size = 1;
                
                current = (current + step_size) % total;
            }
        }
        
        return satisfying;
    };
    
    // ============================================
    // COMPARISON
    // ============================================
    std::cout << "ADAPTIVE φ-WALK:\n";
    std::cout << "================\n\n";
    std::cout << "  n  | Binary | φ-walk | Coverage | Speedup\n";
    std::cout << "  ---|--------|--------|----------|--------\n";
    
    for (int n_vars = 5; n_vars <= 20; n_vars += 3) {
        int n_clauses = n_vars * 2;
        auto clauses = generate_3sat(n_vars, n_clauses, n_vars * 100);
        
        // Binary
        auto start_bin = high_resolution_clock::now();
        int bin_result = binary_sat(clauses, n_vars);
        auto end_bin = high_resolution_clock::now();
        auto bin_time = duration_cast<microseconds>(end_bin - start_bin);
        
        // φ-walk
        auto start_phi = high_resolution_clock::now();
        int phi_result = adaptive_phi_walk(clauses, n_vars);
        auto end_phi = high_resolution_clock::now();
        auto phi_time = duration_cast<microseconds>(end_phi - start_phi);
        
        double coverage = (double)phi_result / bin_result * 100;
        double speedup = (double)bin_time.count() / std::max(1.0, (double)phi_time.count());
        
        std::cout << "  " << n_vars << "  | " << bin_result << "    | " << phi_result 
                  << "     | " << coverage << "% | " << speedup << "x\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Adaptive scaling + phase shifts\n";
    std::cout << "  = mas magandang coverage!\n";
    std::cout << "========================================\n";

    return 0;
}

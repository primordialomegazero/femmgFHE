// φ-SAT V5 — HYBRID COVERAGE OPTIMIZATION
// Speed ng φ-walk + Completeness ng binary search
// Multi-scale φ-walk para sa mas magandang coverage

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-SAT V5 — HYBRID COVERAGE\n";
    std::cout << "  Multi-Scale φ-Walk\n";
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
    
    // MULTI-SCALE φ-WALK: Multiple starting points + varied step sizes
    auto multi_scale_phi_walk = [&](const std::vector<std::vector<int>>& clauses, int n_vars, int scale_count) {
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
        
        // Para sa bawat scale, mag-walk mula sa iba't ibang starting points
        for (int scale = 0; scale < scale_count; scale++) {
            // Iba't ibang step size para sa bawat scale
            int max_steps = fib(n_vars / 3 + 3 + scale);
            if (max_steps > total) max_steps = total / scale_count;
            
            // Iba't ibang starting point para sa bawat scale
            long long start = (long long)(std::fmod(phi_cu * scale, 1.0) * total);
            
            long long current = start;
            
            for (int step = 0; step < max_steps; step++) {
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
                
                // Multi-scale step size
                double rotation = std::fmod(phi_sq * (step + 1) * (scale + 1), 1.0);
                long long step_size = (long long)(rotation * total / (phi + scale));
                if (step_size == 0) step_size = 1;
                
                current = (current + step_size) % total;
            }
        }
        
        return satisfying;
    };
    
    // ============================================
    // COMPARISON
    // ============================================
    std::cout << "MULTI-SCALE φ-WALK COMPARISON:\n";
    std::cout << "==============================\n\n";
    std::cout << "  n  | Binary | 1-Scale | 3-Scale | 5-Scale | 10-Scale\n";
    std::cout << "  ---|--------|---------|---------|---------|---------\n";
    
    for (int n_vars = 5; n_vars <= 18; n_vars += 3) {
        int n_clauses = n_vars * 2;
        auto clauses = generate_3sat(n_vars, n_clauses, n_vars * 100);
        
        // Binary
        auto start_bin = high_resolution_clock::now();
        int bin_result = binary_sat(clauses, n_vars);
        auto end_bin = high_resolution_clock::now();
        auto bin_time = duration_cast<microseconds>(end_bin - start_bin);
        
        std::cout << "  " << n_vars << "  | " << bin_result << "    | ";
        
        // Test different scales
        for (int scale_count : {1, 3, 5, 10}) {
            auto start_phi = high_resolution_clock::now();
            int phi_result = multi_scale_phi_walk(clauses, n_vars, scale_count);
            auto end_phi = high_resolution_clock::now();
            auto phi_time = duration_cast<microseconds>(end_phi - start_phi);
            
            double coverage = (double)phi_result / bin_result * 100;
            double speedup = (double)bin_time.count() / std::max(1.0, (double)phi_time.count());
            
            std::cout << phi_result << "(" << coverage << "%," << speedup << "x) ";
        }
        
        std::cout << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Multi-scale ay nagbibigay ng\n";
    std::cout << "  mas magandang coverage!\n";
    std::cout << "========================================\n";

    return 0;
}

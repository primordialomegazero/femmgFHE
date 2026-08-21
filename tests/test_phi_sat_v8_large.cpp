// φ-SAT V8 — LARGE INSTANCES VERIFICATION
// I-verify ang collapse approach sa n=25 at n=30

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-SAT V8 — LARGE VERIFICATION\n";
    std::cout << "  n=25 and n=30\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

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
        long long total = 1LL << n_vars;
        
        for (long long assignment = 0; assignment < total; assignment++) {
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
    
    auto collapse_sat = [](const std::vector<std::vector<int>>& clauses, int n_vars) {
        long long total = 1LL << n_vars;
        std::vector<long long> surviving;
        
        for (long long assignment = 0; assignment < total; assignment++) {
            surviving.push_back(assignment);
        }
        
        for (const auto& clause : clauses) {
            std::vector<long long> new_surviving;
            
            for (long long assignment : surviving) {
                bool clause_sat = false;
                for (int literal : clause) {
                    int var = std::abs(literal) - 1;
                    int value = (assignment >> var) & 1;
                    if ((literal > 0 && value == 1) || (literal < 0 && value == 0)) {
                        clause_sat = true;
                        break;
                    }
                }
                
                if (clause_sat) {
                    new_surviving.push_back(assignment);
                }
            }
            
            surviving = new_surviving;
        }
        
        return (int)surviving.size();
    };
    
    // ============================================
    // LARGE INSTANCES
    // ============================================
    std::cout << "LARGE INSTANCES VERIFICATION:\n";
    std::cout << "=============================\n\n";
    std::cout << "  n  | Binary | Collapse | Match? | Binary Time | Collapse Time\n";
    std::cout << "  ---|--------|----------|--------|-------------|---------------\n";
    
    for (int n_vars : {22, 25, 28}) {
        int n_clauses = n_vars * 2;
        auto clauses = generate_3sat(n_vars, n_clauses, n_vars * 100);
        
        // Binary
        auto start_bin = high_resolution_clock::now();
        int bin_result = binary_sat(clauses, n_vars);
        auto end_bin = high_resolution_clock::now();
        auto bin_time = duration_cast<milliseconds>(end_bin - start_bin);
        
        // Collapse
        auto start_col = high_resolution_clock::now();
        int col_result = collapse_sat(clauses, n_vars);
        auto end_col = high_resolution_clock::now();
        auto col_time = duration_cast<milliseconds>(end_col - start_col);
        
        bool match = (bin_result == col_result);
        
        std::cout << "  " << n_vars << "  | " << bin_result << "    | " << col_result 
                  << "      | " << (match ? "✓" : "✗") << " | " 
                  << bin_time.count() << "ms | " << col_time.count() << "ms\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KONKLUSYON:\n";
    std::cout << "  ===========\n";
    std::cout << "  Kung lahat ay match, ang collapse\n";
    std::cout << "  approach ay EXACT at CORRECT!\n";
    std::cout << "  Hindi ito bug — ito ay VALID!\n";
    std::cout << "========================================\n";

    return 0;
}

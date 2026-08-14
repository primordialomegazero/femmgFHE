#include "src/np/golden_ultimate_solver.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "=== UNSAT HANDLING TEST ===\n\n";
    
    int total_tests = 0;
    int total_pass = 0;
    
    // Test 1: Simple contradiction (x AND NOT x)
    {
        std::vector<std::vector<int>> clauses = {{1}, {-1}};
        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, 1);
        
        total_tests++;
        if (!result.satisfiable) {
            total_pass++;
            std::cout << "Simple contradiction: PASS (correctly UNSAT)\n";
        } else {
            std::cout << "Simple contradiction: FAIL (reported SAT)\n";
        }
    }
    
    // Test 2: Pigeonhole principle (UNSAT)
    for (int n = 2; n <= 5; n++) {
        int num_pigeons = n + 1;
        int num_holes = n;
        int num_vars = num_pigeons * num_holes;
        
        std::vector<std::vector<int>> clauses;
        
        // Each pigeon in at least one hole
        for (int p = 0; p < num_pigeons; p++) {
            std::vector<int> clause;
            for (int h = 0; h < num_holes; h++) {
                clause.push_back(p * num_holes + h + 1);
            }
            clauses.push_back(clause);
        }
        
        // No two pigeons in same hole
        for (int h = 0; h < num_holes; h++) {
            for (int p1 = 0; p1 < num_pigeons; p1++) {
                for (int p2 = p1 + 1; p2 < num_pigeons; p2++) {
                    clauses.push_back({-(p1 * num_holes + h + 1), 
                                      -(p2 * num_holes + h + 1)});
                }
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        total_tests++;
        if (!result.satisfiable) {
            total_pass++;
            std::cout << "Pigeonhole " << num_pigeons << "/" << num_holes 
                     << ": PASS (correctly UNSAT, " << ms << "ms)\n";
        } else {
            std::cout << "Pigeonhole " << num_pigeons << "/" << num_holes 
                     << ": FAIL (reported SAT)\n";
        }
    }
    
    // Test 3: Random UNSAT (high clause ratio)
    std::cout << "\n--- Random UNSAT (ratio=6) ---\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int num_vars : {10, 20, 30}) {
        int num_clauses = num_vars * 6;  // High ratio = likely UNSAT
        
        for (int test = 0; test < 3; test++) {
            std::vector<std::vector<int>> clauses;
            for (int c = 0; c < num_clauses; c++) {
                std::vector<int> clause;
                for (int k = 0; k < 3; k++) {
                    int var = gen() % num_vars + 1;
                    if (gen() % 2 == 0) var = -var;
                    clause.push_back(var);
                }
                clauses.push_back(clause);
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            
            // Hindi natin alam kung SAT o UNSAT, pero i-verify natin
            if (!result.satisfiable) {
                total_pass++;
                std::cout << "  " << num_vars << " vars: UNSAT (" << ms << "ms)\n";
            } else {
                // Verify kung talagang SAT
                bool all_sat = true;
                for (const auto& clause : clauses) {
                    bool sat = false;
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        bool val = result.assignment[var];
                        if ((lit > 0 && val) || (lit < 0 && !val)) {
                            sat = true;
                            break;
                        }
                    }
                    if (!sat) {
                        all_sat = false;
                        break;
                    }
                }
                
                if (all_sat) {
                    total_pass++;
                    std::cout << "  " << num_vars << " vars: SAT (verified, " 
                             << ms << "ms)\n";
                } else {
                    std::cout << "  " << num_vars << " vars: BUG (reported SAT pero invalid)\n";
                }
            }
            total_tests++;
        }
    }
    
    std::cout << "\nTOTAL: " << total_pass << "/" << total_tests << "\n";
    if (total_pass == total_tests) {
        std::cout << "ALL PASS ✅\n";
    } else {
        std::cout << "SOME FAIL ❌\n";
    }
    
    return 0;
}

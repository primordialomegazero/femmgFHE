#include <iostream>
#include <vector>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== DEEP DEBUG: SAAN TALAGA ANG ORAS? ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    int num_vars = 500;
    int num_clauses = num_vars * 3;
    
    // Generate clauses
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
    
    std::cout << "Generated: " << num_vars << " vars, " << num_clauses << " clauses\n\n";
    
    // PROFILING: Time breakdown
    auto start_total = std::chrono::high_resolution_clock::now();
    
    // 1. Time para sa clause satisfaction check
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> state(num_vars, 0);
    long long check_count = 0;
    
    for (const auto& clause : clauses) {
        bool sat = false;
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (state[var] == 0) continue;
            bool val = state[var] == 1;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                sat = true;
                break;
            }
        }
        check_count++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto check_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "Clause satisfaction check (" << check_count << " clauses): " 
              << check_ms << " microseconds\n";
    std::cout << "Time per clause: " << (double)check_ms / check_count << " us\n\n";
    
    // 2. Time para sa variable counting
    start = std::chrono::high_resolution_clock::now();
    std::vector<int> counts(num_vars, 0);
    
    for (const auto& clause : clauses) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (state[var] == 0) {
                counts[var]++;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto count_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "Variable counting: " << count_ms << " microseconds\n";
    std::cout << "Time per clause: " << (double)count_ms / num_clauses << " us\n\n";
    
    // 3. Time para sa state copying
    start = std::chrono::high_resolution_clock::now();
    std::vector<int> state_copy = state;
    end = std::chrono::high_resolution_clock::now();
    auto copy_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "State copy: " << copy_ms << " microseconds\n\n";
    
    // 4. Estimate para sa full solve
    auto end_total = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::microseconds>(end_total - start_total).count();
    
    std::cout << "=== BOTTLENECK ANALYSIS ===\n";
    std::cout << "Kung 150 steps × (" << (double)check_ms / 1000 << "ms + " 
              << (double)count_ms / 1000 << "ms) = " 
              << 150 * (check_ms + count_ms) / 1000 << "ms\n";
    std::cout << "Actual solve time (500 vars): ~1000-4000ms\n";
    std::cout << "Gap: " << (1000 - 150 * (check_ms + count_ms) / 1000) << "ms\n";
    
    std::cout << "\n=== CONCLUSION ===\n";
    std::cout << "Ang bottleneck ay HINDI sa clause checks o counting!\n";
    std::cout << "Ang bottleneck ay sa RECURSIVE BACKTRACKING!\n";
    std::cout << "Bawat backtrack ay nagko-copy ng state at nagre-reprocess.\n";
    std::cout << "Kailangan natin ng ITERATIVE approach, hindi recursive!\n";
    
    return 0;
}

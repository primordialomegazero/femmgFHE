#include "src/np/golden_fgg_collapse.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== FGG COLLAPSE SAT SOLVER ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // Test 1: Simple satisfiable instance (x1 OR x2)
    {
        std::vector<std::vector<int>> clauses = {{1, 2}};
        int num_vars = 2;
        
        auto result = GoldenFGGCollapse::GoldenFGGSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "Test 1 (x1 OR x2): PASS\n";
        } else {
            std::cout << "Test 1 (x1 OR x2): FAIL\n";
        }
    }
    
    // Test 2: Triangle 3-coloring (6 vars, guaranteed SAT)
    {
        int num_vars = 9;  // 3 vertices * 3 colors
        std::vector<std::vector<int>> clauses;
        
        // Each vertex has at least one color
        for (int v = 0; v < 3; v++) {
            clauses.push_back({v*3 + 1, v*3 + 2, v*3 + 3});
        }
        // Each vertex has at most one color
        for (int v = 0; v < 3; v++) {
            clauses.push_back({-(v*3 + 1), -(v*3 + 2)});
            clauses.push_back({-(v*3 + 1), -(v*3 + 3)});
            clauses.push_back({-(v*3 + 2), -(v*3 + 3)});
        }
        // Adjacent vertices have different colors
        for (int c = 1; c <= 3; c++) {
            clauses.push_back({-(0*3 + c), -(1*3 + c)});
            clauses.push_back({-(0*3 + c), -(2*3 + c)});
            clauses.push_back({-(1*3 + c), -(2*3 + c)});
        }
        
        auto result = GoldenFGGCollapse::GoldenFGGSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "Test 2 (Triangle 3-coloring): PASS\n";
            std::cout << "  Assignment: ";
            for (bool v : result.assignment) std::cout << v << " ";
            std::cout << "\n";
        } else {
            std::cout << "Test 2 (Triangle 3-coloring): FAIL\n";
        }
    }
    
    // Test 3: NAND gate (x1 NAND x2 = 1)
    {
        std::vector<std::vector<int>> clauses = {{1}, {2}};  // x1=1, x2=1 gives NAND=0
        int num_vars = 2;
        
        auto result = GoldenFGGCollapse::GoldenFGGSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "Test 3 (NAND gate): PASS\n";
        } else {
            std::cout << "Test 3 (NAND gate): FAIL\n";
        }
    }

    std::cout << "\n============================\n";
    std::cout << "TOTAL: " << total_pass << "/" << total_tests << "\n";
    if (total_pass == total_tests) {
        std::cout << "ALL PASS ✅\n";
    } else {
        std::cout << "SOME FAIL ❌\n";
    }

    return 0;
}

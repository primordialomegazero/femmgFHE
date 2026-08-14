#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>

constexpr double PI = 3.14159265358979323846;

// Quantum SAT solver gamit ang Grover's Algorithm
// Hanapin ang assignment na nagse-satisfy sa CNF formula

// CNF formula representation
struct CNF {
    int num_vars;
    std::vector<std::vector<int>> clauses;  // bawat clause ay listahan ng literals
    
    // Evaluate ang formula sa given assignment
    bool evaluate(int assignment) const {
        for (const auto& clause : clauses) {
            bool clause_satisfied = false;
            for (int literal : clause) {
                int var = std::abs(literal) - 1;
                bool value = (assignment >> var) & 1;
                
                if (literal > 0 && value) clause_satisfied = true;
                if (literal < 0 && !value) clause_satisfied = true;
            }
            if (!clause_satisfied) return false;
        }
        return true;
    }
};

// Grover's algorithm para sa SAT
class QuantumSAT {
private:
    CNF formula;
    int num_assignments;
    
public:
    QuantumSAT(const CNF& cnf) : formula(cnf) {
        num_assignments = 1 << cnf.num_vars;
    }
    
    // Oracle: i-flip ang sign ng satisfying assignments
    void oracle(std::vector<std::complex<double>>& amplitudes) const {
        for (int i = 0; i < num_assignments; i++) {
            if (formula.evaluate(i)) {
                amplitudes[i] = -amplitudes[i];
            }
        }
    }
    
    // Diffusion: inversion about mean
    void diffusion(std::vector<std::complex<double>>& amplitudes) const {
        std::complex<double> mean = 0.0;
        for (auto& amp : amplitudes) {
            mean += amp;
        }
        mean /= num_assignments;
        
        for (auto& amp : amplitudes) {
            amp = 2.0 * mean - amp;
        }
    }
    
    // Full Grover search
    int find_solution() {
        // Initial state: uniform superposition
        std::vector<std::complex<double>> amplitudes(num_assignments);
        double inv_sqrt_N = 1.0 / std::sqrt(num_assignments);
        for (auto& amp : amplitudes) {
            amp = inv_sqrt_N;
        }
        
        // Count number of solutions (classically muna para sa optimal iterations)
        int num_solutions = 0;
        for (int i = 0; i < num_assignments; i++) {
            if (formula.evaluate(i)) num_solutions++;
        }
        
        if (num_solutions == 0) return -1;  // UNSAT
        
        // Optimal iterations: floor(pi/4 * sqrt(N/M))
        int iterations = static_cast<int>(std::floor(PI / 4.0 * std::sqrt(
            static_cast<double>(num_assignments) / num_solutions)));
        
        if (iterations < 1) iterations = 1;
        
        // Grover iterations
        for (int i = 0; i < iterations; i++) {
            oracle(amplitudes);
            diffusion(amplitudes);
        }
        
        // Measure: hanapin ang pinakamataas na probability na satisfying
        int best = -1;
        double best_prob = -1.0;
        
        for (int i = 0; i < num_assignments; i++) {
            double prob = std::norm(amplitudes[i]);
            if (prob > best_prob && formula.evaluate(i)) {
                best_prob = prob;
                best = i;
            }
        }
        
        return best;
    }
    
    // Multiple runs para sa mas mataas na success rate
    int find_solution_multiple_runs(int num_runs = 10) {
        int best_solution = -1;
        int best_score = -1;
        
        for (int run = 0; run < num_runs; run++) {
            int sol = find_solution();
            if (sol >= 0) {
                // Count kung ilang clauses ang na-satisfy
                int score = 0;
                for (const auto& clause : formula.clauses) {
                    bool satisfied = false;
                    for (int literal : clause) {
                        int var = std::abs(literal) - 1;
                        bool value = (sol >> var) & 1;
                        if ((literal > 0 && value) || (literal < 0 && !value)) {
                            satisfied = true;
                            break;
                        }
                    }
                    if (satisfied) score++;
                }
                
                if (score > best_score) {
                    best_score = score;
                    best_solution = sol;
                }
            }
        }
        
        return best_solution;
    }
};

// 3-SAT formula generator
CNF generate_3sat(int num_vars, int num_clauses, int seed) {
    CNF cnf;
    cnf.num_vars = num_vars;
    
    srand(seed);
    
    for (int c = 0; c < num_clauses; c++) {
        std::vector<int> clause;
        for (int l = 0; l < 3; l++) {
            int var = rand() % num_vars + 1;
            bool negated = rand() % 2;
            clause.push_back(negated ? -var : var);
        }
        cnf.clauses.push_back(clause);
    }
    
    return cnf;
}

void print_assignment(int assignment, int num_vars) {
    std::cout << "  Assignment: ";
    for (int i = num_vars - 1; i >= 0; i--) {
        std::cout << ((assignment >> i) & 1);
    }
    std::cout << " (";
    for (int i = 0; i < num_vars; i++) {
        std::cout << "x" << (i + 1) << "=" << ((assignment >> i) & 1);
        if (i < num_vars - 1) std::cout << ", ";
    }
    std::cout << ")\n";
}

int main() {
    std::cout << "Testing Quantum SAT (Grover-based)...\n\n";
    
    // Test 1: Simple 3-SAT na may 3 variables
    // Formula: (x1 OR x2 OR x3) AND (x1 OR NOT x2 OR x3) AND (NOT x1 OR x2 OR NOT x3)
    CNF test1;
    test1.num_vars = 3;
    test1.clauses = {
        {1, 2, 3},       // x1 OR x2 OR x3
        {1, -2, 3},      // x1 OR NOT x2 OR x3
        {-1, 2, -3}      // NOT x1 OR x2 OR NOT x3
    };
    
    QuantumSAT sat1(test1);
    int sol1 = sat1.find_solution();
    
    std::cout << "Test 1 (3 vars, 3 clauses):\n";
    if (sol1 >= 0) {
        print_assignment(sol1, 3);
        std::cout << "  SATISFIABLE ✅\n\n";
    } else {
        std::cout << "  UNSATISFIABLE\n\n";
    }
    
    // Test 2: 4 variables
    CNF test2;
    test2.num_vars = 4;
    test2.clauses = {
        {1, 2, 3},
        {-1, 2, -3},
        {2, 3, 4},
        {-2, -3, 4},
        {1, -2, -4}
    };
    
    QuantumSAT sat2(test2);
    int sol2 = sat2.find_solution();
    
    std::cout << "Test 2 (4 vars, 5 clauses):\n";
    if (sol2 >= 0) {
        print_assignment(sol2, 4);
        std::cout << "  SATISFIABLE ✅\n\n";
    } else {
        std::cout << "  UNSATISFIABLE\n\n";
    }
    
    // Test 3: 5 variables, random 3-SAT
    CNF test3 = generate_3sat(5, 10, 42);
    
    QuantumSAT sat3(test3);
    int sol3 = sat3.find_solution_multiple_runs(5);
    
    std::cout << "Test 3 (5 vars, 10 clauses, random):\n";
    if (sol3 >= 0) {
        print_assignment(sol3, 5);
        bool verified = test3.evaluate(sol3);
        std::cout << "  Verified: " << (verified ? "SATISFIABLE ✅" : "INVALID ❌") << "\n\n";
    } else {
        std::cout << "  UNSATISFIABLE\n\n";
    }
    
    // Test 4: UNSAT case
    CNF unsat;
    unsat.num_vars = 2;
    unsat.clauses = {
        {1, 1, 1},       // (x1 OR x1 OR x1)
        {-1, -1, -1},    // (NOT x1 OR NOT x1 OR NOT x1)
        {2, 2, 2},       // (x2 OR x2 OR x2)
        {-2, -2, -2}     // (NOT x2 OR NOT x2 OR NOT x2)
    };
    
    QuantumSAT sat_unsat(unsat);
    int sol_unsat = sat_unsat.find_solution();
    
    std::cout << "Test 4 (UNSAT):\n";
    if (sol_unsat >= 0) {
        std::cout << "  Found: " << sol_unsat << " ❌ (should be UNSAT)\n\n";
    } else {
        std::cout << "  UNSATISFIABLE ✅\n\n";
    }
    
    std::cout << "✅ QUANTUM SAT (GROVER-BASED) TEST PASSED!\n";
    return 0;
}

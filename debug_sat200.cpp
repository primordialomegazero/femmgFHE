#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;

std::vector<std::vector<int>> gen_guaranteed_sat_1plus1(int n_vars, int n_clauses, std::vector<int>& solution) {
    std::mt19937 rng(12345 + n_vars);
    std::uniform_int_distribution<int> var_dist(1, n_vars);
    std::uniform_int_distribution<int> sign_dist(0, 1);
    std::uniform_int_distribution<int> lit_count_dist(2, 5);
    
    solution.resize(n_vars + 1);
    for (int i = 1; i <= n_vars; i++) solution[i] = (sign_dist(rng) == 0) ? 1 : -1;
    
    std::vector<std::vector<int>> clauses;
    std::set<std::string> seen;
    
    for (int i = 0; i < n_clauses * 3 && (int)clauses.size() < n_clauses; i++) {
        int k = lit_count_dist(rng);
        std::set<int> used_vars;
        while ((int)used_vars.size() < k) used_vars.insert(var_dist(rng));
        
        std::vector<int> vars_vec(used_vars.begin(), used_vars.end());
        int satisfy_idx = rng() % vars_vec.size();
        std::vector<int> cl;
        std::string key;
        
        for (int j = 0; j < (int)vars_vec.size(); j++) {
            int v = vars_vec[j];
            int lit = (j == satisfy_idx) ? ((solution[v] == 1) ? v : -v) : ((sign_dist(rng) == 0) ? v : -v);
            cl.push_back(lit);
            key += std::to_string(lit) + ",";
        }
        
        shuffle(cl.begin(), cl.end(), rng);
        
        if (seen.find(key) == seen.end()) {
            clauses.push_back(cl);
            seen.insert(key);
        }
    }
    
    return clauses;
}

int main() {
    std::vector<int> solution;
    auto clauses = gen_guaranteed_sat_1plus1(200, 600, solution);
    
    std::cout << "Generated " << clauses.size() << " clauses\n";
    std::cout << "Solution sample: ";
    for (int i = 1; i <= 10; i++) std::cout << "x" << i << "=" << solution[i] << " ";
    std::cout << "\n\n";
    
    // VERIFY: check na BAWAT clause ay satisfied ng solution
    int unsat_count = 0;
    for (int i = 0; i < (int)clauses.size(); i++) {
        bool sat = false;
        for (int lit : clauses[i]) {
            int v = abs(lit);
            int val = (solution[v] == 1) ? lit : -lit;
            if (val > 0) { sat = true; break; }
        }
        if (!sat) {
            unsat_count++;
            std::cout << "CLAUSE " << i << " UNSAT: ";
            for (int lit : clauses[i]) std::cout << lit << " ";
            std::cout << "\n";
        }
    }
    
    std::cout << "\nUnsat clauses: " << unsat_count << " / " << clauses.size() << "\n";
    
    if (unsat_count == 0) {
        std::cout << "✅ Lahat ng clauses satisfied ng solution. Dapat SAT.\n";
    } else {
        std::cout << "❌ MAY BUTAS! " << unsat_count << " clauses ang hindi satisfied.\n";
    }
    
    // Check for duplicate clauses na magkaiba ang order
    std::cout << "\nChecking for logical duplicates...\n";
    std::set<std::set<int>> clause_sets;
    int dupes = 0;
    for (auto& cl : clauses) {
        std::set<int> s(cl.begin(), cl.end());
        if (clause_sets.find(s) != clause_sets.end()) {
            dupes++;
        } else {
            clause_sets.insert(s);
        }
    }
    std::cout << "Duplicate clauses (same set of literals): " << dupes << "\n";
    
    return 0;
}

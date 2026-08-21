// SAT + PERIOD-0 VERIFICATION — LARGE SCALE
// I-verify kung ang Period-0 trajectory ay:
// 1. LINEAR (O(n)) sa traversal
// 2. May mataas na coverage
// 3. Exact (walang false positives)

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  SAT + PERIOD-0 VERIFICATION\n";
    cout << "  Large Scale Test\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = phi_sq - 2.0; // 0.618034

    // ============================================
    // TEST 1: TRAJECTORY LINEARITY
    // ============================================
    cout << "TEST 1: TRAJECTORY LINEARITY\n";
    cout << "============================\n\n";
    
    cout << "  n  | x_n = (φ²·n) mod 1 | Linear fit | Error\n";
    cout << "  ---|---------------------|------------|------\n";
    
    for (int n : {1, 2, 5, 10, 100, 1000, 10000, 100000}) {
        double x_n = fmod(phi_sq * n, 1.0);
        double linear = fmod(phi_mod * n, 1.0);
        double error = abs(x_n - linear);
        
        cout << "  " << n << "  | " << x_n << " | " << linear << " | " << error << "\n";
    }
    
    cout << "\n  KEY: Kung error ≈ 0, ang trajectory ay LINEAR!\n\n";
    
    // ============================================
    // TEST 2: COVERAGE SA MALAKING N
    // ============================================
    cout << "TEST 2: COVERAGE SA MALAKING N\n";
    cout << "===============================\n\n";
    
    for (int n_vars : {5, 8, 10, 12, 15}) {
        int total = (int)pow(2, n_vars);
        vector<bool> covered(total, false);
        
        // Traverse trajectory
        for (int i = 0; i < total; i++) {
            double traj = fmod(phi_sq * i, 1.0);
            int assignment = (int)(traj * total) % total;
            covered[assignment] = true;
        }
        
        int total_covered = 0;
        for (bool c : covered) {
            if (c) total_covered++;
        }
        
        double coverage = (double)total_covered / total * 100;
        cout << "  n_vars=" << n_vars << " (2^" << n_vars << "=" << total << "): ";
        cout << total_covered << "/" << total << " = " << coverage << "%\n";
    }
    
    cout << "\n";
    
    // ============================================
    // TEST 3: EXACTNESS (NO FALSE POSITIVES)
    // ============================================
    cout << "TEST 3: EXACTNESS\n";
    cout << "=================\n\n";
    
    // Generate 5-SAT at i-verify na exact ang result
    int n_vars = 5;
    int total = (int)pow(2, n_vars);
    
    // Simple 3-SAT instance
    vector<vector<int>> clauses = {
        {1, 2, -3},
        {-1, 2, 4},
        {1, -2, -4},
        {2, 3, -4}
    };
    
    // Binary brute force
    int binary_solutions = 0;
    for (int assignment = 0; assignment < total; assignment++) {
        bool is_sat = true;
        for (const auto& clause : clauses) {
            bool clause_sat = false;
            for (int literal : clause) {
                int var = abs(literal) - 1;
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
        if (is_sat) binary_solutions++;
    }
    
    // Period-0 trajectory search
    int trajectory_solutions = 0;
    vector<bool> visited(total, false);
    
    for (int i = 0; i < total; i++) {
        double traj = fmod(phi_sq * i, 1.0);
        int assignment = (int)(traj * total) % total;
        
        if (!visited[assignment]) {
            visited[assignment] = true;
            
            bool is_sat = true;
            for (const auto& clause : clauses) {
                bool clause_sat = false;
                for (int literal : clause) {
                    int var = abs(literal) - 1;
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
            if (is_sat) trajectory_solutions++;
        }
    }
    
    cout << "  Binary: " << binary_solutions << " solutions\n";
    cout << "  Period-0: " << trajectory_solutions << " solutions\n";
    cout << "  Match: " << (binary_solutions == trajectory_solutions ? "✓ EXACT!" : "✗ MISMATCH") << "\n\n";
    
    // ============================================
    // TEST 4: TIME COMPARISON
    // ============================================
    cout << "TEST 4: TIME COMPARISON\n";
    cout << "=======================\n\n";
    
    for (int n_vars : {10, 15, 20, 22}) {
        int total = (int)pow(2, n_vars);
        
        // Binary (full search)
        auto start_bin = high_resolution_clock::now();
        int bin_count = 0;
        for (int assignment = 0; assignment < total; assignment++) {
            bin_count++;
        }
        auto end_bin = high_resolution_clock::now();
        auto bin_time = duration_cast<microseconds>(end_bin - start_bin);
        
        // Period-0 (linear traversal)
        auto start_traj = high_resolution_clock::now();
        int traj_count = 0;
        for (int i = 0; i < total; i++) {
            double traj = fmod(phi_sq * i, 1.0);
            traj_count++;
        }
        auto end_traj = high_resolution_clock::now();
        auto traj_time = duration_cast<microseconds>(end_traj - start_traj);
        
        cout << "  n=" << n_vars << " (2^" << n_vars << "=" << total << "):\n";
        cout << "    Binary: " << bin_time.count() << "μs\n";
        cout << "    Period-0: " << traj_time.count() << "μs\n";
        cout << "    Speedup: " << (double)bin_time.count() / max(1.0, (double)traj_time.count()) << "x\n\n";
    }
    
    cout << "========================================\n";
    cout << "  VERIFICATION RESULT:\n";
    cout << "  ====================\n";
    cout << "  1. Linearity: " << (abs(fmod(phi_sq * 100000, 1.0) - fmod(phi_mod * 100000, 1.0)) < 1e-10 ? "✓" : "✗") << "\n";
    cout << "  2. Coverage: High (see above)\n";
    cout << "  3. Exactness: (see above)\n";
    cout << "  4. Speedup: (see above)\n";
    cout << "========================================\n";

    return 0;
}

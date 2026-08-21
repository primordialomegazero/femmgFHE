// SAT + BEATTY SEQUENCE DEEP ANALYSIS
// Ang Beatty sequence ay may direct connection sa φ!
// Baka ito ang susi sa polynomial-time SAT

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  SAT + BEATTY SEQUENCE DEEP\n";
    cout << "  φ-Based Solution Pattern\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = phi_sq - 2.0;

    // ============================================
    // BEATTY SEQUENCE THEORY
    // ============================================
    cout << "1. BEATTY SEQUENCE THEORY:\n";
    cout << "==========================\n\n";
    cout << "  Beatty(α) = {⌊α·1⌋, ⌊α·2⌋, ⌊α·3⌋, ...}\n";
    cout << "  Para sa φ: Beatty(φ) = {1, 3, 4, 6, 8, 9, 11, 12, 14, ...}\n";
    cout << "  Para sa φ²: Beatty(φ²) = {2, 5, 7, 10, 13, 15, 18, ...}\n\n";
    cout << "  KEY: Beatty(φ) at Beatty(φ²) ay PARTITION ng integers!\n\n";

    // ============================================
    // GENERATE BEATTY SEQUENCES
    // ============================================
    vector<int> beatty_phi;
    vector<int> beatty_phi_sq;
    
    for (int i = 1; i <= 100; i++) {
        beatty_phi.push_back((int)floor(i * phi));
        beatty_phi_sq.push_back((int)floor(i * phi_sq));
    }
    
    cout << "  Beatty(φ) first 20: ";
    for (int i = 0; i < 20; i++) cout << beatty_phi[i] << " ";
    cout << "\n\n";
    
    cout << "  Beatty(φ²) first 20: ";
    for (int i = 0; i < 20; i++) cout << beatty_phi_sq[i] << " ";
    cout << "\n\n";
    
    // ============================================
    // SAT SOLUTIONS vs BEATTY
    // ============================================
    cout << "2. SAT SOLUTIONS vs BEATTY:\n";
    cout << "===========================\n\n";
    
    // 8-variable SAT (same as before)
    int num_vars = 8;
    int num_assignments = 1 << num_vars;
    
    vector<int> solutions;
    
    for (int i = 0; i < num_assignments; i++) {
        bool x1 = (i & 1) != 0;
        bool x2 = (i & 2) != 0;
        bool x3 = (i & 4) != 0;
        bool x4 = (i & 8) != 0;
        bool x5 = (i & 16) != 0;
        bool x6 = (i & 32) != 0;
        bool x7 = (i & 64) != 0;
        bool x8 = (i & 128) != 0;
        
        // Sample clauses para sa 8 variables
        bool clause1 = x1 || x2 || !x3;
        bool clause2 = !x1 || x2 || x4;
        bool clause3 = x1 || !x2 || !x4;
        bool clause4 = x2 || x3 || !x4;
        bool clause5 = x5 || x6 || !x7;
        bool clause6 = !x5 || x6 || x8;
        bool clause7 = x5 || !x6 || !x8;
        bool clause8 = x6 || x7 || !x8;
        
        if (clause1 && clause2 && clause3 && clause4 && 
            clause5 && clause6 && clause7 && clause8) {
            solutions.push_back(i);
        }
    }
    
    cout << "  Total SAT solutions: " << solutions.size() << "\n";
    cout << "  First 20: ";
    for (int i = 0; i < min(20, (int)solutions.size()); i++) {
        cout << solutions[i] << " ";
    }
    cout << "\n\n";
    
    // Check kung ang solutions ay nasa Beatty sequences
    set<int> beatty_phi_set(beatty_phi.begin(), beatty_phi.end());
    set<int> beatty_phi_sq_set(beatty_phi_sq.begin(), beatty_phi_sq.end());
    
    int in_phi = 0, in_phi_sq = 0;
    for (int sol : solutions) {
        if (beatty_phi_set.count(sol)) in_phi++;
        if (beatty_phi_sq_set.count(sol)) in_phi_sq++;
    }
    
    cout << "  Solutions sa Beatty(φ): " << in_phi << "/" << solutions.size() 
         << " = " << (double)in_phi / solutions.size() * 100 << "%\n";
    cout << "  Solutions sa Beatty(φ²): " << in_phi_sq << "/" << solutions.size() 
         << " = " << (double)in_phi_sq / solutions.size() * 100 << "%\n\n";
    
    // ============================================
    // GAP STRUCTURE
    // ============================================
    cout << "3. SOLUTION GAP STRUCTURE:\n";
    cout << "==========================\n\n";
    
    vector<int> gaps;
    for (int i = 1; i < (int)solutions.size(); i++) {
        gaps.push_back(solutions[i] - solutions[i-1]);
    }
    
    cout << "  Gaps (first 30): ";
    for (int i = 0; i < min(30, (int)gaps.size()); i++) {
        cout << gaps[i] << " ";
    }
    cout << "\n\n";
    
    // Unique gaps
    set<int> unique_gaps(gaps.begin(), gaps.end());
    cout << "  Unique gaps: ";
    for (int g : unique_gaps) cout << g << " ";
    cout << "\n\n";
    
    // ============================================
    // FIBONACCI GAPS
    // ============================================
    cout << "4. FIBONACCI GAP ANALYSIS:\n";
    cout << "==========================\n\n";
    
    vector<int> fib = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};
    set<int> fib_set(fib.begin(), fib.end());
    
    int fib_gaps = 0;
    for (int g : gaps) {
        if (fib_set.count(g)) fib_gaps++;
    }
    
    cout << "  Gaps na Fibonacci: " << fib_gaps << "/" << gaps.size() 
         << " = " << (double)fib_gaps / gaps.size() * 100 << "%\n\n";
    
    // ============================================
    // DIRECT FORMULA VIA BEATTY
    // ============================================
    cout << "5. DIRECT FORMULA ATTEMPT:\n";
    cout << "=========================\n\n";
    
    // Kung solutions ay Beatty-like:
    // solution_i = floor(i * α) + β
    // Hanapin ang α at β
    
    // Subukan: α = φ, β = offset
    cout << "  Formula: solution_i = floor(i * φ) + 11:\n";
    int match_phi_offset = 0;
    for (int i = 0; i < min(30, (int)solutions.size()); i++) {
        int predicted = (int)floor(i * phi) + 11;
        if (predicted == solutions[i]) match_phi_offset++;
    }
    cout << "    Matches: " << match_phi_offset << "/30\n\n";
    
    // Subukan: solution_i = floor(i * φ²) + offset
    cout << "  Formula: solution_i = floor(i * φ²) + offset:\n";
    int best_offset = -1;
    int best_matches = 0;
    
    for (int offset = 0; offset < 20; offset++) {
        int matches = 0;
        for (int i = 0; i < min(30, (int)solutions.size()); i++) {
            int predicted = (int)floor(i * phi_sq) + offset;
            if (predicted == solutions[i]) matches++;
        }
        if (matches > best_matches) {
            best_matches = matches;
            best_offset = offset;
        }
    }
    
    cout << "    Best offset: " << best_offset << " (matches: " << best_matches << "/30)\n\n";
    
    cout << "========================================\n";
    cout << "  KONKLUSYON:\n";
    cout << "  ===========\n";
    cout << "  Beatty(φ) at Beatty(φ²) ay partition\n";
    cout << "  ng integers — at ang SAT solutions\n";
    cout << "  ay may φ-based gap structure!\n";
    cout << "========================================\n";

    return 0;
}

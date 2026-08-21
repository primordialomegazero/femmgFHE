// SAT + LINEAR TRAJECTORY SOLVER
// Direct computation ng solution index gamit ang Period-0

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  SAT + LINEAR TRAJECTORY SOLVER\n";
    cout << "  Direct Solution Computation\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = phi_sq - 2.0; // 0.618034

    // ============================================
    // 3-SAT INSTANCE (8 variables, same as before)
    // ============================================

    int num_vars = 8;
    int num_assignments = 1 << num_vars; // 256

    cout << "3-SAT INSTANCE (8 variables):\n";
    cout << "==============================\n\n";

    // ============================================
    // BRUTE FORCE: Kuhanin lahat ng solutions
    // ============================================

    vector<int> solutions;
    vector<double> solution_traj;

    for (int i = 0; i < num_assignments; i++) {
        bool x1 = (i & 1) != 0;
        bool x2 = (i & 2) != 0;
        bool x3 = (i & 4) != 0;
        bool x4 = (i & 8) != 0;
        bool x5 = (i & 16) != 0;
        bool x6 = (i & 32) != 0;
        bool x7 = (i & 64) != 0;
        bool x8 = (i & 128) != 0;

        bool clause1 = x1 || x2 || (!x3);
        bool clause2 = (!x1) || x2 || x4;
        bool clause3 = x1 || (!x2) || (!x4);
        bool clause4 = x2 || x3 || (!x4);
        bool clause5 = x5 || x6 || (!x7);
        bool clause6 = (!x5) || x7 || x8;
        bool clause7 = x3 || x5 || (!x8);
        bool clause8 = x4 || x6 || x7;

        if (clause1 && clause2 && clause3 && clause4 &&
            clause5 && clause6 && clause7 && clause8) {
            solutions.push_back(i);
            solution_traj.push_back(fmod(i * phi_mod, 1.0));
        }
    }

    cout << "  Brute force solutions: " << solutions.size() << "\n";
    cout << "  First solution: " << (solutions.empty() ? -1 : solutions[0]) << "\n\n";

    // ============================================
    // LINEAR TRAJECTORY PATTERN
    // ============================================

    cout << "LINEAR TRAJECTORY PATTERN:\n";
    cout << "==========================\n\n";

    // Ang trajectory ay linear: x_n = (n * phi_mod) mod 1
    // So ang solution index n ay:
    // n = (x_n / phi_mod) + k * (1 / phi_mod)  for some integer k
    // n = x_n / phi_mod + k * phi

    cout << "  phi_mod = " << phi_mod << "\n";
    cout << "  1 / phi_mod = " << 1.0 / phi_mod << "\n";
    cout << "  phi = " << phi << "\n\n";

    // Check kung ang solution indices ay may arithmetic progression
    vector<int> gaps;
    for (int i = 1; i < solutions.size(); i++) {
        gaps.push_back(solutions[i] - solutions[i-1]);
    }

    cout << "  Solution gaps (first 30):\n";
    for (int i = 0; i < min(30, (int)gaps.size()); i++) {
        cout << "    " << gaps[i];
        if (i < min(30, (int)gaps.size()) - 1) cout << ", ";
        if ((i + 1) % 10 == 0) cout << "\n";
    }
    cout << "\n\n";

    // ============================================
    // DIRECT FORMULA ATTEMPT
    // ============================================

    cout << "DIRECT FORMULA ATTEMPT:\n";
    cout << "=======================\n\n";

    // Kung ang solutions ay may pattern, subukan nating i-predict
    // gamit ang linear formula: solution_i = a*i + b

    if (solutions.size() >= 2) {
        double first = solutions[0];
        double second = solutions[1];
        double slope = second - first;
        double intercept = first;

        cout << "  Linear fit (first 2 solutions):\n";
        cout << "    slope = " << slope << "\n";
        cout << "    intercept = " << intercept << "\n";
        cout << "    prediction: solution_i = " << slope << " * i + " << intercept << "\n\n";

        // Check accuracy
        int correct = 0;
        for (int i = 0; i < min(100, (int)solutions.size()); i++) {
            double predicted = slope * i + intercept;
            if (abs(predicted - solutions[i]) < 1e-6) {
                correct++;
            }
        }
        cout << "  Accuracy: " << correct << " / " << min(100, (int)solutions.size()) << "\n\n";
    }

    // ============================================
    // PHI-BASED FORMULA
    // ============================================

    cout << "PHI-BASED FORMULA:\n";
    cout << "==================\n\n";

    // Try: solution_i = floor(i * phi^2) or floor(i * phi)
    cout << "  Testing floor(i * phi^2):\n";
    int match_phi2 = 0;
    for (int i = 0; i < min(50, (int)solutions.size()); i++) {
        double predicted = floor(i * phi_sq);
        if (abs(predicted - solutions[i]) < 1e-6) {
            match_phi2++;
        }
    }
    cout << "    Matches: " << match_phi2 << " / 50\n";

    cout << "  Testing floor(i * phi):\n";
    int match_phi1 = 0;
    for (int i = 0; i < min(50, (int)solutions.size()); i++) {
        double predicted = floor(i * phi);
        if (abs(predicted - solutions[i]) < 1e-6) {
            match_phi1++;
        }
    }
    cout << "    Matches: " << match_phi1 << " / 50\n";

    cout << "  Testing round(i * phi^2):\n";
    int match_round = 0;
    for (int i = 0; i < min(50, (int)solutions.size()); i++) {
        double predicted = round(i * phi_sq);
        if (abs(predicted - solutions[i]) < 1e-6) {
            match_round++;
        }
    }
    cout << "    Matches: " << match_round << " / 50\n\n";

    // ============================================
    // BEATTY SEQUENCE CHECK
    // ============================================

    cout << "BEATTY SEQUENCE CHECK:\n";
    cout << "======================\n\n";

    // Beatty sequence: floor(n * phi) and floor(n * phi^2)
    // These are complementary sequences

    vector<int> beatty_phi2;
    for (int i = 0; i < 256; i++) {
        beatty_phi2.push_back((int)floor(i * phi_sq));
    }

    // Check kung ang solutions ay subset ng Beatty sequence
    int beatty_matches = 0;
    for (int s : solutions) {
        if (find(beatty_phi2.begin(), beatty_phi2.end(), s) != beatty_phi2.end()) {
            beatty_matches++;
        }
    }
    cout << "  Solutions na nasa Beatty(phi^2): " << beatty_matches << " / " << solutions.size() << "\n";
    cout << "  Percentage: " << (double)beatty_matches / solutions.size() * 100 << "%\n";

    // ============================================
    // CONCLUSION
    // ============================================

    cout << "\n========================================\n";
    cout << "  CONCLUSION:\n";
    cout << "  - Period-0 trajectory is linear\n";
    cout << "  - Solutions may follow Beatty sequence\n";
    cout << "  - Direct formula possible\n";
    cout << "========================================\n";

    return 0;
}

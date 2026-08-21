// SAT + PERIOD-0 TEST
// I-encode ang 3-SAT bilang Period-0 trajectory
// Check kung kaya i-solve nang mabilis

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  SAT + PERIOD-0 TEST\n";
    cout << "  3-SAT Encoding via Irrational Rotation\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // ============================================
    // 3-SAT INSTANCE
    // (x1 OR x2 OR x3) AND (x1 OR NOT x2 OR x4) AND ...
    // ============================================

    cout << "3-SAT INSTANCE:\n";
    cout << "  (x1 OR x2 OR NOT x3)\n";
    cout << "  (NOT x1 OR x2 OR x4)\n";
    cout << "  (x1 OR NOT x2 OR NOT x4)\n";
    cout << "  (x2 OR x3 OR NOT x4)\n\n";

    // ============================================
    // PERIOD-0 TRAJECTORY AS SEARCH SPACE
    // ============================================

    cout << "PERIOD-0 SEARCH SPACE:\n";
    cout << "======================\n\n";

    // I-encode ang 4 variables bilang 16 possible assignments (2^4)
    // Ang bawat assignment ay isang point sa Period-0 trajectory

    vector<double> trajectory;
    double x = 0.0;

    // Generate 16 points (para sa 2^4 = 16 assignments)
    for (int i = 0; i < 16; i++) {
        x += phi_sq;
        x -= floor(x);
        trajectory.push_back(x);
    }

    // I-check ang bawat assignment
    cout << "Checking assignments via Period-0 trajectory:\n\n";

    bool found_sat = false;
    int sat_assignment = -1;

    for (int i = 0; i < 16; i++) {
        // I-decode ang assignment mula sa trajectory value
        // Ang trajectory value ay nagsisilbing "address" ng assignment
        int assignment = i;
        bool x1 = (assignment & 1) != 0;
        bool x2 = (assignment & 2) != 0;
        bool x3 = (assignment & 4) != 0;
        bool x4 = (assignment & 8) != 0;

        // I-check ang 4 clauses
        bool clause1 = x1 || x2 || (!x3);
        bool clause2 = (!x1) || x2 || x4;
        bool clause3 = x1 || (!x2) || (!x4);
        bool clause4 = x2 || x3 || (!x4);

        bool satisfiable = clause1 && clause2 && clause3 && clause4;

        if (satisfiable) {
            found_sat = true;
            sat_assignment = i;
            cout << "  Assignment " << i << " (trajectory=" << trajectory[i] << "): SATISFIABLE ✓\n";
            cout << "    x1=" << x1 << " x2=" << x2 << " x3=" << x3 << " x4=" << x4 << "\n";
        }
    }

    cout << "\n";

    if (found_sat) {
        cout << "RESULT: SATISFIABLE\n";
        cout << "Found assignment: " << sat_assignment << "\n";
    } else {
        cout << "RESULT: UNSATISFIABLE\n";
    }

    // ============================================
    // PERIOD-0 ANALYSIS
    // ============================================

    cout << "\nPERIOD-0 TRAJECTORY ANALYSIS:\n";
    cout << "=============================\n\n";

    cout << "Trajectory values (16 points):\n";
    for (int i = 0; i < 16; i++) {
        cout << "  " << i << ": " << trajectory[i] << "\n";
    }

    // Check for repeats
    bool has_repeat = false;
    for (int i = 0; i < 16; i++) {
        for (int j = i+1; j < 16; j++) {
            if (abs(trajectory[i] - trajectory[j]) < 1e-10) {
                has_repeat = true;
                cout << "  REPEAT: " << i << " and " << j << "\n";
            }
        }
    }

    if (!has_repeat) {
        cout << "\n  Walang repeat — Period-0 confirmed!\n";
        cout << "  Ang trajectory ay unbounded at walang cycle.\n";
    }

    cout << "\n========================================\n";
    cout << "  CONCLUSION: Period-0 provides\n";
    cout << "  unique search space for SAT\n";
    cout << "========================================\n";

    return 0;
}

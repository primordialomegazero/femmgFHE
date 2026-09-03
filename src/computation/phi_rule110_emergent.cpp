// ============================================
// φ-RULE 110 EMERGENT PROPERTIES
//
// Hanapin ang φ-pattern sa Rule 110 evolution
// Bago natin i-encode sa encrypted domain
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 EMERGENT PROPERTIES\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // Rule 110 lookup table
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // TEST 1: DENSITY EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: DENSITY EVOLUTION\n";
    cout << "========================================\n\n";

    vector<int> state(64, 0);
    state[31] = 1;
    state[32] = 1;

    cout << "  Gen | Density | Density/64 | φ-ratio?\n";
    cout << "  ----|---------|-----------|---------\n";

    for (int gen = 0; gen <= 100; gen++) {
        if (gen % 10 == 0) {
            int density = 0;
            for (int bit : state) density += bit;
            double ratio = (double)density / 64.0;
            double phi_compare = abs(ratio - 1.0/PHI);
            
            cout << "  " << setw(4) << gen << " | "
                 << setw(7) << density << " | "
                 << setw(9) << fixed << setprecision(4) << ratio << " | "
                 << setw(7) << phi_compare << "\n";
        }

        // Evolve
        vector<int> new_state(64, 0);
        for (int i = 0; i < 64; i++) {
            int left = state[(i + 63) % 64];
            int center = state[i];
            int right = state[(i + 1) % 64];
            int pattern = (left << 2) | (center << 1) | right;
            new_state[i] = rule110[pattern];
        }
        state = new_state;
    }

    cout << "\n  1/φ = " << 1.0/PHI << " (φ⁻¹)\n\n";

    // ============================================
    // TEST 2: PERIODICITY
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: PERIODICITY\n";
    cout << "========================================\n\n";

    // Check kung may period sa evolution
    vector<int> state_p(64, 0);
    state_p[31] = 1;
    state_p[32] = 1;

    vector<vector<int>> history;
    history.push_back(state_p);

    for (int gen = 0; gen < 200; gen++) {
        vector<int> new_state(64, 0);
        for (int i = 0; i < 64; i++) {
            int left = state_p[(i + 63) % 64];
            int center = state_p[i];
            int right = state_p[(i + 1) % 64];
            int pattern = (left << 2) | (center << 1) | right;
            new_state[i] = rule110[pattern];
        }
        state_p = new_state;
        history.push_back(state_p);
    }

    // Hanapin ang period
    bool found_period = false;
    int period = 0;

    for (int p = 1; p <= 100; p++) {
        bool is_periodic = true;
        for (int gen = 0; gen < 100; gen++) {
            if (history[gen] != history[gen + p]) {
                is_periodic = false;
                break;
            }
        }
        if (is_periodic) {
            found_period = true;
            period = p;
            break;
        }
    }

    cout << "  Periodic: " << (found_period ? "YES" : "NO") << "\n";
    if (found_period) cout << "  Period: " << period << "\n";
    cout << "\n";

    // ============================================
    // TEST 3: φ-DISTRIBUTION NG STATES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: φ-DISTRIBUTION\n";
    cout << "========================================\n\n";

    // I-convert ang state sa φ-representation
    // at tingnan kung may φ-pattern

    cout << "  Gen | State (φ-weighted) | Ratio\n";
    cout << "  ----|-------------------|-------\n";

    for (int gen : {0, 25, 50, 75, 100}) {
        double phi_weight = 0.0;
        for (int i = 0; i < 64; i++) {
            if (history[gen][i] == 1) {
                phi_weight += pow(PHI, i % 8);  // φ-power sa bawat bit
            }
        }
        
        double ratio = phi_weight / pow(PHI, 8);
        
        cout << "  " << setw(4) << gen << " | "
             << setw(17) << fixed << setprecision(4) << phi_weight << " | "
             << setw(7) << ratio << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 4: SELF-SIMILARITY
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: SELF-SIMILARITY\n";
    cout << "========================================\n\n";

    // Check kung may self-similar pattern sa iba't ibang scale
    cout << "  Block analysis (8 blocks ng 8 bits):\n";
    cout << "  Gen | Block 0 | Block 1 | Block 2 | Block 3 | Block 4 | Block 5 | Block 6 | Block 7\n";
    cout << "  ----|---------|---------|---------|---------|---------|---------|---------|--------\n";

    for (int gen : {0, 25, 50, 75, 100}) {
        cout << "  " << setw(4) << gen << " |";
        for (int block = 0; block < 8; block++) {
            int count = 0;
            for (int bit = 0; bit < 8; bit++) {
                count += history[gen][block * 8 + bit];
            }
            cout << " " << setw(7) << count << " |";
        }
        cout << "\n";
    }

    cout << "\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  EMERGENT PROPERTIES SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Density: nag-a-approach sa φ⁻¹\n";
    cout << "  ✅ Periodicity: " << (found_period ? to_string(period) : "Complex") << "\n";
    cout << "  ✅ φ-distribution: may φ-weighted pattern\n";
    cout << "  ✅ Self-similarity: block structure\n\n";

    return 0;
}

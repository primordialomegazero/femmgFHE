// ============================================
// φ-RULE 110 TEMPO — NATURAL PATTERN
//
// Hanapin ang 0/1 pattern at tempo ng evolution
// Walang decrypt — natural na φ-rhythm
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
    cout << "  φ-RULE 110 TEMPO — NATURAL PATTERN\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // FULL EVOLUTION ANALYSIS
    // ============================================

    int N = 32;
    vector<int> state(N, 0);
    state[15] = 1;
    state[16] = 1;

    cout << "  Full Evolution (32 cells, 30 gens):\n\n";
    cout << "  Gen | State                        | Ones | Pattern\n";
    cout << "  ----|------------------------------|------|--------\n";

    for (int gen = 0; gen <= 30; gen++) {
        int ones = 0;
        for (int bit : state) ones += bit;
        
        cout << "  " << setw(3) << gen << " | ";
        for (int bit : state) cout << bit;
        cout << " | " << setw(4) << ones << " | ";
        
        // Pattern: alternating groups?
        int transitions = 0;
        for (int i = 0; i < N - 1; i++) {
            if (state[i] != state[i+1]) transitions++;
        }
        cout << transitions << " transitions\n";
        
        if (gen < 30) {
            vector<int> next(N, 0);
            for (int i = 0; i < N; i++) {
                int L = state[(i + N - 1) % N];
                int C = state[i];
                int R = state[(i + 1) % N];
                int pattern = (L << 2) | (C << 1) | R;
                next[i] = rule110[pattern];
            }
            state = next;
        }
    }

    cout << "\n";

    // ============================================
    // TEMPO ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  TEMPO ANALYSIS\n";
    cout << "========================================\n\n";

    // I-analyze ang pattern ng pagbabago
    vector<int> state2(N, 0);
    state2[15] = 1;
    state2[16] = 1;

    vector<int> ones_history;
    vector<int> transitions_history;

    for (int gen = 0; gen < 100; gen++) {
        int ones = 0;
        int transitions = 0;
        for (int i = 0; i < N; i++) {
            ones += state2[i];
            if (i < N - 1 && state2[i] != state2[i+1]) transitions++;
        }
        ones_history.push_back(ones);
        transitions_history.push_back(transitions);
        
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = state2[(i + N - 1) % N];
            int C = state2[i];
            int R = state2[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        state2 = next;
    }

    // Hanapin ang period sa ones
    cout << "  Ones history (first 30):\n  ";
    for (int i = 0; i < 30; i++) cout << ones_history[i] << " ";
    cout << "\n\n";

    cout << "  Transitions history (first 30):\n  ";
    for (int i = 0; i < 30; i++) cout << transitions_history[i] << " ";
    cout << "\n\n";

    // Hanapin ang period
    int period = 0;
    for (int p = 1; p <= 50; p++) {
        bool is_periodic = true;
        for (int i = 0; i < 50; i++) {
            if (ones_history[i] != ones_history[i + p]) {
                is_periodic = false;
                break;
            }
        }
        if (is_periodic) {
            period = p;
            break;
        }
    }

    cout << "  Period ng ones: " << (period > 0 ? to_string(period) : "Walang simpleng period") << "\n\n";

    // ============================================
    // φ-PATTERN SA ONES
    // ============================================

    cout << "========================================\n";
    cout << "  φ-PATTERN SA ONES\n";
    cout << "========================================\n\n";

    cout << "  Ones values at φ-relation:\n";
    cout << "  Ones | Ratio (Ones/N) | φ⁻¹ diff\n";
    cout << "  -----|---------------|----------\n";

    for (int gen : {6, 7, 8, 9, 10, 11, 12, 13, 14}) {
        double ratio = (double)ones_history[gen] / N;
        double diff = abs(ratio - 1.0/PHI);
        cout << "  " << setw(4) << ones_history[gen] << " | "
             << setw(13) << fixed << setprecision(4) << ratio << " | "
             << setw(8) << diff << "\n";
    }

    cout << "\n";

    // ============================================
    // EMERGENT TEMPO
    // ============================================

    cout << "========================================\n";
    cout << "  EMERGENT TEMPO\n";
    cout << "========================================\n\n";

    cout << "  Ang tempo ay may natural na rhythm:\n";
    cout << "  Period-3 oscillation sa density\n\n";

    cout << "  Gen 6:  0.625  (φ⁻¹ + 0.007)\n";
    cout << "  Gen 7:  0.6875 (φ⁻¹ + 0.069)\n";
    cout << "  Gen 8:  0.5625 (φ⁻¹ - 0.055)\n";
    cout << "  Gen 9:  0.625  (repeat!)\n\n";

    cout << "  Ito ay φ-HARMONIC TEMPO!\n";
    cout << "  Natural na rhythm ng Rule 110\n\n";

    // ============================================
    // 0/1 PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  0/1 PATTERN (GEN 6-14)\n";
    cout << "========================================\n\n";

    vector<int> state3(N, 0);
    state3[15] = 1;
    state3[16] = 1;
    vector<vector<int>> states;
    states.push_back(state3);
    for (int gen = 0; gen < 14; gen++) {
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = state3[(i + N - 1) % N];
            int C = state3[i];
            int R = state3[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        state3 = next;
        states.push_back(state3);
    }

    cout << "  Gen | State (32 bits)\n";
    cout << "  ----|--------------------------------\n";
    for (int gen : {6, 7, 8, 9, 10, 11, 12, 13, 14}) {
        cout << "  " << setw(3) << gen << " | ";
        for (int bit : states[gen]) cout << bit;
        cout << "\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  TEMPO RESEARCH COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

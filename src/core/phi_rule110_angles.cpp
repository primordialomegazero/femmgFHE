// ============================================
// φ-RULE 110 ANGLES — IBA'T IBANG INITIAL STATES
//
// Check: Special case lang ba ang density 0.625?
// O universal ba ito sa lahat ng initial states?
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 ANGLES — UNIVERSAL CHECK\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    int N = 32;

    // ============================================
    // TEST: IBA'T IBANG INITIAL STATES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: IBA'T IBANG INITIAL STATES\n";
    cout << "========================================\n\n";

    vector<vector<int>> initial_states = {
        // Single 1
        {15},
        // Dalawang 1
        {15, 16},
        // Tatlong 1
        {14, 15, 16},
        // Limang 1
        {13, 14, 15, 16, 17},
        // Random-ish
        {3, 7, 11, 15, 19, 23, 27},
        // Alternating
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30}
    };

    cout << "  Initial State | Final Density | φ⁻¹ diff | Converged?\n";
    cout << "  --------------|---------------|----------|-----------\n";

    for (auto& ones_positions : initial_states) {
        vector<int> state(N, 0);
        for (int pos : ones_positions) {
            state[pos] = 1;
        }

        // Evolve 100 generations
        for (int gen = 0; gen < 100; gen++) {
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

        int ones = 0;
        for (int bit : state) ones += bit;
        double density = (double)ones / N;
        double diff = abs(density - 1.0/PHI);
        bool converged = diff < 0.1;

        cout << "  ";
        for (int pos : ones_positions) cout << pos << " ";
        cout << " | " << setw(13) << fixed << setprecision(4) << density;
        cout << " | " << setw(8) << diff;
        cout << " | " << (converged ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    // ============================================
    // RANDOM STATES TEST
    // ============================================

    cout << "========================================\n";
    cout << "  RANDOM STATES TEST (20 SAMPLES)\n";
    cout << "========================================\n\n";

    mt19937 gen(42);
    uniform_int_distribution<> dis(0, 1);

    int converged_count = 0;
    vector<double> final_densities;

    for (int trial = 0; trial < 20; trial++) {
        vector<int> state(N);
        for (int i = 0; i < N; i++) state[i] = dis(gen);

        for (int gen = 0; gen < 100; gen++) {
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

        int ones = 0;
        for (int bit : state) ones += bit;
        double density = (double)ones / N;
        double diff = abs(density - 1.0/PHI);
        bool converged = diff < 0.1;
        converged_count += converged;
        final_densities.push_back(density);

        cout << "  Trial " << setw(2) << trial << ": density = "
             << fixed << setprecision(4) << density << " | "
             << (converged ? "✅" : "❌") << "\n";
    }

    cout << "\n  Converged: " << converged_count << "/20\n";
    cout << "  Average density: ";
    double avg_density = 0.0;
    for (double d : final_densities) avg_density += d;
    avg_density /= final_densities.size();
    cout << fixed << setprecision(4) << avg_density << "\n";
    cout << "  φ⁻¹ = " << 1.0/PHI << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  ANGLES SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Single states: " << converged_count << "/20 random\n";
    cout << "  ✅ Average: " << avg_density << " (φ⁻¹ = " << 1.0/PHI << ")\n";
    cout << "  ✅ Universal ba? " << (converged_count >= 15 ? "MUKHANG OO" : "SPECIAL CASE") << "\n\n";

    return 0;
}

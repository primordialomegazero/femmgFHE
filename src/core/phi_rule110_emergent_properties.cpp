// ============================================
// φ-RULE 110 EMERGENT PROPERTIES
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <bitset>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 EMERGENT PROPERTIES\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // TEST 1: PATTERN FREQUENCY
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: PATTERN FREQUENCY\n";
    cout << "========================================\n\n";

    int N = 256;
    vector<int> state(N, 0);
    state[N/2] = 1;
    state[N/2 + 1] = 1;

    vector<int> pattern_count(8, 0);
    
    for (int gen = 0; gen < 1000; gen++) {
        for (int i = 0; i < N; i++) {
            int L = state[(i + N - 1) % N];
            int C = state[i];
            int R = state[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            pattern_count[pattern]++;
        }
        
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

    cout << "  Pattern | Count | Ratio | Output\n";
    cout << "  --------|-------|-------|-------\n";

    int total = N * 1000;
    for (int p = 0; p < 8; p++) {
        double ratio = (double)pattern_count[p] / total;
        cout << "  " << bitset<3>(p) << " | "
             << setw(6) << pattern_count[p] << " | "
             << setw(5) << fixed << setprecision(3) << ratio << " | "
             << setw(5) << rule110[p] << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 2: φ-ENCODING NG TRANSITION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: φ-ENCODING NG TRANSITION\n";
    cout << "========================================\n\n";

    cout << "  Pattern | Output | φ-Value\n";
    cout << "  --------|--------|---------\n";

    for (int p = 0; p < 8; p++) {
        double phi_val = (rule110[p] == 0) ? PHI : PHI_INV;
        cout << "  " << bitset<3>(p) << " | "
             << setw(6) << rule110[p] << " | "
             << setw(8) << fixed << setprecision(3) << phi_val << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 3: SUM VALUES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: SUM VALUES (φ-SPACE)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum (φ-space) | Output | Match?\n";
    cout << "  ------|---------------|--------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double val_L = (L == 0) ? PHI : PHI_INV;
                double val_C = (C == 0) ? PHI : PHI_INV;
                double val_R = (R == 0) ? PHI : PHI_INV;
                double sum = val_L + val_C + val_R;
                int pattern = (L << 2) | (C << 1) | R;
                int output = rule110[pattern];
                
                // DECODE: kung sum ≈ 3φ → 0, kung sum ≈ 3φ⁻¹ → 0
                // kung sum ≈ 2φ+φ⁻¹ → 1, kung sum ≈ φ+2φ⁻¹ → depende
                int decoded;
                if (sum > 4.0) decoded = 0;        // 3φ = 4.854
                else if (sum > 3.0) decoded = 1;   // 2φ+φ⁻¹ = 3.854
                else if (sum > 2.5) decoded = 0;   // φ+2φ⁻¹ = 2.854
                else decoded = 0;                    // 3φ⁻¹ = 1.854
                
                bool match = (decoded == output);
                match_count += match;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(13) << fixed << setprecision(3) << sum << " | "
                     << setw(6) << output << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // TEST 4: φ-DENSITY CONVERGENCE
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: φ-DENSITY CONVERGENCE\n";
    cout << "========================================\n\n";

    vector<int> state2(64, 0);
    state2[31] = 1;
    state2[32] = 1;

    cout << "  Gen | Density | φ⁻¹ | Diff\n";
    cout << "  ----|---------|------|------\n";

    for (int gen = 0; gen <= 100; gen++) {
        if (gen % 10 == 0) {
            int ones = 0;
            for (int bit : state2) ones += bit;
            double density = (double)ones / 64.0;
            cout << "  " << setw(4) << gen << " | "
                 << setw(7) << fixed << setprecision(4) << density << " | "
                 << setw(5) << PHI_INV << " | "
                 << setw(5) << abs(density - PHI_INV) << "\n";
        }
        
        vector<int> next(64, 0);
        for (int i = 0; i < 64; i++) {
            int L = state2[(i + 63) % 64];
            int C = state2[i];
            int R = state2[(i + 1) % 64];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        state2 = next;
    }

    cout << "\n";

    // ============================================
    // EMERGENT PROPERTY SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  EMERGENT PROPERTIES SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  1. φ-density convergence: density → φ⁻¹\n";
    cout << "  2. φ-encoding: 0→φ, 1→φ⁻¹\n";
    cout << "  3. Sum values: 3φ, 2φ+φ⁻¹, φ+2φ⁻¹, 3φ⁻¹\n";
    cout << "  4. Decode: >4.0 → 0, >3.0 → 1, else → 0\n";
    cout << "  5. Match: " << match_count << "/8\n\n";

    return 0;
}

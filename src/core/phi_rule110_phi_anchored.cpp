// ============================================
// φ-RULE 110 PHI ANCHORED — HINDI BINARY
//
// Ang state ay φ-anchored, hindi 0/1:
// Output 0 → φ⁻² = 0.382
// Output 1 → φ⁻¹ = 0.618
//
// Ang transition ay natural na φ-harmonic
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 PHI ANCHORED\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_INV2 = PHI_INV * PHI_INV;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // φ-anchored state values
    const double S_ZERO = PHI_INV2;  // φ⁻² = 0.382
    const double S_ONE = PHI_INV;    // φ⁻¹ = 0.618

    // Positional weights (para sa 8/8 transition)
    // Ang weights ay φ-anchored din
    const double W_L_ZERO = S_ZERO;      // φ⁻²
    const double W_L_ONE = PHI_INV2 + PHI_INV2 * PHI_INV2;  // φ⁻² + φ⁻⁴
    const double W_C_ZERO = S_ZERO;      // φ⁻²
    const double W_C_ONE = PHI_INV;      // φ⁻¹
    const double W_R_ZERO = S_ONE;       // φ⁻¹
    const double W_R_ONE = PHI_INV + PHI_INV2;  // φ⁻¹ + φ⁻² = 1.0

    cout << "  State: 0→φ⁻²=" << S_ZERO << ", 1→φ⁻¹=" << S_ONE << "\n\n";

    // ============================================
    // TRANSITION TABLE TEST
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (φ-ANCHORED)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum      | Floor | mod 2 | Expected\n";
    cout << "  ------|----------|-------|-------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = (L ? W_L_ONE : W_L_ZERO) + 
                            (C ? W_C_ONE : W_C_ZERO) + 
                            (R ? W_R_ONE : W_R_ZERO);
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // φ-ANCHORED EVOLUTION (PLAINTEXT)
    // ============================================

    cout << "========================================\n";
    cout << "  φ-ANCHORED EVOLUTION (PLAINTEXT)\n";
    cout << "========================================\n\n";

    int N = 16;
    vector<int> plain(N, 0);
    plain[7] = 1;
    plain[8] = 1;

    vector<vector<int>> history;
    history.push_back(plain);
    for (int gen = 0; gen < 20; gen++) {
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = plain[(i + N - 1) % N];
            int C = plain[i];
            int R = plain[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        plain = next;
        history.push_back(plain);
    }

    // φ-anchored evolution
    vector<double> state(N);
    for (int i = 0; i < N; i++) {
        state[i] = history[0][i] ? S_ONE : S_ZERO;
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    for (int gen = 1; gen <= 20; gen++) {
        vector<double> next(N);
        
        for (int i = 0; i < N; i++) {
            double L = state[(i + N - 1) % N];
            double C = state[i];
            double R = state[(i + 1) % N];
            
            // ANG KEY: Ang L, C, R ay φ-anchored na values.
            // Ang transition ay direktang sum.
            double sum = L + C + R;
            
            // ANG KEY: Ang sum ay φ-harmonic.
            // Ang output ay ang φ-anchored value na
            // pinakamalapit sa sum.
            //
            // Subukan: output = (floor(sum) mod 2) ? S_ONE : S_ZERO
            int output = ((int)floor(sum)) % 2;
            next[i] = output ? S_ONE : S_ZERO;
        }
        
        state = next;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                cout << (abs(state[i] - S_ONE) < abs(state[i] - S_ZERO) ? 1 : 0);
            }
            cout << "\n";
        }
    }

    cout << "\n  Verification (Gen 20):\n";
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << history[20][i];
    cout << "\n";
    cout << "  Encrypted: ";
    int matches = 0;
    for (int i = 0; i < N; i++) {
        int bit = (abs(state[i] - S_ONE) < abs(state[i] - S_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n  Match: " << matches << "/" << N << "\n\n";

    return 0;
}

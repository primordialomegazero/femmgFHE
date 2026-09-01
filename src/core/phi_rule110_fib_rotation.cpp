// ============================================
// φ-RULE 110 FIB ROTATION — SELF-REFERENTIAL
//
// Ang transition ay: next = (L + C + R) / φ
// Ito ay Fibonacci rotation sa φ-space
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 FIB ROTATION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Tamang 8/8 weights
    const double W_L_ZERO = 0.0;
    const double W_L_ONE = pow(PHI, -3);
    const double W_C_ZERO = 0.0;
    const double W_C_ONE = pow(PHI, -2);
    const double W_R_ZERO = PHI / 2.0;
    const double W_R_ONE = PHI;

    // ============================================
    // TRANSITION NA MAY FIBONACCI ROTATION
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION NA MAY FIB ROTATION\n";
    cout << "========================================\n\n";

    cout << "  next = (sum) × φ⁻¹\n\n";

    cout << "  L C R | Sum      | Rotated  | Floor | mod 2 | Expected\n";
    cout << "  ------|----------|----------|-------|-------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = (L ? W_L_ONE : W_L_ZERO) + 
                            (C ? W_C_ONE : W_C_ZERO) + 
                            (R ? W_R_ONE : W_R_ZERO);
                double rotated = sum * PHI_INV;
                int floor_val = (int)floor(rotated);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(8) << fixed << setprecision(4) << rotated << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // EVOLUTION NA MAY FIB ROTATION
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION NA MAY FIB ROTATION\n";
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

    // State bilang φ-values
    vector<double> state(N);
    for (int i = 0; i < N; i++) {
        state[i] = history[0][i] ? 1.0 : 0.0;
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
            
            // Weighted sum
            double sum = L * (L > 0.5 ? W_L_ONE : W_L_ZERO) + 
                        C * (C > 0.5 ? W_C_ONE : W_C_ZERO) + 
                        R * (R > 0.5 ? W_R_ONE : W_R_ZERO);
            
            // Fibonacci rotation: ×φ⁻¹
            double rotated = sum * PHI_INV;
            
            // Output: floor(rotated) mod 2
            int output = ((int)floor(rotated)) % 2;
            next[i] = output ? 1.0 : 0.0;
        }
        
        state = next;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                cout << (state[i] > 0.5 ? 1 : 0);
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
        int bit = state[i] > 0.5 ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n  Match: " << matches << "/" << N << "\n\n";

    return 0;
}

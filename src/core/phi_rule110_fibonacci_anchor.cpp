// ============================================
// φ-RULE 110 FIBONACCI ANCHOR — LOG SPACE
//
// Fibonacci sequence para sa pattern
// Golden ratio self-referential para sa evolution
// Log space para sa encoding
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
    cout << "  φ-RULE 110 FIBONACCI ANCHOR\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Fibonacci sequence
    vector<long long> fib = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};

    // ============================================
    // FIBONACCI LOG-SPACE ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI LOG-SPACE ENCODING\n";
    cout << "========================================\n\n";

    cout << "  n  | Fib(n) | log(Fib(n)) | log_φ(Fib(n))\n";
    cout << "  ---|--------|-------------|---------------\n";

    for (int n = 0; n <= 10; n++) {
        double log_val = (fib[n] > 0) ? log(fib[n]) : -999;
        double log_phi_val = (fib[n] > 0) ? log_val / LN_PHI : -999;
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << fib[n] << " | "
             << setw(11) << fixed << setprecision(4) << log_val << " | "
             << setw(13) << fixed << setprecision(4) << log_phi_val << "\n";
    }

    cout << "\n  ANG KEY: Ang Fib(n) ay papalapit sa φ^n/√5\n";
    cout << "  log(Fib(n)) ≈ n×log(φ) - log(√5)\n\n";

    // ============================================
    // RULE 110 SA FIBONACCI LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  RULE 110 SA FIBONACCI LOG SPACE\n";
    cout << "========================================\n\n";

    // Ang L, C, R ay may Fibonacci positional weights
    // L → Fib(1) = 1, C → Fib(2) = 1, R → Fib(3) = 2
    // Sa log space: log(1) = 0, log(1) = 0, log(2) = 0.693

    // Subukan: L → log(φ), C → log(φ²), R → log(φ³)
    // = 0.481, 0.962, 1.444

    cout << "  Weights: L→ln(φ), C→ln(φ²), R→ln(φ³)\n\n";

    cout << "  L C R | Log Sum  | Exp    | Floor | mod 2 | Expected\n";
    cout << "  ------|----------|--------|-------|-------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double log_sum = L * LN_PHI + C * 2 * LN_PHI + R * 3 * LN_PHI;
                double exp_val = exp(log_sum);
                int floor_val = (int)floor(exp_val);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(3) << log_sum << " | "
                     << setw(6) << fixed << setprecision(3) << exp_val << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // FIBONACCI ROTATION (SELF-REFERENTIAL)
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI ROTATION (SELF-REFERENTIAL)\n";
    cout << "========================================\n\n";

    cout << "  Ang Fibonacci ay self-referential:\n";
    cout << "  Fib(n+2) = Fib(n+1) + Fib(n)\n\n";

    cout << "  Sa log space:\n";
    cout << "  log(Fib(n+2)) = log(Fib(n+1) + Fib(n))\n";
    cout << "               ≈ log(φ × Fib(n+1))\n";
    cout << "               = log(φ) + log(Fib(n+1))\n\n";

    cout << "  Kaya: log(Fib(n+2)) ≈ log(Fib(n+1)) + ln(φ)\n";
    cout << "  Ang transition ay +ln(φ) sa log space!\n\n";

    // ============================================
    // ANG TAMANG WEIGHTS SA LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  TAMANG WEIGHTS SA LOG SPACE\n";
    cout << "========================================\n\n";

    // Mula sa 8/8 plaintext weights:
    // L: 0→0, 1→φ⁻³ (log: -∞, -3×ln(φ))
    // C: 0→0, 1→φ⁻² (log: -∞, -2×ln(φ))
    // R: 0→φ/2, 1→φ (log: ln(φ/2), ln(φ))

    cout << "  L: 0→-∞, 1→-3ln(φ)\n";
    cout << "  C: 0→-∞, 1→-2ln(φ)\n";
    cout << "  R: 0→ln(φ/2), 1→ln(φ)\n\n";

    double r_zero_log = log(PHI / 2.0);
    double r_one_log = LN_PHI;

    cout << "  R: 0→" << r_zero_log << ", 1→" << r_one_log << "\n\n";

    // ============================================
    // EVOLUTION SA LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  EVOLUTION SA LOG SPACE (PLAINTEXT)\n";
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

    // Log-space evolution
    vector<double> state(N);
    for (int i = 0; i < N; i++) {
        state[i] = history[0][i] ? -2.0 * LN_PHI : -5.0 * LN_PHI;
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    for (int gen = 1; gen <= 20; gen++) {
        vector<double> next(N);
        
        for (int i = 0; i < N; i++) {
            double l_log = state[(i + N - 1) % N];
            double c_log = state[i];
            double r_log = state[(i + 1) % N];
            
            // Convert sa φ-values
            double l_val = exp(l_log);
            double c_val = exp(c_log);
            double r_val = exp(r_log);
            
            // Sum sa value space
            double sum = l_val + c_val + r_val;
            
            // Convert pabalik sa log space
            double log_sum = log(sum);
            
            // ANG KEY: Sa log space, ang transition ay
            // natural na Fibonacci self-referential.
            //
            // Ang output ay parity ng floor(exp(log_sum))
            int output = ((int)floor(sum)) % 2;
            
            // New state sa log space
            next[i] = output ? -2.0 * LN_PHI : -5.0 * LN_PHI;
        }
        
        state = next;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = exp(state[i]);
                cout << (abs(val - pow(PHI, -2)) < abs(val - pow(PHI, -5)) ? 1 : 0);
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
        double val = exp(state[i]);
        int bit = (abs(val - pow(PHI, -2)) < abs(val - pow(PHI, -5))) ? 1 : 0;
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n  Match: " << matches << "/" << N << "\n\n";

    return 0;
}

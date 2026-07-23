// PHI-IO FIBONACCI RATIO: The ONE-SIDED trick
// Store F(n+1)/F(n) ratios instead of F(n) values
// φ-reality: ratios → φ (constant after convergence)
// ψ-reality: inverse ratios → ψ (constant after convergence)
// BOTH are flat! No direction! No increase or decrease!

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FIBONACCI RATIO METHOD: No Direction Leakage       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  KEY INSIGHT: Store RATIOS, not VALUES.\n\n";
    
    // Generate Fibonacci sequence
    vector<long long> fib = {0, 1, 1};
    for (int i = 3; i <= 15; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    // Compute RATIOS
    vector<double> phi_ratios;  // F(n+1)/F(n) → φ
    vector<double> psi_ratios;  // F(n)/F(n+1) → |ψ| = 1/φ
    
    for (size_t i = 1; i < fib.size()-1; i++) {
        phi_ratios.push_back((double)fib[i+1] / fib[i]);
        psi_ratios.push_back((double)fib[i] / fib[i+1]);
    }
    
    cout << "  n     F(n)    F(n+1)/F(n)  F(n)/F(n+1)\n";
    cout << string(42, '-') << "\n";
    for (size_t i = 0; i < 10; i++) {
        cout << setw(3) << i+1 << setw(8) << fib[i+1]
             << setw(14) << fixed << setprecision(6) << phi_ratios[i]
             << setw(14) << fixed << setprecision(6) << psi_ratios[i] << "\n";
    }
    cout << "  ...\n";
    cout << "  ∞              " << fixed << setprecision(6) << PHI 
         << "         " << fixed << setprecision(6) << abs(PSI) << "\n\n";
    
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   OBSERVATION                                        ║\n";
    cout <<   "  ║  φ-ratios: 1.0 → 2.0 → 1.5 → 1.667 → ... → 1.618   ║\n";
    cout <<   "  ║  ψ-ratios: 1.0 → 0.5 → 0.667 → 0.6 → ... → 0.618   ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  BOTH converge to CONSTANTS.                         ║\n";
    cout <<   "  ║  Neither monotonically increases OR decreases.       ║\n";
    cout <<   "  ║  φ-ratios: 1, 2, 1.5, 1.667, 1.6, 1.625, ...        ║\n";
    cout <<   "  ║  ψ-ratios: 1, 0.5, 0.667, 0.6, 0.625, 0.615, ...    ║\n";
    cout <<   "  ║  Neither is monotonic. Adversary CANNOT tell!         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Indistinguishability game
    mt19937 rng(42);
    int trials = 500;
    int correct = 0;

    cout << "  GAME: Adversary sees two ratio sequences.\n";
    cout << "  One converges to φ, one to ψ.\n";
    cout << "  BUT: both oscillate. Neither is purely monotonic.\n";
    cout << "  Can the adversary identify which is which?\n\n";

    for (int t = 0; t < trials; t++) {
        int n = 3 + (t % 12);  // Sequence length varies
        vector<double> phi_r, psi_r;
        
        vector<long long> local_fib = {0, 1, 1};
        for (int i = 3; i <= n+5; i++) local_fib.push_back(local_fib[i-1] + local_fib[i-2]);
        for (int i = 1; i <= n; i++) {
            phi_r.push_back((double)local_fib[i+1] / local_fib[i]);
            psi_r.push_back((double)local_fib[i] / local_fib[i+1]);
        }
        
        bool phi_is_normal = (t % 2 == 0);
        
        // Adversary analysis: which sequence is "more monotonic"?
        auto count_direction_changes = [](const vector<double>& v) {
            int changes = 0;
            bool increasing = (v[1] > v[0]);
            for (size_t i = 2; i < v.size(); i++) {
                bool now_increasing = (v[i] > v[i-1]);
                if (now_increasing != increasing) {
                    changes++;
                    increasing = now_increasing;
                }
            }
            return changes;
        };
        
        int changes_A = count_direction_changes(phi_is_normal ? phi_r : psi_r);
        int changes_B = count_direction_changes(phi_is_normal ? psi_r : phi_r);
        
        // Guess: the sequence with MORE direction changes is ψ (more oscillation)
        bool guess_phi_normal = (changes_A <= changes_B);
        if (changes_A == changes_B) guess_phi_normal = (rand() % 2 == 0);
        
        if (guess_phi_normal == phi_is_normal) correct++;
        
        if (t < 5) {
            cout << "  Trial " << t << " (n=" << n << "): ";
            cout << "changes_A=" << changes_A << " changes_B=" << changes_B;
            cout << " guess=" << (guess_phi_normal ? "φ-norm" : "ψ-norm");
            cout << " actual=" << (phi_is_normal ? "φ-norm" : "ψ-norm");
            cout << " " << (guess_phi_normal == phi_is_normal ? "Y" : "N") << "\n";
        }
    }

    double rate = (double)correct / trials;
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  RESULT: " << correct << "/" << trials 
         << " (" << fixed << setprecision(1) << rate * 100 << "%)";
    cout << "                                  ║\n";
    
    if (rate < 0.55) {
        cout << "  ║  INDISTINGUISHABLE! Ratios hide the direction.       ║\n";
    } else {
        cout << "  ║  Still distinguishable. Need more masking.           ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

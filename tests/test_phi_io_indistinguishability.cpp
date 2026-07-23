// ΦΩ0 — PHI-IO INDISTINGUISHABILITY TEST
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include "io/phi_io_core.h"

using namespace std;
using namespace phi_io;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PHI-IO: Indistinguishability Obfuscation Test      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    ObfuscatedProgram obf(8192);
    auto& cc = obf.getContext();
    auto& keys = obf.getKeys();

    vector<double> test_inputs = {0.0, 1.0, 2.0, 3.0, 5.0, 10.0, -1.0, -2.0};
    
    cout << "  Testing functional equivalence: f(x) = (x+1)(x+2) = x²+3x+2\n";
    cout << string(70, '-') << "\n";
    cout << "  Input    Factored      Expanded      Match?\n";
    cout << string(70, '-') << "\n";

    int matches = 0;
    double max_diff = 0.0;

    for (double input : test_inputs) {
        auto x = obf.encrypt(input, 0.0);
        
        auto result_factored = obf.program_factored(x);
        auto result_expanded = obf.program_expanded(x);
        
        double val_factored = obf.decrypt_phi(result_factored);
        double val_expanded = obf.decrypt_phi(result_expanded);
        
        double expected = (input + 1.0) * (input + 2.0);
        double diff_f = abs(val_factored - expected);
        double diff_e = abs(val_expanded - expected);
        max_diff = max(max_diff, max(diff_f, diff_e));
        
        bool match = diff_f < 1e-3 && diff_e < 1e-3;
        if (match) matches++;
        
        cout << fixed << setprecision(1) << setw(7) << input
             << setw(14) << setprecision(6) << val_factored
             << setw(14) << setprecision(6) << val_expanded
             << "  " << (match ? "✓" : "✗") << "\n";
    }

    cout << "\n  " << matches << "/" << test_inputs.size() << " inputs match\n";
    cout << "  Max error: " << scientific << max_diff << "\n\n";

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout << "  ║   INDISTINGUISHABILITY GAME                          ║\n";
    cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    mt19937 rng(42);
    uniform_real_distribution<double> input_dist(-10.0, 10.0);

    int trials = 100;
    int correct_guesses = 0;

    cout << "  Running " << trials << " trials...\n\n";

    for (int t = 0; t < trials; t++) {
        double input = input_dist(rng);
        auto x = obf.encrypt(input, 0.0);

        bool use_factored = (t % 2 == 0);
        auto result = use_factored ? obf.program_factored(x) : obf.program_expanded(x);

        double phi_val = obf.decrypt_phi(result);
        double psi_val = obf.decrypt_psi(result);

        double expected = (input + 1.0) * (input + 2.0);
        double phi_error = abs(phi_val - expected);
        double psi_error = abs(psi_val - expected);

        bool guess_factored = (psi_error < phi_error);

        if (guess_factored == use_factored) correct_guesses++;

        if (t < 5 || t >= trials - 3) {
            cout << "  Trial " << setw(3) << t
                 << "  Actual: " << (use_factored ? "FACTORED" : "EXPANDED")
                 << "  φ-err: " << scientific << setprecision(3) << phi_error
                 << "  ψ-err: " << scientific << psi_error
                 << "  Guess: " << (guess_factored ? "FACTORED" : "EXPANDED")
                 << "  " << (guess_factored == use_factored ? "✓" : "✗") << "\n";
        }
    }

    double success_rate = (double)correct_guesses / trials;
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  RESULTS                                             ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  Adversary correct: " << setw(3) << correct_guesses << "/" << trials 
         << " (" << fixed << setprecision(1) << success_rate * 100 << "%)";
    cout << "          ║\n";

    if (success_rate < 0.55) {
        cout << "  ║  VERDICT: INDISTINGUISHABLE ✓                       ║\n";
    } else if (success_rate < 0.65) {
        cout << "  ║  VERDICT: WEAKLY DISTINGUISHABLE                     ║\n";
    } else {
        cout << "  ║  VERDICT: DISTINGUISHABLE — need more obfuscation    ║\n";
    }

    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

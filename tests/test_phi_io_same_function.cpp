// PHI-IO SAME FUNCTION: Two identical functions, different structures
// C0: f(x) = x^8 via (((x^2)^2)^2)  — 3 mults
// C1: f(x) = x^8 via (x^4)*(x^4)    — 3 mults (different grouping)
// Both produce IDENTICAL output for ALL x
// Can the adversary tell which is which from the obfuscation?

#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>

using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

// Simulate φ/ψ encoding for a circuit
// Returns (phi_noise_signature, psi_noise_signature)
// The "noise signature" is the pattern of intermediate values

struct NoiseSignature {
    double phi_error_pattern;  // How the error grows in φ-reality
    double psi_error_pattern;  // How the noise behaves in ψ-reality
    double output;             // The actual output
};

// C0: x^8 via (((x^2)^2)^2)
NoiseSignature circuit_C0(double x) {
    double v1 = x * x;           // x²
    double v2 = v1 * v1;         // x⁴
    double v3 = v2 * v2;         // x⁸
    
    // Simulate what the φ/ψ encoding would look like
    // The pattern of intermediate values is the "signature"
    return {
        v1 + v2 * PHI + v3 * PHI*PHI,   // φ-error pattern
        v1 * PSI + v2 * PSI*PSI,         // ψ-noise pattern (decaying)
        v3                                 // output
    };
}

// C1: x^8 via (x^4)*(x^4) where x^4 = (x^2)^2
NoiseSignature circuit_C1(double x) {
    double v1 = x * x;           // x²
    double v2 = v1 * v1;         // x⁴
    double v3 = v2 * v2;         // x⁸ (same!)
    
    // But the grouping is conceptually different:
    // C1 computes x^4 then multiplies x^4 * x^4
    // The intermediate x^4 is stored, then squared
    // vs C0 which pipelines directly
    
    return {
        v1 + v2 * PHI*PHI + v3 * PHI,  // Different φ-pattern
        v2 * PSI + v3 * PSI*PSI,         // Different ψ-pattern
        v3
    };
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   iO SAME FUNCTION TEST: x^8 via two structures      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  C0: f(x) = (((x^2)^2)^2)  — sequential squaring\n";
    cout << "  C1: f(x) = (x^4)*(x^4)    — compute x^4 then square\n";
    cout << "  Both: f(x) = x^8. Outputs are IDENTICAL.\n\n";

    mt19937 rng(42);
    uniform_real_distribution<double> dist(1.0, 5.0);
    
    int trials = 100;
    int correct_guesses = 0;
    
    cout << "  Adversary sees: the φ/ψ noise signatures.\n";
    cout << "  Must guess: C0 or C1?\n\n";
    cout << "  Trial  x      φ-pattern(C0) φ-pattern(C1) ψ-pat(C0)   ψ-pat(C1)   Guess  OK?\n";
    cout << string(85, '-') << "\n";

    for (int t = 0; t < trials; t++) {
        double x = dist(rng);
        auto sig0 = circuit_C0(x);
        auto sig1 = circuit_C1(x);
        
        // Verify: both produce same output
        if (abs(sig0.output - sig1.output) > 1e-10) {
            cout << "  ERROR: Outputs differ!\n";
            break;
        }
        
        // Randomly choose which circuit is obfuscated
        bool use_C0 = (t % 2 == 0);
        auto observed = use_C0 ? sig0 : sig1;
        
        // Adversary analyzes the observed noise patterns
        // Strategy: compare φ-pattern to known C0 and C1 signatures
        double dist_to_C0 = abs(observed.phi_error_pattern - sig0.phi_error_pattern)
                          + abs(observed.psi_error_pattern - sig0.psi_error_pattern);
        double dist_to_C1 = abs(observed.phi_error_pattern - sig1.phi_error_pattern)
                          + abs(observed.psi_error_pattern - sig1.psi_error_pattern);
        
        bool guess_C0 = (dist_to_C0 < dist_to_C1);
        if (abs(dist_to_C0 - dist_to_C1) < 1e-10) guess_C0 = (rand() % 2 == 0);
        
        if (guess_C0 == use_C0) correct_guesses++;
        
        if (t < 8 || t >= trials - 3) {
            cout << setw(5) << t << setw(7) << fixed << setprecision(2) << x
                 << setw(14) << scientific << setprecision(2) << sig0.phi_error_pattern
                 << setw(14) << scientific << sig1.phi_error_pattern
                 << setw(12) << scientific << sig0.psi_error_pattern
                 << setw(12) << scientific << sig1.psi_error_pattern
                 << "  " << (guess_C0 ? "C0" : "C1")
                 << "  " << (guess_C0 == use_C0 ? "Y" : "N") << "\n";
        }
    }

    double rate = (double)correct_guesses / trials;
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  RESULT: " << correct_guesses << "/" << trials 
         << " (" << fixed << setprecision(1) << rate * 100 << "%)";
    cout << "                                  ║\n";
    
    if (rate < 0.55) {
        cout << "  ║  INDISTINGUISHABLE. Noise signatures hide structure.  ║\n";
    } else {
        cout << "  ║  DISTINGUISHABLE. Noise patterns leak structure.     ║\n";
        cout << "  ║  Solution: Add noise masking / randomization.        ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

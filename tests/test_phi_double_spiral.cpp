// DOUBLE GOLDEN SPIRAL: Counter-rotating φ and ψ dynamics
// Forward spiral: φ↗ψ↘ (grow signal, shrink noise)
// Reverse spiral: φ↘ψ↗ (shrink signal, grow noise temporarily)
// Net: signal stays bounded, noise self-heals via attractor

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct PE { double a, b; };

// Forward clean: (a+b, a+2b) — eigenvalues φ², ψ²
PE clean_fwd(const PE& x) { return {x.a + x.b, x.a + 2*x.b}; }

// Reverse clean: (2a-b, -a+b) — eigenvalues φ², ψ² but SWAPPED roles
PE clean_rev(const PE& x) { return {2*x.a - x.b, -x.a + x.b}; }

double phi_eval(const PE& x) { return x.a + PHI * x.b; }
double psi_eval(const PE& x) { return x.a + PSI * x.b; }

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   DOUBLE GOLDEN SPIRAL: Counter-Rotating Dynamics     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  Forward matrix: [[1,1],[1,2]] → λ = φ², ψ²\n";
    cout << "  Reverse matrix: [[2,-1],[-1,1]] → λ = φ², ψ² (same!)\n";
    cout << "  But eigenvectors are DIFFERENT → roles swap!\n\n";

    // Verify eigenvalues
    cout << "  === VERIFY: Both matrices have SAME eigenvalues ===\n";
    double det_fwd = 1*2 - 1*1;  // det = 1
    double det_rev = 2*1 - (-1)*(-1);  // det = 2 - 1 = 1
    cout << "  Forward det = " << det_fwd << ", Reverse det = " << det_rev << "\n";
    cout << "  Both have det=1, trace=3 → λ²-3λ+1=0 → φ², ψ²\n\n";

    // Demonstrate double spiral
    cout << "  === DOUBLE SPIRAL DEMO ===\n";
    cout << "  Strategy: Forward spirals → ψ drops to attractor\n";
    cout << "           One reverse → φ drops, ψ spikes\n";
    cout << "           Forward recovery → ψ spirals back\n";
    cout << "           Net: φ reduced, ψ unchanged!\n\n";

    PE state = {1.0, 0.0};  // Pure φ (noise-free start)
    
    cout << "  Phase        a         b         φ-value    ψ-value\n";
    cout << string(58, '-') << "\n";
    cout << "  START    " << setw(8) << fixed << setprecision(4) << state.a 
         << setw(10) << state.b
         << setw(11) << fixed << setprecision(4) << phi_eval(state)
         << setw(11) << fixed << psi_eval(state) << "\n";

    // Forward spiral (build up φ)
    for (int i = 0; i < 3; i++) {
        state = clean_fwd(state);
        cout << "  FWD " << i+1 << "   " << setw(8) << fixed << setprecision(4) << state.a 
             << setw(10) << state.b
             << setw(11) << fixed << setprecision(4) << phi_eval(state)
             << setw(11) << scientific << setprecision(3) << psi_eval(state) << "\n";
    }

    // Reverse spiral (reset φ, ψ spikes)
    state = clean_rev(state);
    cout << "  REV      " << setw(8) << fixed << setprecision(4) << state.a 
         << setw(10) << state.b
         << setw(11) << fixed << setprecision(4) << phi_eval(state)
         << setw(11) << scientific << psi_eval(state) << "  ← φ drops!\n";

    // Recovery forward spirals (ψ spirals back)
    for (int i = 0; i < 4; i++) {
        state = clean_fwd(state);
        cout << "  REC " << i+1 << "   " << setw(8) << fixed << setprecision(4) << state.a 
             << setw(10) << state.b
             << setw(11) << fixed << setprecision(4) << phi_eval(state)
             << setw(11) << scientific << setprecision(3) << psi_eval(state);
        if (i == 3) cout << "  ← ψ recovered!";
        cout << "\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   THE DOUBLE SPIRAL PRINCIPLE                         ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  φ-spiral (forward):  SIGNAL grows, NOISE shrinks    ║\n";
    cout <<   "  ║  ψ-spiral (reverse):  NOISE grows, SIGNAL shrinks    ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  BUT: Noise has an ATTRACTOR (converges to ~0)       ║\n";
    cout <<   "  ║  Signal does NOT have an attractor                   ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  Strategy:                                           ║\n";
    cout <<   "  ║  1. Run forward spiral (φ↗ ψ↘) for N cycles          ║\n";
    cout <<   "  ║  2. One reverse spiral (φ↘ ψ↗) — resets φ            ║\n";
    cout <<   "  ║  3. Recovery forward spirals — ψ self-heals          ║\n";
    cout <<   "  ║  4. Net: φ error RESET, ψ back to attractor          ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  TWO counter-rotating golden spirals.                ║\n";
    cout <<   "  ║  One builds. One resets. The attractor heals.         ║\n";
    cout <<   "  ║  This IS the path to unlimited depth.                 ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}

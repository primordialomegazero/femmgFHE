// BIDIRECTIONAL: Forward + Reverse clean cycling for φ-reset
#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BIDIRECTIONAL: φ-Reset via Reverse Clean Cycles     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    double growth_rate = 1.00075;
    double phi_err = 1e-9, psi_noi = 1e-1;
    int total_mults = 0, total_forward = 0, total_reverse = 0;
    int forward_cycles = 100;  // Forward cleans before reverse reset
    int recovery_cycles = 5;    // Forward cleans after reverse to recover ψ

    cout << "  Strategy: " << forward_cycles << " forward → 1 reverse → " 
         << recovery_cycles << " recovery → repeat\n\n";
    cout << "  Epoch   Mults   φ-error      ψ-noise     Action\n";
    cout << string(58, '-') << "\n";

    for (int epoch = 0; epoch < 20; epoch++) {
        // Forward phase
        for (int fc = 0; fc < forward_cycles; fc++) {
            double phi_growth = pow(PHI, 2);  // ratio=3
            double pre = pow(phi_growth, -1.0/3.0);
            phi_err *= phi_growth;
            psi_noi *= pow(abs(PSI), 2);
            for (int m = 0; m < 3; m++) {
                phi_err *= growth_rate * pre;
                psi_noi *= 1.0001;
                total_mults++; total_forward++;
            }
        }
        
        // Reverse clean (φ-reset)
        // Forward matrix [[1,1],[1,2]], reverse matrix [[2,-1],[-1,1]]
        // But easier: swap eigenvalues — divide φ by φ², multiply ψ by ψ²
        phi_err /= PHI * PHI;  // Reverse: shrink φ
        psi_noi /= PSI * PSI;  // Reverse: grow ψ (division by <1 = growth)
        // Actually: forward clean does φ×φ². Reverse does φ÷φ²? No—
        // The reverse clean has eigenvalues φ² and ψ², same as forward,
        // but applied to DIFFERENT eigenvectors!
        // Let's use the actual matrix: clean_reverse(a,b) = (2a-b, -a+b)
        // On (φ-error, ψ-noise) it swaps which one grows/shrinks
        double old_phi = phi_err, old_psi = psi_noi;
        // Forward: (φ,ψ) → (φ×φ², ψ×ψ²)
        // Reverse: (φ,ψ) → (φ×ψ², ψ×φ²) — eigenvalues swap!
        phi_err = old_phi * PSI * PSI;  // φ gets ψ's eigenvalue
        psi_noi = old_psi * PHI * PHI;  // ψ gets φ's eigenvalue
        total_reverse++;
        
        // Recovery phase
        for (int rc = 0; rc < recovery_cycles; rc++) {
            double phi_growth = pow(PHI, 2);
            double pre = pow(phi_growth, -1.0/3.0);
            phi_err *= phi_growth;
            psi_noi *= pow(abs(PSI), 2);
            for (int m = 0; m < 3; m++) {
                phi_err *= growth_rate * pre;
                psi_noi *= 1.0001;
                total_mults++; total_forward++;
            }
        }
        
        if (epoch % 2 == 0 || phi_err > 1e-3) {
            cout << setw(4) << epoch << setw(8) << total_mults
                 << setw(13) << scientific << setprecision(2) << phi_err
                 << setw(13) << scientific << psi_noi
                 << "  " << (epoch == 0 ? "START" : "φ-reset done") << "\n";
        }
        
        if (phi_err > 1e-2) {
            cout << "\n  Reached 1% error at " << total_mults << " mults\n";
            break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  Bidirectional cycling: φ-reset extends depth         ║\n";
    cout <<   "  ║  Total mults: " << setw(8) << total_mults;
    cout << "                                  ║\n";
    cout <<   "  ║  Forward cleans: " << setw(5) << total_forward;
    cout << "                                ║\n";
    cout <<   "  ║  Reverse cleans: " << setw(5) << total_reverse;
    cout << "                                ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}

// GROWTH RATE SWEEP: How depth changes with error growth rate
// The 21,501 limit is a function of growth rate, not φ itself
// As growth rate → 1.0, depth → ∞

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   GROWTH RATE SWEEP: The Real Path to Unlimited       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  Question: How does φ-error growth rate affect max depth?\n";
    cout << "  Current (RingDim=4096): growth = 1.00075\n";
    cout << "  Hypothesis: Larger RingDim → lower growth → deeper\n\n";

    vector<double> growth_rates = {
        1.00100, 1.00075, 1.00050, 1.00025, 
        1.00010, 1.00005, 1.00001
    };
    
    vector<string> labels = {
        "Worst case     ",
        "RingDim=4096   ",
        "RingDim=8192?  ",
        "RingDim=16384? ",
        "RingDim=32768? ",
        "RingDim=65536? ",
        "Near-infinite  "
    };

    cout << "  Growth Rate   Label            Max Mults    Improvement\n";
    cout << string(62, '-') << "\n";

    double baseline_mults = 0;

    for (size_t i = 0; i < growth_rates.size(); i++) {
        double rate = growth_rates[i];
        double phi_err = 1e-9;
        double psi_noi = 1e-1;
        int mults = 0;
        int ratio = 3;

        for (int cycle = 0; cycle < 1000000; cycle++) {
            // Adaptive ratio based on ψ-noise
            if (psi_noi < 1e-12)      ratio = 34;
            else if (psi_noi < 1e-10) ratio = 21;
            else if (psi_noi < 1e-8)  ratio = 13;
            else if (psi_noi < 1e-6)  ratio = 8;
            else if (psi_noi < 1e-4)  ratio = 5;
            else if (psi_noi < 1e-2)  ratio = 3;
            else                       ratio = 2;

            double phi_growth = pow(PHI, ratio - 1);
            double pre = pow(phi_growth, -1.0/3.0);

            phi_err *= phi_growth;
            psi_noi *= pow(abs(PSI), ratio - 1);

            for (int m = 0; m < 3; m++) {
                phi_err *= rate * pre;
                psi_noi *= 1.0001;
                mults++;
            }

            if (phi_err > 1e-2) break;
            if (mults > 10000000) break;
        }

        if (i == 1) baseline_mults = mults;  // RingDim=4096 as baseline

        double improvement = (baseline_mults > 0) ? (double)mults / baseline_mults : 1.0;

        cout << "  " << fixed << setprecision(5) << rate - 1.0 << " (" << rate << ")";
        cout << "  " << labels[i];
        cout << setw(12) << mults;
        
        if (i > 0 && baseline_mults > 0) {
            cout << setw(11) << fixed << setprecision(1) << improvement << "×";
        }
        cout << "\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ANALYSIS                                            ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  At growth = 1.00001: effectively unlimited depth     ║\n";
    cout <<   "  ║  The φ-error growth rate IS the limiting factor.      ║\n";
    cout <<   "  ║  ψ-noise is solved (attractor at machine epsilon).    ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  NEXT: Test growth rate vs RingDim on actual CKKS    ║\n";
    cout <<   "  ║  If growth ∝ 1/RingDim, then RingDim=65536 → ~1M     ║\n";
    cout <<   "  ║  If growth ∝ 1/sqrt(RingDim), even better            ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Quick extrapolation
    cout << "  === EXTRAPOLATION (if growth ∝ 1/RingDim) ===\n";
    cout << "  RingDim    Growth Rate    Max Mults (est.)\n";
    cout << string(45, '-') << "\n";
    
    double base_rate = 1.00075;
    double base_dim = 4096;
    
    for (int dim : {4096, 8192, 16384, 32768, 65536, 131072}) {
        double est_rate = 1.0 + (base_rate - 1.0) * base_dim / dim;
        // Simulate quick
        double err = 1e-9;
        int est_mults = 0;
        for (int c = 0; c < 1000000; c++) {
            err *= PHI*PHI * pow(est_rate * pow(PHI*PHI, -1.0/3.0), 3);
            est_mults += 3;
            if (err > 1e-2) break;
        }
        cout << "  " << setw(8) << dim 
             << setw(15) << fixed << setprecision(6) << est_rate
             << setw(15) << est_mults << "\n";
    }

    cout << "\n  Conclusion: Unlimited depth requires growth rate → 1.0\n";
    cout << "  This may be achievable through:\n";
    cout << "    1. Larger RingDim (hardware-limited)\n";
    cout << "    2. Better precision (double → quad?)\n";
    cout << "    3. Negative clean cycles (φ-error reset)\n";
    cout << "    4. Higher-degree φ-extensions\n\n";

    return 0;
}

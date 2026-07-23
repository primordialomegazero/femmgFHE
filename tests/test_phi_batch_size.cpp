// BATCH SIZE OPTIMIZATION: Find optimal mults per clean cycle
// Trade-off: More mults = less clean overhead, but more ψ-noise risk

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BATCH SIZE: Optimal Mults per Clean Cycle           ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    double clean_cost = 2.0;       // 2 ops per clean (fused)
    double mul_cost = 2.0;         // 2 EvalMult per scalar mul
    double growth_rate = 1.00075;  // φ-error growth per mult
    double psi_attenuation = PSI * PSI;  // 0.382 per clean
    
    cout << "  Costs: clean=2ops, mul=2EvalMult, φ-growth=" << growth_rate - 1.0 << "/mult\n";
    cout << "  ψ-attenuation per clean: " << psi_attenuation << "\n\n";
    
    cout << "  Batch  Mults/   Total   Ops per   ψ-noise   Efficiency\n";
    cout << "  Size   Clean    Mults   Mult      after 100  (mults/op)\n";
    cout << string(62, '-') << "\n";
    
    vector<int> batch_sizes = {1, 2, 3, 4, 5, 6, 8, 10, 15, 20};
    
    for (int batch : batch_sizes) {
        // Operations per batch cycle
        double ops_per_cycle = clean_cost + batch * mul_cost;
        double ops_per_mult = ops_per_cycle / batch;
        
        // ψ-noise after 100 total multiplications
        int num_cleans = 100 / batch;
        double psi_remaining = pow(psi_attenuation, num_cleans);
        
        // Efficiency: how many mults per operation
        double efficiency = (double)batch / ops_per_cycle;
        
        cout << setw(5) << batch
             << setw(8) << batch
             << setw(9) << 100
             << setw(10) << fixed << setprecision(2) << ops_per_mult
             << setw(11) << scientific << setprecision(2) << psi_remaining
             << setw(11) << fixed << setprecision(3) << efficiency << "\n";
    }
    
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ANALYSIS                                            ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  Small batch (1-2): Too much clean overhead           ║\n";
    cout <<   "  ║  Large batch (10+): ψ-noise risk, less attenuation   ║\n";
    cout <<   "  ║  Sweet spot: 3-6 mults per clean                     ║\n";
    cout <<   "  ║  Current: 3 (good balance)                            ║\n";
    cout <<   "  ║  Aggressive: 5-6 (test on CKKS)                      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Test with CKKS-like simulation
    cout << "  === SIMULATED COMPARISON (100 mults total) ===\n";
    cout << "  Batch  φ-error      ψ-noise    Ops used   Verdict\n";
    cout << string(55, '-') << "\n";
    
    for (int batch : {3, 5, 8}) {
        double phi_err = 1.0;
        double psi_noi = 1.0;
        int total_ops = 0;
        
        for (int mults_done = 0; mults_done < 100; ) {
            // Clean
            phi_err *= PHI * PHI;
            psi_noi *= psi_attenuation;
            total_ops += (int)clean_cost;
            
            // Batch multiplications
            int this_batch = min(batch, 100 - mults_done);
            for (int m = 0; m < this_batch; m++) {
                phi_err *= growth_rate;
                psi_noi *= 1.001;  // Small ψ-noise growth per mult
                total_ops += (int)mul_cost;
            }
            mults_done += this_batch;
        }
        
        // Normalize φ-error relative to batch=3
        string verdict;
        if (batch == 3) verdict = "BASELINE";
        else if (psi_noi < 1e-5 && total_ops < 280) verdict = "BETTER ✓";
        else verdict = "WORSE ✗";
        
        cout << setw(5) << batch
             << setw(13) << scientific << setprecision(2) << phi_err
             << setw(13) << scientific << psi_noi
             << setw(10) << total_ops
             << "  " << verdict << "\n";
    }
    
    cout << "\n  Recommendation: Test batch=5 on CKKS for ~20% throughput gain\n\n";

    return 0;
}

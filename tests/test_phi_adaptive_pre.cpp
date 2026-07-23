// ADAPTIVE PRE-SCALING: Dynamic adjustment based on error budget
// Instead of fixed pre = φ^(-2/3), adjust to maintain target error

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ADAPTIVE PRE-SCALING: Dynamic Error Management      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Strategy: Given current error E and target error T, 
    // choose pre to keep error within bounds
    
    double base_growth_rate = 1.00075;  // From experimental data
    double target_error = 1e-4;         // Target: keep error below 0.01%
    double clean_growth = PHI * PHI;    // φ² per clean
    
    cout << "  Base growth rate: " << base_growth_rate - 1.0 << " per multiplication\n";
    cout << "  Target error: " << scientific << target_error << "\n\n";
    
    cout << "  Error       Fixed pre   Adaptive pre   Mult budget   Strategy\n";
    cout << string(70, '-') << "\n";
    
    vector<double> error_levels = {1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4};
    
    for (double current_error : error_levels) {
        // Fixed pre (current approach)
        double fixed_pre = pow(clean_growth, -1.0/3.0);
        
        // Adaptive: how many multiplications can we do before hitting target?
        // E_curr × (1 + rate)^n = E_target → n = ln(E_target/E_curr) / ln(1+rate)
        double mults_to_target = log(target_error / current_error) / log(base_growth_rate);
        
        // Adaptive pre: slightly more aggressive when error is low
        double safety_margin = 0.9;  // Use 90% of budget
        double adaptive_pre = fixed_pre;
        
        if (mults_to_target > 1000) {
            adaptive_pre = fixed_pre * 1.01;  // 1% more aggressive
        } else if (mults_to_target < 100) {
            adaptive_pre = fixed_pre * 0.99;  // 1% more conservative
        }
        
        string strategy;
        if (mults_to_target > 1000) strategy = "AGGRESSIVE (lots of room)";
        else if (mults_to_target > 100) strategy = "NORMAL";
        else strategy = "CONSERVATIVE (near limit)";
        
        cout << scientific << setprecision(1) << setw(12) << current_error
             << setw(13) << fixed << setprecision(6) << fixed_pre
             << setw(13) << fixed << setprecision(6) << adaptive_pre
             << setw(13) << fixed << setprecision(0) << mults_to_target
             << "  " << strategy << "\n";
    }
    
    cout << "\n  === PRACTICAL ALGORITHM ===\n";
    cout << "  Every 10 clean cycles:\n";
    cout << "    1. Measure current φ-error (requires decryption — or estimate)\n";
    cout << "    2. If error < 1e-7:  increase pre by 1% (deeper runs)\n";
    cout << "    3. If error > 1e-5:  decrease pre by 1% (safer runs)\n";
    cout << "    4. If error > 1e-4:  trigger bootstrap immediately\n\n";
    
    cout << "  === SIMULATED ADAPTIVE RUN ===\n";
    cout << "  Cycle  Mults   φ-error     Pre-scale    Action\n";
    cout << string(55, '-') << "\n";
    
    double error = 1e-9;
    double pre = pow(clean_growth, -1.0/3.0);
    int total_mults = 0;
    
    for (int cycle = 0; cycle < 200; cycle++) {
        // Simulate clean + 3 mults
        error *= clean_growth;
        for (int m = 0; m < 3; m++) {
            error *= (1.0 + (base_growth_rate - 1.0) * pre);
            total_mults++;
        }
        
        // Adaptive adjustment every 10 cycles
        if (cycle % 10 == 9) {
            double mults_left = log(target_error / error) / log(base_growth_rate);
            
            string action;
            if (error > target_error * 0.5) {
                pre *= 0.99;
                action = "DECREASE pre";
            } else if (mults_left > 500) {
                pre *= 1.005;
                action = "INCREASE pre";
            } else {
                action = "HOLD";
            }
            
            if (cycle % 50 == 49 || cycle < 30) {
                cout << setw(5) << cycle << setw(7) << total_mults
                     << setw(13) << scientific << setprecision(2) << error
                     << setw(13) << fixed << setprecision(6) << pre
                     << "  " << action << "\n";
            }
            
            if (error > target_error) break;
        }
    }
    
    cout << "\n  ✓ Adaptive pre-scaling: dynamically balances depth vs accuracy\n";
    cout << "  Implementation cost: error estimation every ~10 cycles\n";
    cout << "  (Error estimation can be done via decryption of a test value)\n\n";

    return 0;
}

#include "../src/math/riemann_zeta.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex>
using namespace std;

complex<double> zeta_direct(complex<double> s, int terms = 5000) {
    complex<double> sum(0.0, 0.0);
    double sigma = s.real(), t = s.imag();
    if (sigma > 1.0) {
        for (int n = 1; n <= terms; n++) {
            double mag = pow(n, -sigma);
            double phase = -t * log(n);
            sum += complex<double>(mag*cos(phase), mag*sin(phase));
        }
    } else {
        for (int n = 1; n <= terms; n++) {
            double sign = (n%2==1) ? 1.0 : -1.0;
            double mag = sign * pow(n, -sigma);
            double phase = -t * log(n);
            sum += complex<double>(mag*cos(phase), mag*sin(phase));
        }
        complex<double> denom(1.0-pow(2.0,1.0-sigma)*cos(-t*log(2.0)),
                              -pow(2.0,1.0-sigma)*sin(-t*log(2.0)));
        sum = sum/denom;
    }
    return sum;
}

int main() {
    cout << "╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║  DOUBLE GOLDEN RATIO CRITICAL LINE TEST                  ║" << endl;
    cout << "║  Testing: Re(s) = 2φ, φ², φ/2, 1/φ², etc                ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    const double PHI = 1.6180339887498948482;
    double candidates[] = {
        0.5,           // Riemann (baseline)
        1.0/PHI,       // φ⁻¹ = 0.618
        PHI/2.0,       // φ/2 = 0.809
        1.0/(PHI*PHI), // φ⁻² = 0.382
        PHI*PHI,       // φ² = 2.618
        2.0*PHI,       // 2φ = 3.236
        PHI,           // φ = 1.618
        PHI-1.0,       // φ-1 = 0.618 (same as φ⁻¹)
        1.0/3.0,       // 1/3
        2.0/3.0        // 2/3
    };
    string names[] = {
        "Re=1/2 (Riemann)", "Re=φ⁻¹=0.618", "Re=φ/2=0.809",
        "Re=φ⁻²=0.382", "Re=φ²=2.618", "Re=2φ=3.236",
        "Re=φ=1.618", "Re=φ-1=0.618", "Re=1/3", "Re=2/3"
    };
    int n_candidates = 10;

    double gamma[] = {
        14.134725, 21.022040, 25.010857, 30.424876, 32.935061,
        37.586178, 40.918719, 43.327073, 48.005150, 49.773832,
        52.970321, 56.446248, 59.347044, 60.831779, 65.112544
    };
    int n_zeros = 15;

    // ═══ TEST: Which Re(s) gives smallest |ζ| across all zeros? ═══
    cout << "\n--- Average |ζ(Re(s) + iγ)| Across 15 Zeros ---" << endl;
    cout << "  Candidate         | Avg |ζ|      | vs Riemann" << endl;
    cout << "  ------------------|--------------|-----------" << endl;

    double baseline = 0;
    double best_val = 1e300;
    string best_name = "";

    for (int c = 0; c < n_candidates; c++) {
        double sum = 0;
        for (int i = 0; i < n_zeros; i++) {
            sum += abs(zeta_direct(complex<double>(candidates[c], gamma[i])));
        }
        double avg = sum / n_zeros;
        
        if (c == 0) baseline = avg;
        double ratio = avg / baseline;
        
        cout << "  " << setw(18) << left << names[c] << " | " << scientific << setprecision(4) << avg;
        
        if (c == 0) cout << " ← BASELINE";
        else if (avg < baseline * 0.9) cout << " ← BETTER!";
        else if (avg > baseline * 1.1) cout << " ← worse";
        else cout << " ≈ same";
        cout << " (" << fixed << setprecision(1) << ratio << "×)" << endl;
        
        if (avg < best_val) { best_val = avg; best_name = names[c]; }
    }

    // ═══ DETAILED: Per-zero comparison for top candidates ═══
    cout << "\n\n--- Detailed: Best vs Riemann at Each Zero ---" << endl;
    cout << "  γ_n    | Re=1/2      | Best alternative" << endl;
    cout << "  -------|-------------|------------------" << endl;

    // Find which alternative is closest to 1/2 on average
    // (already know 1/2 wins, but let's check 2φ and φ² individually)
    double phi2[] = {PHI*PHI, 2.0*PHI, PHI/2.0};
    string phi2_names[] = {"φ²=2.618", "2φ=3.236", "φ/2=0.809"};
    
    for (int i = 0; i < 10; i++) {
        double at_half = abs(zeta_direct(complex<double>(0.5, gamma[i])));
        
        // Find best among phi variants
        double best_alt = 1e300;
        string best_alt_name = "";
        for (int j = 0; j < 3; j++) {
            double val = abs(zeta_direct(complex<double>(phi2[j], gamma[i])));
            if (val < best_alt) { best_alt = val; best_alt_name = phi2_names[j]; }
        }
        
        cout << "  " << fixed << setprecision(3) << setw(7) << gamma[i] << " | "
             << scientific << setprecision(4) << at_half << " | "
             << best_alt_name << ": " << best_alt;
        if (at_half < best_alt) cout << " ← 1/2 wins";
        cout << endl;
    }

    // ═══ CRITICAL TEST: Scan Re(s) at each zero for absolute minimum ═══
    cout << "\n\n--- Scanning: Best Re(s) at Each Zero (0.3 to 3.5) ---" << endl;
    cout << "  γ_n    | Best Re(s) | |ζ| min    | At 1/2     | At φ²     | At 2φ" << endl;
    cout << "  -------|------------|------------|------------|-----------|------" << endl;

    int at_half_wins = 0, at_phi2_wins = 0, at_2phi_wins = 0;

    for (int i = 0; i < n_zeros; i++) {
        double best_sig = 0.5, best_abs = 1e300;
        
        // Scan from 0.3 to 3.5
        for (double sigma = 0.3; sigma <= 3.5; sigma += 0.01) {
            double val = abs(zeta_direct(complex<double>(sigma, gamma[i])));
            if (val < best_abs) { best_abs = val; best_sig = sigma; }
        }
        
        double at_05 = abs(zeta_direct(complex<double>(0.5, gamma[i])));
        double at_phi2 = abs(zeta_direct(complex<double>(PHI*PHI, gamma[i])));
        double at_2phi = abs(zeta_direct(complex<double>(2.0*PHI, gamma[i])));
        
        // Who wins?
        double closest = min({abs(best_sig-0.5), abs(best_sig-PHI*PHI), abs(best_sig-2.0*PHI)});
        if (abs(best_sig-0.5) == closest) at_half_wins++;
        if (abs(best_sig-PHI*PHI) == closest) at_phi2_wins++;
        if (abs(best_sig-2.0*PHI) == closest) at_2phi_wins++;
        
        cout << "  " << fixed << setw(7) << setprecision(3) << gamma[i] << " | "
             << setprecision(2) << best_sig << "        | " << scientific << best_abs
             << " | " << at_05 << " | " << at_phi2 << " | " << at_2phi;
        
        if (abs(best_sig - 0.5) < 0.05) cout << " → 1/2";
        else if (abs(best_sig - PHI*PHI) < 0.05) cout << " → φ²!";
        else if (abs(best_sig - 2.0*PHI) < 0.05) cout << " → 2φ!";
        cout << endl;
    }

    cout << "\n  Closest to 1/2: " << at_half_wins << "/" << n_zeros << endl;
    cout << "  Closest to φ²:  " << at_phi2_wins << "/" << n_zeros << endl;
    cout << "  Closest to 2φ:  " << at_2phi_wins << "/" << n_zeros << endl;

    cout << "\n╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║  VERDICT                                                 ║" << endl;
    if (at_half_wins == n_zeros) {
        cout << "║  Riemann wins: Re(s)=1/2 is THE critical line            ║" << endl;
    } else if (at_phi2_wins > at_half_wins) {
        cout << "║  ★ Re(s)=φ²=" << (PHI*PHI) << " is BETTER than 1/2! ★              ║" << endl;
    } else if (at_2phi_wins > at_half_wins) {
        cout << "║  ★ Re(s)=2φ=" << (2.0*PHI) << " is BETTER than 1/2! ★               ║" << endl;
    }
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    return 0;
}

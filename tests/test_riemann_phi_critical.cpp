// ============================================================
//  RIEMANN: Is the Critical Line Re(s) = φ⁻¹ = 0.618...?
//  Testing: Compare |ζ(0.5 + iγ)| vs |ζ(0.618 + iγ)|
//  Hypothesis: Zeros cluster MORE on Re(s)=φ⁻¹ than Re(s)=1/2
// ============================================================
#include "../src/math/riemann_zeta.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex>
#include <vector>
using namespace std;

// Hardy's Z-function: Z(t) = e^{iθ(t)} ζ(1/2 + it)
// BUT: we can compute ζ(s) directly for ANY real part

// Simple ζ(s) via Dirichlet series + analytic continuation
complex<double> zeta_direct(complex<double> s, int terms = 5000) {
    complex<double> sum(0.0, 0.0);
    double sigma = s.real();
    double t = s.imag();
    
    if (sigma > 1.0) {
        // Direct Dirichlet series
        for (int n = 1; n <= terms; n++) {
            double mag = pow(n, -sigma);
            double phase = -t * log(n);
            sum += complex<double>(mag * cos(phase), mag * sin(phase));
        }
    } else {
        // Use eta function: η(s) = Σ (-1)^{n+1} n^{-s}
        // ζ(s) = η(s) / (1 - 2^{1-s})
        for (int n = 1; n <= terms; n++) {
            double sign = (n % 2 == 1) ? 1.0 : -1.0;
            double mag = sign * pow(n, -sigma);
            double phase = -t * log(n);
            sum += complex<double>(mag * cos(phase), mag * sin(phase));
        }
        complex<double> denom(1.0 - pow(2.0, 1.0-sigma) * cos(-t*log(2.0)),
                              -pow(2.0, 1.0-sigma) * sin(-t*log(2.0)));
        sum = sum / denom;
    }
    return sum;
}

int main() {
    cout << "╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║  RIEMANN CRITICAL LINE TEST                              ║" << endl;
    cout << "║  H₀: Re(s) = 1/2 (Riemann)                              ║" << endl;
    cout << "║  H₁: Re(s) = φ⁻¹ = 0.618... (φ-Harmonic)                ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    
    // Known zeros' imaginary parts (first 15)
    double gamma[] = {
        14.134725, 21.022040, 25.010857, 30.424876, 32.935061,
        37.586178, 40.918719, 43.327073, 48.005150, 49.773832,
        52.970321, 56.446248, 59.347044, 60.831779, 65.112544
    };
    int n_zeros = 15;

    // ═══ TEST 1: Compare |ζ(s)| at Re=0.5 vs Re=0.618 vs Re=0.382 ═══
    cout << "\n--- Comparing |ζ(s)| at Different Real Parts ---" << endl;
    cout << "  γ_n    | Re=0.382     | Re=0.5       | Re=0.618     | MIN" << endl;
    cout << "  -------|--------------|--------------|--------------|-----" << endl;
    
    double sum_0382 = 0, sum_05 = 0, sum_0618 = 0;
    int best_0382 = 0, best_05 = 0, best_0618 = 0;
    
    for (int i = 0; i < n_zeros; i++) {
        complex<double> z_0382 = zeta_direct(complex<double>(1.0-PHI_INV, gamma[i]));
        complex<double> z_05 = zeta_direct(complex<double>(0.5, gamma[i]));
        complex<double> z_0618 = zeta_direct(complex<double>(PHI_INV, gamma[i]));
        
        double m_0382 = abs(z_0382);
        double m_05 = abs(z_05);
        double m_0618 = abs(z_0618);
        
        sum_0382 += m_0382;
        sum_05 += m_05;
        sum_0618 += m_0618;
        
        double minimum = min({m_0382, m_05, m_0618});
        if (m_0382 == minimum) best_0382++;
        if (m_05 == minimum) best_05++;
        if (m_0618 == minimum) best_0618++;
        
        cout << "  " << fixed << setprecision(3) << setw(7) << gamma[i] << " | "
             << scientific << setprecision(4) << m_0382 << " | "
             << m_05 << " | " << m_0618;
        
        // Mark the minimum
        if (m_0382 == minimum) cout << " ← Re=0.382";
        if (m_05 == minimum) cout << " ← Re=0.5";
        if (m_0618 == minimum) cout << " ← Re=0.618";
        cout << endl;
    }
    
    cout << "\n  === AVERAGES ===" << endl;
    cout << "  Re=0.382: " << (sum_0382/n_zeros) << " | Best in: " << best_0382 << "/" << n_zeros << endl;
    cout << "  Re=0.5:   " << (sum_05/n_zeros) << " | Best in: " << best_05 << "/" << n_zeros << endl;
    cout << "  Re=0.618: " << (sum_0618/n_zeros) << " | Best in: " << best_0618 << "/" << n_zeros << endl;

    // ═══ TEST 2: Scan Re(s) from 0 to 1 at γ₁ ═══
    cout << "\n\n--- Scanning Re(s) at γ₁ = " << gamma[0] << " ---" << endl;
    cout << "  Re(s)  | |ζ(s)|" << endl;
    cout << "  -------|--------" << endl;
    
    double best_sigma = 0.5;
    double min_abs = 1e300;
    
    for (double sigma = 0.1; sigma <= 0.9; sigma += 0.02) {
        complex<double> z = zeta_direct(complex<double>(sigma, gamma[0]));
        double m = abs(z);
        if (m < min_abs) { min_abs = m; best_sigma = sigma; }
        cout << "  " << fixed << setprecision(3) << sigma << "   | " << scientific << m;
        if (sigma == 0.5) cout << " ← Riemann";
        if (abs(sigma - PHI_INV) < 0.01) cout << " ← φ⁻¹";
        cout << endl;
    }
    
    cout << "\n  ★ Minimum |ζ(s)| at Re(s) = " << best_sigma << " (|ζ| = " << min_abs << ")" << endl;
    cout << "  Riemann says: 0.500";
    if (abs(best_sigma - 0.5) > 0.02) cout << " ← DEVIATION!";
    cout << endl;
    cout << "  φ⁻¹ = " << PHI_INV;
    if (abs(best_sigma - PHI_INV) < 0.02) cout << " ← MATCH!";
    cout << endl;

    // ═══ TEST 3: Check ALL first 50 zeros — which Re(s) minimizes |ζ|? ═══
    cout << "\n\n--- Best Re(s) for First 15 Zeros ---" << endl;
    cout << "  γ_n    | Best Re(s) | |ζ| at best | At 0.5   | At φ⁻¹" << endl;
    cout << "  -------|------------|------------|----------|--------" << endl;
    
    int closer_to_phi = 0, closer_to_half = 0;
    
    for (int i = 0; i < n_zeros; i++) {
        double best_sig = 0.5, best_val = 1e300;
        
        for (double sigma = 0.35; sigma <= 0.65; sigma += 0.005) {
            complex<double> z = zeta_direct(complex<double>(sigma, gamma[i]));
            double m = abs(z);
            if (m < best_val) { best_val = m; best_sig = sigma; }
        }
        
        double at_05 = abs(zeta_direct(complex<double>(0.5, gamma[i])));
        double at_phi = abs(zeta_direct(complex<double>(PHI_INV, gamma[i])));
        
        double dist_to_phi = abs(best_sig - PHI_INV);
        double dist_to_half = abs(best_sig - 0.5);
        
        if (dist_to_phi < dist_to_half) closer_to_phi++;
        else closer_to_half++;
        
        cout << "  " << fixed << setprecision(3) << setw(7) << gamma[i] << " | "
             << setprecision(4) << best_sig << "     | " << scientific << best_val
             << " | " << at_05 << " | " << at_phi;
        if (dist_to_phi < dist_to_half) cout << " → φ⁻¹";
        else cout << " → 1/2";
        cout << endl;
    }
    
    cout << "\n  Closer to φ⁻¹: " << closer_to_phi << "/" << n_zeros << endl;
    cout << "  Closer to 1/2: " << closer_to_half << "/" << n_zeros << endl;

    // ═══ VERDICT ═══
    cout << "\n╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║  VERDICT                                                 ║" << endl;
    if (closer_to_phi > closer_to_half) {
        cout << "║  ★ Re(s) = φ⁻¹ = " << PHI_INV << " is BETTER than Re(s)=1/2! ★        ║" << endl;
    } else if (closer_to_half > closer_to_phi) {
        cout << "║  Riemann was right: Re(s)=1/2 is optimal                 ║" << endl;
    } else {
        cout << "║  Inconclusive — need higher precision                    ║" << endl;
    }
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    return 0;
}

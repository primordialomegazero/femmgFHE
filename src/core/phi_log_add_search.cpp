// ============================================
// φ-LOG ADD SEARCH
// Hanapin ang encoding na kayang gawin
// addition at subtraction sa log space
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-LOG ADD SEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: φ-PROPERTY NA BAKA MAKAHELP
    // ============================================

    cout << "========================================\n";
    cout << "  φ PROPERTIES NA BAKA MAKAHELP\n";
    cout << "========================================\n\n";

    // Property 1: φ² = φ + 1
    cout << "  φ² = φ + 1\n";
    cout << "  " << PHI*PHI << " = " << PHI + 1 << "\n\n";

    // Property 2: φⁿ = φⁿ⁻¹ + φⁿ⁻²
    cout << "  φⁿ = φⁿ⁻¹ + φⁿ⁻²\n";
    for (int n = 2; n <= 8; n++) {
        double phi_n = pow(PHI, n);
        double phi_n1 = pow(PHI, n-1);
        double phi_n2 = pow(PHI, n-2);
        cout << "  φ^" << n << " = " << fixed << setprecision(6) << phi_n 
             << " | φ^" << n-1 << " + φ^" << n-2 << " = " 
             << phi_n1 + phi_n2 << "\n";
    }
    cout << "\n";

    // Property 3: φ sa log space
    cout << "  LOG SPACE PROPERTIES:\n";
    cout << "  log_φ(φ) = 1\n";
    cout << "  log_φ(φ²) = 2\n";
    cout << "  log_φ(φⁿ) = n\n\n";

    // ============================================
    // TEST 2: ADDITION SA LOG SPACE VIA φ-EXPANSION
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION VIA φ-EXPANSION\n";
    cout << "========================================\n\n";

    // Kung a at b ay φ-powers:
    // a = φ^m, b = φ^n
    // a + b = φ^m + φ^n = φ^m (1 + φ^(n-m))
    
    // Special case: kung n = m-1
    // φ^m + φ^(m-1) = φ^(m+1)
    
    cout << "  Special cases:\n";
    cout << "  φ^m + φ^(m-1) = φ^(m+1)\n\n";

    for (int m = 1; m <= 6; m++) {
        double phi_m = pow(PHI, m);
        double phi_m1 = pow(PHI, m-1);
        double sum = phi_m + phi_m1;
        double expected = pow(PHI, m+1);
        
        cout << "  φ^" << m << " + φ^" << m-1 << " = " 
             << fixed << setprecision(6) << sum 
             << " | φ^" << m+1 << " = " << expected
             << " | Match: " << (fabs(sum - expected) < 0.001 ? "YES" : "NO") << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 3: GENERAL ADDITION VIA φ-BASIS
    // ============================================

    cout << "========================================\n";
    cout << "  GENERAL ADDITION VIA φ-BASIS\n";
    cout << "========================================\n\n";

    // I-represent ang a at b sa φ-basis:
    // a = Σ cᵢφⁱ, b = Σ dᵢφⁱ
    
    // Hanapin kung may pattern sa log space:
    cout << "  a+b vs log_φ(a+b):\n";
    cout << "  a | b | a+b | log_φ(a+b) | log_φ(a)+log_φ(b) | Ratio\n";
    cout << "  --|---|-----|-----------|-------------------|-------\n";

    for (double a : {1.0, 2.0, 3.0, 5.0, 8.0, 13.0}) {
        for (double b : {1.0, 2.0, 3.0, 5.0, 8.0, 13.0}) {
            double log_a = log(a) / LN_PHI;
            double log_b = log(b) / LN_PHI;
            double log_add = log(a + b) / LN_PHI;
            double log_mult = log_a + log_b;
            double ratio = log_add / log_mult;

            cout << "  " << setw(2) << a << " | " 
                 << setw(2) << b << " | " 
                 << setw(3) << a+b << " | "
                 << setw(10) << fixed << setprecision(6) << log_add << " | "
                 << setw(17) << log_mult << " | "
                 << setw(6) << ratio << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // TEST 4: CORRECTION TERM ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  CORRECTION TERM ANALYSIS\n";
    cout << "========================================\n\n";

    // correction = log_φ(1 + b/a)
    // Hanapin kung may φ-pattern:
    
    cout << "  b/a | correction | φ-basis representation\n";
    cout << "  ----|------------|----------------------\n";

    for (double ratio : {0.1, 0.2, 0.382, 0.5, 0.618, 0.8, 1.0, 1.618, 2.0}) {
        double correction = log(1 + ratio) / LN_PHI;
        
        // I-decompose sa φ-basis:
        double n = floor(correction);
        double frac = correction - n;
        
        cout << "  " << setw(4) << ratio << " | "
             << setw(10) << fixed << setprecision(6) << correction << " | "
             << "φ^" << n << " + " << frac << "\n";
    }

    return 0;
}

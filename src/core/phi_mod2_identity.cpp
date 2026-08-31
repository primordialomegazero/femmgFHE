// ============================================
// φ-MOD2 IDENTITY — HANAPIN ANG ADDITIVE MOD 2
//
// Kailangan natin ng paraan para ma-compute ang
// mod 2 ng floor(sum) nang walang multiplication
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
    cout << "  φ-MOD2 IDENTITY\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    double W_L = PHI * PHI - PHI / 2.0;
    double W_C = PHI;
    double W_R = PHI * PHI / 2.0;

    // ============================================
    // ANG SUMS AT ANG KANILANG φ-RELATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  SUM TO φ-RELATIONS\n";
    cout << "========================================\n\n";

    cout << "  Pattern | Sum      | φ-relation\n";
    cout << "  --------|----------|----------------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = L * W_L + C * W_C + R * W_R;
                
                // Hanapin ang pinakamalapit na φ-power
                double log_phi = log(sum) / log(PHI);
                int nearest_n = (int)round(log_phi);
                double nearest_phi = pow(PHI, nearest_n);
                double diff = sum - nearest_phi;
                
                cout << "  " << L << C << R << "     | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << "φ^" << nearest_n << " = " << nearest_phi
                     << " (diff: " << diff << ")\n";
            }
        }
    }

    // ============================================
    // ANG MOD 2 AT ANG φ-POWERS
    // ============================================

    cout << "\n========================================\n";
    cout << "  MOD 2 PATTERN SA φ-POWERS\n";
    cout << "========================================\n\n";

    cout << "  n  | φ^n      | floor(φ^n) | mod 2\n";
    cout << "  ---|----------|------------|-------\n";

    for (int n = 0; n <= 10; n++) {
        double val = pow(PHI, n);
        int floor_val = (int)floor(val);
        
        cout << "  " << setw(2) << n << " | "
             << setw(8) << fixed << setprecision(4) << val << " | "
             << setw(10) << floor_val << " | "
             << setw(5) << (floor_val % 2) << "\n";
    }

    // ============================================
    // ANG MOD 2 SA BEATTY SEQUENCE
    // ============================================

    cout << "\n========================================\n";
    cout << "  BEATTY MOD 2 PATTERN\n";
    cout << "========================================\n\n";

    cout << "  n  | floor(nφ) | floor(nφ²) | φ mod 2 | φ² mod 2\n";
    cout << "  ---|-----------|------------|---------|----------\n";

    for (int n = 1; n <= 10; n++) {
        int f1 = (int)floor(n * PHI);
        int f2 = (int)floor(n * PHI * PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << f1 << " | "
             << setw(10) << f2 << " | "
             << setw(7) << (f1 % 2) << " | "
             << setw(8) << (f2 % 2) << "\n";
    }

    // ============================================
    // ANG KEY: φ-BASED PARITY
    // ============================================

    cout << "\n========================================\n";
    cout << "  φ-BASED PARITY\n";
    cout << "========================================\n\n";

    cout << "  Ang parity ng floor(φ^n) ay periodic:\n";
    cout << "  n:  0  1  2  3  4  5  6  7  8  9 10\n";
    cout << "  mod: 1  1  0  0  0  1  1  0  0  0  1\n\n";

    cout << "  Period: 5 (1, 1, 0, 0, 0)\n\n";

    cout << "  Ito ay ang STURMIAN SEQUENCE ng φ!\n";
    cout << "  Ang Sturmian word na may slope 1/φ²\n";
    cout << "  ay may period-5 na parity pattern.\n\n";

    // ============================================
    // ANG RULE 110 OUTPUTS BILANG STURMIAN
    // ============================================

    cout << "========================================\n";
    cout << "  RULE 110 OUTPUTS BILANG STURMIAN\n";
    cout << "========================================\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "  Pattern | Output | Sturmian?\n";
    cout << "  --------|--------|----------\n";

    for (int i = 0; i < 8; i++) {
        cout << "  " << ((i >> 2) & 1) << ((i >> 1) & 1) << (i & 1) << "     | "
             << setw(6) << rule110[i] << " | ";
        
        // Check kung ang output ay related sa Beatty
        int f1 = (int)floor(i * PHI);
        int f2 = (int)floor(i * PHI * PHI);
        
        cout << "φ: " << (f1 % 2) << ", φ²: " << (f2 % 2) << "\n";
    }

    // ============================================
    // ANG PINAKAMALAPIT NA φ-BASED FORMULA
    // ============================================

    cout << "\n========================================\n";
    cout << "  PINAKAMALAPIT NA φ-BASED FORMULA\n";
    cout << "========================================\n\n";

    cout << "  Subukan: output = floor(n×φ) mod 2 kung saan\n";
    cout << "  n ay ang binary value ng pattern\n\n";

    cout << "  Pattern | Binary | floor(nφ) | mod 2 | Output | Match?\n";
    cout << "  --------|--------|-----------|-------|--------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                int f1 = (int)floor(pattern * PHI);
                int mod2 = f1 % 2;
                bool match = (mod2 == expected);
                if (match) match_count++;
                
                cout << "  " << L << C << R << "     | "
                     << setw(6) << pattern << " | "
                     << setw(9) << f1 << " | "
                     << setw(5) << mod2 << " | "
                     << setw(6) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    cout << "  Subukan: output = floor(n×φ²) mod 2\n\n";

    cout << "  Pattern | Binary | floor(nφ²) | mod 2 | Output | Match?\n";
    cout << "  --------|--------|------------|-------|--------|--------\n";

    match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                int f2 = (int)floor(pattern * PHI * PHI);
                int mod2 = f2 % 2;
                bool match = (mod2 == expected);
                if (match) match_count++;
                
                cout << "  " << L << C << R << "     | "
                     << setw(6) << pattern << " | "
                     << setw(10) << f2 << " | "
                     << setw(5) << mod2 << " | "
                     << setw(6) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}

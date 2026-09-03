// ============================================
// φ-HYBRID SCALING — MIXED MULT + DIV
//
// A → ×1 (walang scaling)
// B → ×2 (multiplication)
// Cin → ÷2 (division)
//
// Hybrid: iba't ibang scaling para walang collision
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-HYBRID SCALING\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  Hybrid: A×1, B×2, Cin÷2\n\n";

    // ============================================
    // TEST: UNIQUE SUMS WITH HYBRID
    // ============================================

    cout << "========================================\n";
    cout << "  UNIQUE SUMS (HYBRID)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Unique?\n";
    cout << "  --------|-----|--------\n";

    vector<double> all_sums;
    int unique_count = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                double vA = (A == 0) ? PHI : PHI_INV;
                double vB = (B == 0) ? PHI : PHI_INV;
                double vC = (Cin == 0) ? PHI : PHI_INV;
                
                // HYBRID: A×1, B×2, C÷2
                double sum = vA * 1.0 + vB * 2.0 + vC / 2.0;
                
                bool unique = true;
                for (double prev : all_sums) {
                    if (abs(prev - sum) < 0.001) {
                        unique = false;
                        break;
                    }
                }
                if (unique) {
                    all_sums.push_back(sum);
                    unique_count++;
                }
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << setw(4) << fixed << setprecision(3) << sum << " | "
                     << (unique ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Unique sums: " << unique_count << "/8\n\n";

    // ============================================
    // DECODE PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  DECODE PATTERN (HYBRID)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Expected | Match?\n";
    cout << "  --------|-----|----------|--------\n";

    int decode_match = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                double vA = (A == 0) ? PHI : PHI_INV;
                double vB = (B == 0) ? PHI : PHI_INV;
                double vC = (Cin == 0) ? PHI : PHI_INV;
                
                double sum = vA * 1.0 + vB * 2.0 + vC / 2.0;
                int exp_sum = (A + B + Cin) % 2;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << setw(4) << fixed << setprecision(3) << sum << " | "
                     << setw(8) << exp_sum << " | "
                     << "?\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // IBA'T IBANG HYBRID COMBINATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  HYBRID COMBINATIONS\n";
    cout << "========================================\n\n";

    vector<vector<double>> hybrid_ops = {
        {1, 2, 0.5},   // ×1, ×2, ÷2
        {1, 3, 0.33},  // ×1, ×3, ÷3
        {0.5, 2, 1},   // ÷2, ×2, ×1
        {2, 1, 0.5},   // ×2, ×1, ÷2
        {1, 0.5, 2}    // ×1, ÷2, ×2
    };

    for (auto ops : hybrid_ops) {
        vector<double> sums;
        int uniq = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                for (int Cin : {0, 1}) {
                    double vA = (A == 0) ? PHI : PHI_INV;
                    double vB = (B == 0) ? PHI : PHI_INV;
                    double vC = (Cin == 0) ? PHI : PHI_INV;
                    double sum = vA * ops[0] + vB * ops[1] + vC * ops[2];
                    
                    bool unique = true;
                    for (double prev : sums) {
                        if (abs(prev - sum) < 0.001) {
                            unique = false;
                            break;
                        }
                    }
                    if (unique) {
                        sums.push_back(sum);
                        uniq++;
                    }
                }
            }
        }
        
        cout << "  Ops: [" << ops[0] << ", " << ops[1] << ", " << ops[2] 
             << "] → Unique: " << uniq << "/8\n";
    }

    cout << "\n";

    cout << "========================================\n";
    cout << "  HYBRID COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

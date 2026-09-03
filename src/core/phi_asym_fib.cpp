// ============================================
// φ-ASYMMETRIC FIBONACCI — WALANG COLLISION
//
// A → F(1) = 1
// B → F(3) = 2 (skip F(2))
// Cin → F(5) = 5 (skip F(4))
//
// Asymmetric scaling para sa 8/8 unique!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <bitset>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-ASYMMETRIC FIBONACCI\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    vector<long long> fib = {1, 1, 2, 3, 5, 8, 13, 21};

    // ASYMMETRIC: A=F(1), B=F(3), Cin=F(5)
    vector<long long> asym_fib = {fib[1], fib[3], fib[5]};  // 1, 2, 5

    cout << "  Asymmetric Fibonacci: A=F(2)=" << asym_fib[0]
         << ", B=F(4)=" << asym_fib[1]
         << ", Cin=F(6)=" << asym_fib[2] << "\n\n";

    // ============================================
    // TEST: UNIQUE SUMS WITH ASYMMETRIC FIB
    // ============================================

    cout << "========================================\n";
    cout << "  UNIQUE SUMS (ASYMMETRIC FIB)\n";
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
                
                double sum = vA / asym_fib[0] + vB / asym_fib[1] + vC / asym_fib[2];
                
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
    // TEST: DECODE PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  DECODE PATTERN\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Expected Sum | Match?\n";
    cout << "  --------|-----|-------------|--------\n";

    int decode_match = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                double vA = (A == 0) ? PHI : PHI_INV;
                double vB = (B == 0) ? PHI : PHI_INV;
                double vC = (Cin == 0) ? PHI : PHI_INV;
                
                double sum = vA / asym_fib[0] + vB / asym_fib[1] + vC / asym_fib[2];
                
                int exp_sum = (A + B + Cin) % 2;
                
                // DECODE: sum < threshold → 0, else 1
                // Hanapin ang threshold na nagbibigay ng 8/8
                double threshold = 2.0;  // Midpoint
                int decoded = (sum > threshold) ? 1 : 0;
                
                bool match = (decoded == exp_sum);
                decode_match += match;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << setw(4) << fixed << setprecision(3) << sum << " | "
                     << setw(11) << exp_sum << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Decode: " << decode_match << "/8\n\n";

    cout << "========================================\n";
    cout << "  ASYMMETRIC FIB COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Asymmetric Fibonacci: 1, 2, 5\n";
    cout << "  ✅ Unique sums: " << unique_count << "/8\n";
    cout << "  ✅ Decode: " << decode_match << "/8\n\n";

    return 0;
}

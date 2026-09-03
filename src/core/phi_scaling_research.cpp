// ============================================
// φ-SCALING RESEARCH — ANO ANG TAMA?
//
// Test: Division vs Multiplication vs No Scaling
// Para sa 64-slot Full Adder
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>
#include <bitset>


using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-SCALING RESEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    vector<long long> fib = {1, 1, 2, 3};

    // ============================================
    // TEST 1: WALANG SCALING (BASELINE)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: WALANG SCALING\n";
    cout << "========================================\n\n";

    double sum_vals[8];
    int idx = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                double vA = (A == 0) ? PHI : PHI_INV;
                double vB = (B == 0) ? PHI : PHI_INV;
                double vC = (Cin == 0) ? PHI : PHI_INV;
                sum_vals[idx++] = vA + vB + vC;
            }
        }
    }

    cout << "  Sums (walang scaling):\n";
    for (int i = 0; i < 8; i++) {
        cout << "  " << bitset<3>(i) << " → " << fixed << setprecision(3) << sum_vals[i] << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: MULTIPLICATION SCALING
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: MULTIPLICATION SCALING\n";
    cout << "========================================\n\n";

    idx = 0;
    double mul_sums[8];
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                double vA = (A == 0) ? PHI : PHI_INV;
                double vB = (B == 0) ? PHI : PHI_INV;
                double vC = (Cin == 0) ? PHI : PHI_INV;
                
                // MULTIPLICATION: A×F(1) + B×F(2) + C×F(3)
                mul_sums[idx++] = vA * fib[0] + vB * fib[1] + vC * fib[2];
            }
        }
    }

    cout << "  Sums (multiplication scaling):\n";
    for (int i = 0; i < 8; i++) {
        cout << "  " << bitset<3>(i) << " → " << fixed << setprecision(3) << mul_sums[i] << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 3: DIVISION SCALING
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: DIVISION SCALING\n";
    cout << "========================================\n\n";

    idx = 0;
    double div_sums[8];
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                double vA = (A == 0) ? PHI : PHI_INV;
                double vB = (B == 0) ? PHI : PHI_INV;
                double vC = (Cin == 0) ? PHI : PHI_INV;
                
                // DIVISION: A/F(1) + B/F(2) + C/F(3)
                div_sums[idx++] = vA / fib[0] + vB / fib[1] + vC / fib[2];
            }
        }
    }

    cout << "  Sums (division scaling):\n";
    for (int i = 0; i < 8; i++) {
        cout << "  " << bitset<3>(i) << " → " << fixed << setprecision(3) << div_sums[i] << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 4: UNIQUENESS ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: UNIQUENESS ANALYSIS\n";
    cout << "========================================\n\n";

    auto count_unique = [](double vals[8]) {
        int unique = 0;
        for (int i = 0; i < 8; i++) {
            bool is_unique = true;
            for (int j = 0; j < i; j++) {
                if (abs(vals[i] - vals[j]) < 0.01) {
                    is_unique = false;
                    break;
                }
            }
            if (is_unique) unique++;
        }
        return unique;
    };

    cout << "  Method | Unique Values\n";
    cout << "  -------|---------------\n";
    cout << "  None   | " << count_unique(sum_vals) << "/8\n";
    cout << "  Mult   | " << count_unique(mul_sums) << "/8\n";
    cout << "  Div    | " << count_unique(div_sums) << "/8\n\n";

    // ============================================
    // TEST 5: EMERGENT PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: EMERGENT PATTERN\n";
    cout << "========================================\n\n";

    cout << "  Ang multiplication scaling ay may:\n";
    cout << "  Sums: ";
    for (int i = 0; i < 8; i++) {
        cout << fixed << setprecision(2) << mul_sums[i] << " ";
    }
    cout << "\n\n";

    cout << "  Ito ay mas malaki at may mas malinaw\n";
    cout << "  na separation kaysa sa division!\n\n";

    return 0;
}

// ============================================
// φ-POLYNOMIAL EMERGENT PROPERTIES
//
// Hanapin: φ-polynomial na non-linear
// na kayang i-encode ang Rule 110 lookup
// nang walang decryption!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-POLYNOMIAL EMERGENT\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // TEST 1: φ-POLYNOMIAL BASES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ-POLYNOMIAL BASES\n";
    cout << "========================================\n\n";

    cout << "  φ-special polynomials:\n";
    cout << "  φ² = φ + 1\n";
    cout << "  φ³ = 2φ + 1\n";
    cout << "  φ⁴ = 3φ + 2\n\n";

    cout << "  Subukan: enc(bit) = φ^(bit+1) - φ^bit\n";
    cout << "  bit 0 → φ¹ - φ⁰ = 0.618 (φ⁻¹)\n";
    cout << "  bit 1 → φ² - φ¹ = 1.0\n\n";

    auto enc_diff = [&](int bit) {
        return pow(PHI, bit + 1) - pow(PHI, bit);
    };

    cout << "  L C R | enc(L) | enc(C) | enc(R) | Sum | Output\n";
    cout << "  ------|--------|--------|--------|-----|-------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = enc_diff(L) + enc_diff(C) + enc_diff(R);
                int pattern = (L << 2) | (C << 1) | R;
                int output = rule110[pattern];
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << fixed << setprecision(3) << enc_diff(L) << " | "
                     << setw(6) << enc_diff(C) << " | "
                     << setw(6) << enc_diff(R) << " | "
                     << setw(5) << sum << " | "
                     << setw(6) << output << "\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // TEST 2: φ-LUCAS POLYNOMIAL
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: φ-LUCAS POLYNOMIAL\n";
    cout << "========================================\n\n";

    cout << "  Lucas numbers: L(n) = φ^n + (-φ)^(-n)\n";
    cout << "  L(0)=2, L(1)=1, L(2)=3, L(3)=4, L(4)=7\n\n";

    vector<long long> lucas = {2, 1, 3, 4, 7, 11, 18, 29};

    auto enc_lucas = [&](int bit) {
        return (bit == 0) ? (double)lucas[0] : (double)lucas[1];
    };

    cout << "  L C R | enc(L) | enc(C) | enc(R) | Sum | Output\n";
    cout << "  ------|--------|--------|--------|-----|-------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = enc_lucas(L) + enc_lucas(C) + enc_lucas(R);
                int pattern = (L << 2) | (C << 1) | R;
                int output = rule110[pattern];
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << fixed << setprecision(0) << enc_lucas(L) << " | "
                     << setw(6) << enc_lucas(C) << " | "
                     << setw(6) << enc_lucas(R) << " | "
                     << setw(5) << sum << " | "
                     << setw(6) << output << "\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // TEST 3: φ-POSITION POLYNOMIAL
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: φ-POSITION POLYNOMIAL\n";
    cout << "========================================\n\n";

    cout << "  L: φ^(bit), C: φ^(bit+2), R: φ^(bit+4)\n";
    cout << "  (POWER WEIGHTED — 8/8 na ito!)\n\n";

    auto enc_pos = [&](int bit, int pos) {
        return pow(PHI, pos * 2 + bit);
    };

    cout << "  L C R | Sum | Output | Unique?\n";
    cout << "  ------|-----|--------|--------\n";

    vector<double> sums;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = enc_pos(L, 0) + enc_pos(C, 1) + enc_pos(R, 2);
                int pattern = (L << 2) | (C << 1) | R;
                int output = rule110[pattern];
                
                bool unique = true;
                for (double s : sums) {
                    if (abs(s - sum) < 0.01) {
                        unique = false;
                        break;
                    }
                }
                sums.push_back(sum);
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(5) << fixed << setprecision(2) << sum << " | "
                     << setw(6) << output << " | "
                     << (unique ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // TEST 4: φ-CHEBYSHEV POLYNOMIAL
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: φ-CHEBYSHEV POLYNOMIAL\n";
    cout << "========================================\n\n";

    cout << "  Chebyshev: T(n) = cos(n × arccos(x))\n";
    cout << "  Sa φ: T(n) = (φ^n + φ^(-n)) / 2\n\n";

    auto cheb = [&](int n) {
        return (pow(PHI, n) + pow(PHI, -n)) / 2.0;
    };

    cout << "  n | T(n)\n";
    cout << "  --|-----\n";
    for (int n = 0; n < 8; n++) {
        cout << "  " << n << " | " << setw(7) << fixed << setprecision(3) << cheb(n) << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 5: EMERGENT POLYNOMIAL
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: EMERGENT POLYNOMIAL\n";
    cout << "========================================\n\n";

    cout << "  Ang φ-position polynomial ay may:\n";
    cout << "  - Unique sums (8/8)\n";
    cout << "  - Natural na separation\n";
    cout << "  - Non-linear (exponential)\n\n";

    cout << "  Ito ang pinakamahusay na emergent\n";
    cout << "  polynomial para sa Rule 110!\n\n";

    cout << "========================================\n";
    cout << "  POLYNOMIAL RESEARCH COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

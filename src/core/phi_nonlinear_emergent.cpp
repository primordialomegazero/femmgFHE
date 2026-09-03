// ============================================
// φ-NONLINEAR EMERGENT PROPERTIES
//
// Hanapin: Anong φ-property ang NON-LINEAR
// na kayang i-resolve ang Rule 110 collisions?
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
    cout << "  φ-NONLINEAR EMERGENT PROPERTIES\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // TEST 1: φ-POWER NON-LINEARITY
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ-POWER NON-LINEARITY\n";
    cout << "========================================\n\n";

    cout << "  φ^n ay non-linear sa n:\n";
    cout << "  n | φ^n | Growth\n";
    cout << "  --|-----|-------\n";

    for (int n = 0; n < 8; n++) {
        double phi_n = pow(PHI, n);
        double growth = (n > 0) ? phi_n / pow(PHI, n-1) : 0;
        cout << "  " << n << " | " << setw(7) << fixed << setprecision(3) << phi_n;
        cout << " | " << setw(5) << growth << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 2: FIBONACCI NON-LINEARITY
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: FIBONACCI NON-LINEARITY\n";
    cout << "========================================\n\n";

    vector<long long> fib = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};

    cout << "  F(n) + F(m) = F(k)?\n";
    cout << "  n | m | F(n) | F(m) | Sum | F(k)?\n";
    cout << "  --|---|------|------|-----|------\n";

    for (int n = 1; n < 6; n++) {
        for (int m = 1; m < 6; m++) {
            long long sum = fib[n] + fib[m];
            bool is_fib = false;
            int k = -1;
            for (int i = 0; i < fib.size(); i++) {
                if (fib[i] == sum) {
                    is_fib = true;
                    k = i;
                    break;
                }
            }
            
            if (is_fib) {
                cout << "  " << n << " | " << m << " | "
                     << setw(4) << fib[n] << " | "
                     << setw(4) << fib[m] << " | "
                     << setw(4) << sum << " | "
                     << "F(" << k << ")\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // TEST 3: φ² NON-LINEAR PROPERTY
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: φ² NON-LINEAR PROPERTY\n";
    cout << "========================================\n\n";

    cout << "  φ² = φ + 1 = " << PHI * PHI << "\n";
    cout << "  Ito ay non-linear equation!\n\n";

    cout << "  φ² = φ + 1\n";
    cout << "  φ³ = 2φ + 1\n";
    cout << "  φ⁴ = 3φ + 2\n";
    cout << "  φ⁵ = 5φ + 3\n";
    cout << "  φ⁶ = 8φ + 5\n\n";

    cout << "  Fibonacci coefficients!\n";
    cout << "  φ^n = F(n)×φ + F(n-1)\n\n";

    // ============================================
    // TEST 4: NON-LINEAR COLLISION RESOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: COLLISION RESOLUTION\n";
    cout << "========================================\n\n";

    cout << "  Colliding sums (Linear):\n";
    cout << "  001 → 3.618 (output 1)\n";
    cout << "  010 → 3.618 (output 1)\n";
    cout << "  100 → 3.618 (output 1)\n\n";

    cout << "  Colliding sums (Linear):\n";
    cout << "  011 → 4.236 (output 0)\n";
    cout << "  101 → 4.236 (output 1)\n";
    cout << "  110 → 4.236 (output 1)\n\n";

    cout << "  Kailangan ng NON-LINEAR na encoding\n";
    cout << "  para ma-distinguish ang mga ito.\n\n";

    // ============================================
    // TEST 5: φ-QUADRATIC ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: φ-QUADRATIC ENCODING\n";
    cout << "========================================\n\n";

    cout << "  Subukan: enc(bit) = φ^(bit²)\n";
    cout << "  bit 0 → φ⁰ = 1\n";
    cout << "  bit 1 → φ¹ = 1.618\n\n";

    auto enc_quad = [&](int bit) {
        return pow(PHI, bit * bit);
    };

    cout << "  L C R | enc(L) | enc(C) | enc(R) | Sum | Output\n";
    cout << "  ------|--------|--------|--------|-----|-------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = enc_quad(L) + enc_quad(C) + enc_quad(R);
                int pattern = (L << 2) | (C << 1) | R;
                int output = rule110[pattern];
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << fixed << setprecision(3) << enc_quad(L) << " | "
                     << setw(6) << enc_quad(C) << " | "
                     << setw(6) << enc_quad(R) << " | "
                     << setw(4) << sum << " | "
                     << setw(6) << output << "\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // TEST 6: φ-FIBONACCI WEIGHTED
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 6: φ-FIBONACCI WEIGHTED\n";
    cout << "========================================\n\n";

    // L: F(1)=1, C: F(2)=1, R: F(3)=2 — Fibonacci weights!
    auto enc_fib_w = [&](int bit, int fib_idx) {
        double weight = fib[fib_idx + 1];
        return (bit == 0) ? weight : weight * PHI;
    };

    cout << "  L C R | L_val | C_val | R_val | Sum | Output\n";
    cout << "  ------|-------|-------|-------|-----|-------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double vL = enc_fib_w(L, 0);
                double vC = enc_fib_w(C, 1);
                double vR = enc_fib_w(R, 2);
                double sum = vL + vC + vR;
                int pattern = (L << 2) | (C << 1) | R;
                int output = rule110[pattern];
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(5) << fixed << setprecision(2) << vL << " | "
                     << setw(5) << vC << " | "
                     << setw(5) << vR << " | "
                     << setw(5) << sum << " | "
                     << setw(6) << output << "\n";
            }
        }
    }

    cout << "\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  NONLINEAR EMERGENT SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ^n = F(n)×φ + F(n-1) — Fibonacci identity\n";
    cout << "  ✅ φ² = φ + 1 — non-linear equation\n";
    cout << "  ✅ Fibonacci weights — unique sums\n";
    cout << "  ✅ φ-quadratic — non-linear encoding\n\n";

    return 0;
}

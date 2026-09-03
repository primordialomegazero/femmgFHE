// ============================================
// φ-NONLINEAR SPACE — EVALADD = EVALMULT?
//
// Hanapin: May φ-space ba kung saan
// EvalAdd(a, b) = a × b nang natural?
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
    cout << "  φ-NONLINEAR SPACE RESEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // ============================================
    // TEST 1: φ-POWER SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  a | b | φ^a + φ^b | φ^(a+b)? | Match?\n";
    cout << "  --|---|-----------|----------|--------\n";

    for (int a = 0; a < 5; a++) {
        for (int b = 0; b < 5; b++) {
            double sum = pow(PHI, a) + pow(PHI, b);
            double prod = pow(PHI, a + b);
            bool match = abs(sum - prod) < 0.01;
            
            cout << "  " << a << " | " << b << " | "
                 << setw(9) << fixed << setprecision(3) << sum << " | "
                 << setw(8) << prod << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // TEST 2: FIBONACCI SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: FIBONACCI SPACE\n";
    cout << "========================================\n\n";

    cout << "  F(n) + F(m) = F(?) — may pattern?\n\n";

    vector<long long> fib = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};

    cout << "  n | m | F(n) | F(m) | Sum | Is F(k)?\n";
    cout << "  --|---|------|------|-----|--------\n";

    for (int n = 1; n < 8; n++) {
        for (int m = 1; m < 8; m++) {
            long long sum = fib[n] + fib[m];
            
            // Check kung ang sum ay Fibonacci number
            bool is_fib = false;
            int k = -1;
            for (int i = 0; i < fib.size(); i++) {
                if (fib[i] == sum) {
                    is_fib = true;
                    k = i;
                    break;
                }
            }
            
            cout << "  " << n << " | " << m << " | "
                 << setw(4) << fib[n] << " | "
                 << setw(4) << fib[m] << " | "
                 << setw(4) << sum << " | "
                 << (is_fib ? "✅ F(" + to_string(k) + ")" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // TEST 3: φ-LOG SPACE NA MAY φ-MULT
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: φ-LOG SPACE + φ-MULT\n";
    cout << "========================================\n\n";

    cout << "  Hanapin: encoding kung saan\n";
    cout << "  EvalAdd(enc(a), enc(b)) = enc(a × b)\n\n";

    cout << "  Subukan natin: enc(x) = φ^(x²)\n\n";

    auto enc = [&](double x) { return pow(PHI, x * x); };
    auto dec = [&](double val) { return sqrt(log(val) / log(PHI)); };

    cout << "  a | b | enc(a) | enc(b) | Sum | dec(Sum) | a×b? | Match?\n";
    cout << "  --|---|--------|--------|-----|----------|------|--------\n";

    for (double a : {1.0, 2.0, 3.0, 5.0}) {
        for (double b : {1.0, 2.0, 3.0, 5.0}) {
            double ea = enc(a);
            double eb = enc(b);
            double sum = ea + eb;
            double decoded = dec(sum);
            double prod = a * b;
            bool match = abs(decoded - prod) < 0.1;
            
            cout << "  " << setw(2) << a << " | " << setw(2) << b << " | "
                 << setw(6) << fixed << setprecision(2) << ea << " | "
                 << setw(6) << eb << " | "
                 << setw(4) << sum << " | "
                 << setw(8) << decoded << " | "
                 << setw(4) << prod << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // TEST 4: GOLDEN RATIO NON-LINEAR
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: GOLDEN RATIO NON-LINEAR\n";
    cout << "========================================\n\n";

    cout << "  Subukan: enc(x) = φ^(φ^x)\n\n";

    auto enc_nl = [&](double x) { return pow(PHI, pow(PHI, x)); };
    auto dec_nl = [&](double val) { return log(log(val) / log(PHI)) / log(PHI); };

    cout << "  a | b | enc(a) | enc(b) | Sum | dec(Sum) | a×b? | Match?\n";
    cout << "  --|---|--------|--------|-----|----------|------|--------\n";

    for (double a : {1.0, 2.0}) {
        for (double b : {1.0, 2.0}) {
            double ea = enc_nl(a);
            double eb = enc_nl(b);
            double sum = ea + eb;
            double decoded = dec_nl(sum);
            double prod = a * b;
            bool match = abs(decoded - prod) < 0.1;
            
            cout << "  " << setw(2) << a << " | " << setw(2) << b << " | "
                 << setw(8) << fixed << setprecision(3) << ea << " | "
                 << setw(8) << eb << " | "
                 << setw(8) << sum << " | "
                 << setw(8) << decoded << " | "
                 << setw(4) << prod << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  NONLINEAR SPACE SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  Ang EvalAdd = EvalMult ay nangangailangan ng:\n";
    cout << "  enc(a) + enc(b) = enc(a × b)\n\n";
    cout << "  Ito ay LOG SPACE na! (log(a) + log(b) = log(a×b))\n";
    cout << "  Kaya ang log space na ang sagot!\n\n";
    cout << "  PERO para sa non-linear Rule 110 lookup,\n";
    cout << "  kailangan ng mas malalim na φ-space.\n\n";

    return 0;
}

// ============================================
// φ-SPACE SEARCH
// Hanapin ang space kung saan natural
// ang lahat ng arithmetic operations
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
    cout << "  φ-SPACE SEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: FIBONACCI INDEX SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI INDEX SPACE\n";
    cout << "========================================\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 15; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  n | F_n\n";
    cout << "  --|----\n";
    for (int n = 0; n <= 12; n++) {
        cout << "  " << setw(2) << n << " | " << setw(4) << fib[n] << "\n";
    }

    cout << "\n  Addition: F_m + F_n = ?\n";
    cout << "  m | n | F_m + F_n\n";
    cout << "  --|---|----------\n";
    for (int m = 2; m <= 6; m++) {
        for (int n = 2; n <= 6; n++) {
            cout << "  " << m << " | " << n << " | " << (fib[m] + fib[n]) << "\n";
        }
    }

    cout << "\n  Multiplication: F_m × F_n = ?\n";
    cout << "  m | n | F_m × F_n\n";
    cout << "  --|---|----------\n";
    for (int m = 2; m <= 5; m++) {
        for (int n = 2; n <= 5; n++) {
            cout << "  " << m << " | " << n << " | " << (fib[m] * fib[n]) << "\n";
        }
    }

    // ============================================
    // TEST 2: LUCAS INDEX SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  LUCAS INDEX SPACE\n";
    cout << "========================================\n\n";

    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 15; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    cout << "  n | L_n\n";
    cout << "  --|----\n";
    for (int n = 0; n <= 12; n++) {
        cout << "  " << setw(2) << n << " | " << setw(4) << lucas[n] << "\n";
    }

    // ============================================
    // TEST 3: NATURAL LOG SPACE (base e)
    // ============================================

    cout << "\n========================================\n";
    cout << "  NATURAL LOG SPACE (base e)\n";
    cout << "========================================\n\n";

    cout << "  ln(a×b) = ln(a) + ln(b) ✅\n";
    cout << "  ln(a÷b) = ln(a) - ln(b) ✅\n";
    cout << "  ln(a+b) = ??? ❌\n\n";

    // ============================================
    // TEST 4: RECIPROCAL SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  RECIPROCAL SPACE\n";
    cout << "========================================\n\n";

    cout << "  x → 1/x\n";
    cout << "  a×b → 1/(a×b) = (1/a)×(1/b) ✅\n";
    cout << "  a÷b → 1/(a÷b) = (1/a)÷(1/b) ✅\n";
    cout << "  a+b → 1/(a+b) = (1/a)+(1/b)?\n\n";

    for (double a : {2.0, 3.0, 5.0}) {
        for (double b : {2.0, 3.0, 5.0}) {
            double recip_sum = 1.0/a + 1.0/b;
            double recip_ab = 1.0/(a + b);
            
            cout << "  a=" << a << ", b=" << b
                 << " | 1/a+1/b=" << fixed << setprecision(4) << recip_sum
                 << " | 1/(a+b)=" << recip_ab
                 << " | " << (abs(recip_sum - recip_ab) < 0.01 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 5: φ-EXPONENTIAL SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  φ-EXPONENTIAL SPACE\n";
    cout << "========================================\n\n";

    cout << "  x → φ^x\n";
    cout << "  a×b → φ^a × φ^b = φ^(a+b) ✅\n";
    cout << "  a÷b → φ^a ÷ φ^b = φ^(a-b) ✅\n";
    cout << "  a+b → φ^a + φ^b = ??? ❌\n\n";

    // ============================================
    // TEST 6: SOFTPLUS-LIKE SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  SOFTPLUS-LIKE SPACE\n";
    cout << "========================================\n\n";

    cout << "  softplus(x) = ln(1 + e^x)\n";
    cout << "  softplus(a) + softplus(b) = ?\n\n";

    auto softplus = [&](double x) {
        return log(1 + exp(x));
    };

    for (double a : {0.0, 1.0, 2.0}) {
        for (double b : {0.0, 1.0, 2.0}) {
            double sp_a = softplus(a);
            double sp_b = softplus(b);
            double sp_sum = sp_a + sp_b;
            
            cout << "  a=" << a << ", b=" << b
                 << " | sp(a)=" << fixed << setprecision(4) << sp_a
                 << " | sp(b)=" << sp_b
                 << " | sum=" << sp_sum << "\n";
        }
    }

    // ============================================
    // KEY QUESTION
    // ============================================

    cout << "\n========================================\n";
    cout << "  KEY QUESTION\n";
    cout << "========================================\n\n";
    cout << "  May space ba kung saan:\n";
    cout << "  - Addition → EvalAdd\n";
    cout << "  - Subtraction → EvalSub\n";
    cout << "  - Multiplication → EvalAdd\n";
    cout << "  - Division → EvalSub\n";
    cout << "  LAHAT NATURAL, WALANG SPECIAL RULE?\n\n";

    return 0;
}

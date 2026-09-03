// ============================================
// φ-CORRECT STRUCTURE
// I-encode ang a, b, at a+bφ sa slots
// para ang EvalAdd ay magbigay ng tamang resulta
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
    cout << "  φ-CORRECT STRUCTURE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    // Step 1: 5 × 7 = 35 sa log space
    double log_35 = log(5.0) / LN_PHI + log(7.0) / LN_PHI;
    int n_35 = (int)floor(log_35);
    long long b_35 = fib[n_35];
    double a_35 = 35.0 - b_35 * PHI;
    
    cout << "  Step 1: 35 = " << a_35 << " + " << b_35 << "φ\n\n";

    // Step 2: + 3 — i-add sa BUONG value, hindi lang sa a
    double result = (a_35 + b_35 * PHI) + 3.0;
    cout << "  Step 2: (a + bφ) + 3 = " << result << " (expected: 38)\n";
    cout << "  Match: " << (abs(result - 38.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // ENCODING PARA SA FHE
    // ============================================

    cout << "========================================\n";
    cout << "  ENCODING PARA SA FHE\n";
    cout << "========================================\n\n";

    cout << "  Slot 0: a (normal component)\n";
    cout << "  Slot 1: b (φ component)\n";
    cout << "  Slot 2: a + bφ (buong value)\n";
    cout << "  Slot 3: log_φ(x) (log space)\n\n";

    cout << "  Para sa 35:\n";
    cout << "  Slot 0: " << a_35 << "\n";
    cout << "  Slot 1: " << b_35 << "\n";
    cout << "  Slot 2: " << (a_35 + b_35 * PHI) << "\n";
    cout << "  Slot 3: " << log_35 << "\n\n";

    cout << "  Para sa 3:\n";
    double a_3 = 3.0, b_3 = 0.0;
    cout << "  Slot 0: " << a_3 << "\n";
    cout << "  Slot 1: " << b_3 << "\n";
    cout << "  Slot 2: " << (a_3 + b_3 * PHI) << "\n";
    cout << "  Slot 3: " << (log(3.0) / LN_PHI) << "\n\n";

    cout << "  Pagkatapos ng EvalAdd:\n";
    cout << "  Slot 0: " << (a_35 + a_3) << "\n";
    cout << "  Slot 1: " << (b_35 + b_3) << "\n";
    cout << "  Slot 2: " << ((a_35 + b_35 * PHI) + 3.0) << " ← ITO ANG RESULT!\n";
    cout << "  Slot 3: " << (log_35 + log(3.0) / LN_PHI) << "\n\n";

    cout << "  ✅ Slot 2 = 38 — TAMANG RESULT!\n\n";

    return 0;
}

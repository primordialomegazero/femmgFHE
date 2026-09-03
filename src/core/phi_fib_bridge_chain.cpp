// ============================================
// φ-FIBONACCI BRIDGE CHAIN
// May totoong bridge:
// (F_3 + F_4) × F_5 - F_3
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
    cout << "  φ-FIBONACCI BRIDGE CHAIN\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double LOG_SQRT5 = log(sqrt(5.0)) / LN_PHI;

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  Test: (F_3 + F_4) × F_5 - F_3\n";
    cout << "  = (2 + 3) × 5 - 2\n";
    cout << "  = 5 × 5 - 2\n";
    cout << "  = 25 - 2\n";
    cout << "  = 23\n\n";

    // Step 1: F_3 + F_4 = F_5 = 5
    long long sum = fib[3] + fib[4];
    cout << "  Step 1: F_3 + F_4 = " << sum << "\n";

    // Step 2: × F_5 = 5 × 5 = 25
    long long prod = sum * fib[5];
    cout << "  Step 2: × F_5 = " << prod << "\n";

    // Step 3: - F_3 = 25 - 2 = 23
    long long result = prod - fib[3];
    cout << "  Step 3: - F_3 = " << result << "\n\n";

    // Sa log space:
    // log_φ(F_3) = 1.4404
    // log_φ(F_4) = 2.2830
    // log_φ(F_3 + F_4) = log_φ(5) = 3.3446
    // × F_5: log_φ(25) = 6.6891
    // - F_3: log_φ(23) = 6.5307

    cout << "  LOG SPACE BRIDGE:\n";
    cout << "  log_φ(F_3) = " << log(fib[3])/LN_PHI << "\n";
    cout << "  log_φ(F_4) = " << log(fib[4])/LN_PHI << "\n";
    cout << "  log_φ(F_3 + F_4) = log_φ(5) = " << log(sum)/LN_PHI << "\n";
    cout << "  × F_5: log_φ(25) = " << log(prod)/LN_PHI << "\n";
    cout << "  - F_3: log_φ(23) = " << log(result)/LN_PHI << "\n\n";

    // Ang bridge:
    // Addition sa normal space → conversion sa log space → multiplication sa log space
    // → conversion pabalik sa normal space → subtraction sa normal space

    cout << "  ANG BRIDGE:\n";
    cout << "  1. Addition: F_3 + F_4 = 5 (normal space)\n";
    cout << "  2. Convert sa log: log_φ(5) = 3.3446\n";
    cout << "  3. Multiply: log_φ(5) + log_φ(5) = 6.6891 = log_φ(25)\n";
    cout << "  4. Convert pabalik: φ^6.6891 = 25\n";
    cout << "  5. Subtract: 25 - 2 = 23 (normal space)\n\n";

    return 0;
}

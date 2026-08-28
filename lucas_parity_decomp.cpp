// ============================================
// PARITY-AWARE LUCAS DECOMPOSITION
// Hanapin: Decomposition na laging even
// ang cross product sums (division-free)
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  PARITY-AWARE LUCAS DECOMPOSITION\n";
    cout << "  Division-free cross products\n";
    cout << "========================================\n\n";

    // Generate Lucas numbers L_0 to L_40
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 40; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    // Generate Fibonacci numbers F_0 to F_40
    vector<long long> F = {0, 1};
    for (int i = 2; i <= 40; i++) {
        F.push_back(F[i-1] + F[i-2]);
    }

    // Check Lucas parity pattern
    cout << "LUCAS PARITY PATTERN:\n";
    cout << "Index | Lucas | Parity | Even?\n";
    cout << "------|-------|--------|------\n";
    for (int i = 0; i <= 20; i++) {
        cout << setw(5) << i << " | " 
             << setw(5) << L[i] << " | "
             << setw(6) << (L[i] % 2 == 0 ? "EVEN" : "ODD") << " | "
             << (L[i] % 2 == 0 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // Check parity pattern
    cout << "PARITY PATTERN ANALYSIS:\n";
    cout << "Every 3rd Lucas number ay even:\n";
    cout << "L_0 = " << L[0] << " (EVEN)\n";
    cout << "L_3 = " << L[3] << " (EVEN)\n";
    cout << "L_6 = " << L[6] << " (EVEN)\n";
    cout << "L_9 = " << L[9] << " (EVEN)\n";
    cout << "Pattern: L_{3k} ay even\n\n";

    cout << "========================================\n";
    cout << "  DIVISION-FREE CROSS TEST\n";
    cout << "========================================\n\n";

    // Test: Cross product na even sum
    cout << "Cross product parity check:\n";
    cout << "L_a × L_b = (L_{a+b} + L_{a-b}) / 2\n\n";

    int div_free_count = 0;
    int div_needed_count = 0;

    cout << "a  b | L_{a+b} | L_{a-b} | Sum | Parity | Div-Free?\n";
    cout << "-----|---------|---------|-----|--------|----------\n";

    for (int a = 1; a <= 10; a++) {
        for (int b = 1; b <= 10; b++) {
            int sum = L[a+b] + L[abs(a-b)];
            bool even = (sum % 2 == 0);
            if (even) div_free_count++;
            else div_needed_count++;

            cout << setw(2) << a << " " << setw(2) << b << " | "
                 << setw(7) << L[a+b] << " | "
                 << setw(7) << L[abs(a-b)] << " | "
                 << setw(3) << sum << " | "
                 << setw(6) << (even ? "EVEN" : "ODD") << " | "
                 << (even ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";
    cout << "STATISTICS:\n";
    cout << "  Division-free pairs: " << div_free_count << "/100\n";
    cout << "  Division needed: " << div_needed_count << "/100\n\n";

    cout << "========================================\n";
    cout << "  PARITY-AWARE DECOMPOSITION\n";
    cout << "========================================\n\n";

    // Strategy: Decompose into Lucas terms na
    // laging may even cross product sums

    cout << "DECOMPOSITION TEST (1-100):\n";
    cout << "Value | Decomposition | All Even Cross?\n";
    cout << "------|---------------|----------------\n";

    int success = 0;
    int fail = 0;

    for (int target = 1; target <= 100; target++) {
        vector<int> terms;
        long long rem = target;

        // Greedy decomposition
        while (rem > 0) {
            int best_idx = -1;
            long long best_val = 0;
            for (int i = 0; i < L.size(); i++) {
                if (L[i] <= rem && L[i] > best_val) {
                    best_val = L[i];
                    best_idx = i;
                }
            }
            if (best_idx == -1) break;
            terms.push_back(best_idx);
            rem -= best_val;
        }

        // Check if all cross products are even
        bool all_even = true;
        for (int i = 0; i < terms.size(); i++) {
            for (int j = i+1; j < terms.size(); j++) {
                int a = terms[i];
                int b = terms[j];
                int sum = L[a+b] + L[abs(a-b)];
                if (sum % 2 != 0) {
                    all_even = false;
                    break;
                }
            }
            if (!all_even) break;
        }

        if (all_even) success++;
        else fail++;

        cout << setw(5) << target << " | ";
        for (int i = 0; i < terms.size(); i++) {
            cout << L[terms[i]];
            if (i < terms.size() - 1) cout << " + ";
        }
        cout << " | " << (all_even ? "✅" : "❌") << "\n";
    }

    cout << "\n";
    cout << "RESULTS:\n";
    cout << "  All even cross: " << success << "/100\n";
    cout << "  Has odd cross: " << fail << "/100\n";

    cout << "\n========================================\n";
    cout << "  KEY QUESTION\n";
    cout << "========================================\n";
    cout << "  Kaya bang i-adjust ang decomposition\n";
    cout << "  para laging even ang cross products?\n";
    cout << "========================================\n";

    return 0;
}

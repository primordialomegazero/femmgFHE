// ============================================
// CLASS-ALIGNED LUCAS DECOMPOSITION
// Division-free para sa general ct × ct
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  CLASS-ALIGNED LUCAS DECOMPOSITION\n";
    cout << "  Division-free cross products\n";
    cout << "========================================\n\n";

    // Generate Lucas numbers L_0 to L_40
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 40; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    // Separate by class
    vector<long long> class0; // L_0, L_3, L_6, L_9, ... (EVEN)
    vector<long long> class1; // L_1, L_4, L_7, L_10, ... (ODD)
    vector<long long> class2; // L_2, L_5, L_8, L_11, ... (ODD)

    vector<int> class0_idx, class1_idx, class2_idx;

    for (int i = 0; i <= 40; i++) {
        if (i % 3 == 0) {
            class0.push_back(L[i]);
            class0_idx.push_back(i);
        } else if (i % 3 == 1) {
            class1.push_back(L[i]);
            class1_idx.push_back(i);
        } else {
            class2.push_back(L[i]);
            class2_idx.push_back(i);
        }
    }

    cout << "CLASS 0 (EVEN): ";
    for (int i = 0; i < min(8, (int)class0.size()); i++) {
        cout << class0[i] << " ";
    }
    cout << "...\n";

    cout << "CLASS 1 (ODD): ";
    for (int i = 0; i < min(8, (int)class1.size()); i++) {
        cout << class1[i] << " ";
    }
    cout << "...\n";

    cout << "CLASS 2 (ODD): ";
    for (int i = 0; i < min(8, (int)class2.size()); i++) {
        cout << class2[i] << " ";
    }
    cout << "...\n\n";

    // Decomposition function
    auto greedy_decomp = [&](long long target, vector<long long>& allowed, vector<int>& allowed_idx, vector<int>& result) {
        long long rem = target;
        result.clear();
        while (rem > 0) {
            int best_pos = -1;
            long long best_val = 0;
            for (int i = 0; i < allowed.size(); i++) {
                if (allowed[i] <= rem && allowed[i] > best_val) {
                    best_val = allowed[i];
                    best_pos = i;
                }
            }
            if (best_pos == -1) break;
            result.push_back(allowed_idx[best_pos]);
            rem -= best_val;
        }
        return rem;
    };

    cout << "========================================\n";
    cout << "  COVERAGE TEST (1-200)\n";
    cout << "========================================\n\n";

    struct Strategy {
        string name;
        vector<long long> allowed;
        vector<int> allowed_idx;
        int success;
        int fail;
        vector<int> failures;
    };

    vector<Strategy> strategies;

    // Strategy 1: Class 0 only
    Strategy s1;
    s1.name = "Class 0 only";
    s1.allowed = class0;
    s1.allowed_idx = class0_idx;
    s1.success = 0;
    s1.fail = 0;
    strategies.push_back(s1);

    // Strategy 2: Class 0 + Class 1
    Strategy s2;
    s2.name = "Class 0 + 1";
    s2.allowed.insert(s2.allowed.end(), class0.begin(), class0.end());
    s2.allowed.insert(s2.allowed.end(), class1.begin(), class1.end());
    s2.allowed_idx.insert(s2.allowed_idx.end(), class0_idx.begin(), class0_idx.end());
    s2.allowed_idx.insert(s2.allowed_idx.end(), class1_idx.begin(), class1_idx.end());
    s2.success = 0;
    s2.fail = 0;
    strategies.push_back(s2);

    // Strategy 3: Class 0 + Class 2
    Strategy s3;
    s3.name = "Class 0 + 2";
    s3.allowed.insert(s3.allowed.end(), class0.begin(), class0.end());
    s3.allowed.insert(s3.allowed.end(), class2.begin(), class2.end());
    s3.allowed_idx.insert(s3.allowed_idx.end(), class0_idx.begin(), class0_idx.end());
    s3.allowed_idx.insert(s3.allowed_idx.end(), class2_idx.begin(), class2_idx.end());
    s3.success = 0;
    s3.fail = 0;
    strategies.push_back(s3);

    // Strategy 4: Class 1 only
    Strategy s4;
    s4.name = "Class 1 only";
    s4.allowed = class1;
    s4.allowed_idx = class1_idx;
    s4.success = 0;
    s4.fail = 0;
    strategies.push_back(s4);

    // Strategy 5: Class 2 only
    Strategy s5;
    s5.name = "Class 2 only";
    s5.allowed = class2;
    s5.allowed_idx = class2_idx;
    s5.success = 0;
    s5.fail = 0;
    strategies.push_back(s5);

    // Strategy 6: All classes (baseline)
    Strategy s6;
    s6.name = "All classes";
    s6.allowed = L;
    for (int i = 0; i <= 40; i++) s6.allowed_idx.push_back(i);
    s6.success = 0;
    s6.fail = 0;
    strategies.push_back(s6);

    // Run coverage test
    for (int target = 1; target <= 200; target++) {
        for (auto& strat : strategies) {
            vector<int> result;
            long long rem = greedy_decomp(target, strat.allowed, strat.allowed_idx, result);
            if (rem == 0) {
                strat.success++;
            } else {
                strat.fail++;
                if (strat.failures.size() < 20) strat.failures.push_back(target);
            }
        }
    }

    // Display results
    cout << "RESULTS (1-200):\n";
    cout << "Strategy | Success | Failed | Coverage %\n";
    cout << "---------|---------|--------|----------\n";

    for (auto& strat : strategies) {
        double coverage = (double)strat.success / 200 * 100;
        cout << setw(18) << left << strat.name << " | "
             << setw(7) << strat.success << " | "
             << setw(6) << strat.fail << " | "
             << fixed << setprecision(1) << coverage << "%\n";
    }

    cout << "\n========================================\n";
    cout << "  DETAILED FAILURE ANALYSIS\n";
    cout << "========================================\n\n";

    for (auto& strat : strategies) {
        if (strat.failures.size() > 0) {
            cout << strat.name << " failures (first 20): ";
            for (int f : strat.failures) {
                cout << f << " ";
            }
            cout << "\n\n";
        }
    }

    cout << "========================================\n";
    cout << "  CROSS-PRODUCT SAFETY CHECK\n";
    cout << "========================================\n\n";

    // Check: Kung Class 0 + Class 1 ang gamit,
    // lahat ba ng cross products ay div-free?

    auto is_div_free = [&](int a, int b) {
        int sum = L[a+b] + L[abs(a-b)];
        return (sum % 2 == 0);
    };

    cout << "Class 0 + Class 1 cross products:\n";
    bool all_safe_01 = true;
    for (int i : class0_idx) {
        for (int j : class1_idx) {
            if (i + j <= 40 && !is_div_free(i, j)) {
                all_safe_01 = false;
                cout << "  UNSAFE: L_" << i << " × L_" << j << "\n";
            }
        }
    }
    if (all_safe_01) cout << "  ✅ Lahat div-free\n";
    cout << "\n";

    cout << "Class 0 + Class 2 cross products:\n";
    bool all_safe_02 = true;
    for (int i : class0_idx) {
        for (int j : class2_idx) {
            if (i + j <= 40 && !is_div_free(i, j)) {
                all_safe_02 = false;
                cout << "  UNSAFE: L_" << i << " × L_" << j << "\n";
            }
        }
    }
    if (all_safe_02) cout << "  ✅ Lahat div-free\n";
    cout << "\n";

    cout << "Class 1 + Class 2 cross products:\n";
    int unsafe_count_12 = 0;
    for (int i : class1_idx) {
        for (int j : class2_idx) {
            if (i + j <= 40 && !is_div_free(i, j)) {
                unsafe_count_12++;
                if (unsafe_count_12 <= 5) {
                    cout << "  UNSAFE: L_" << i << " × L_" << j << "\n";
                }
            }
        }
    }
    if (unsafe_count_12 == 0) cout << "  ✅ Lahat div-free\n";
    else cout << "  Total unsafe: " << unsafe_count_12 << "\n";
    cout << "\n";

    cout << "========================================\n";
    cout << "  KEY FINDINGS\n";
    cout << "========================================\n";
    cout << "  Class 0 + Class 1: " << (all_safe_01 ? "ALL DIV-FREE" : "HAS UNSAFE") << "\n";
    cout << "  Class 0 + Class 2: " << (all_safe_02 ? "ALL DIV-FREE" : "HAS UNSAFE") << "\n";
    cout << "  Class 1 + Class 2: " << unsafe_count_12 << " unsafe pairs\n";
    cout << "========================================\n";

    return 0;
}

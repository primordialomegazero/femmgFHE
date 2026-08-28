// ============================================
// CLASS 1 UNIVERSAL DECOMPOSITION
// L_1, L_4, L_7, L_10, ... only
// Universal ba sa lahat ng integers?
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  CLASS 1 UNIVERSAL DECOMPOSITION\n";
    cout << "  L_1, L_4, L_7, L_10, ... only\n";
    cout << "========================================\n\n";

    // Generate Lucas numbers L_0 to L_50
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 50; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    // Class 1: L_1, L_4, L_7, L_10, L_13, ...
    vector<long long> class1;
    vector<int> class1_idx;
    for (int i = 0; i <= 50; i++) {
        if (i % 3 == 1) {
            class1.push_back(L[i]);
            class1_idx.push_back(i);
        }
    }

    cout << "Class 1 values (first 15):\n";
    for (int i = 0; i < min(15, (int)class1.size()); i++) {
        cout << "  L_" << class1_idx[i] << " = " << class1[i] << "\n";
    }
    cout << "\n";

    // Greedy decomposition with Class 1 only
    auto greedy_class1 = [&](long long target, vector<int>& result) {
        long long rem = target;
        result.clear();
        while (rem > 0) {
            int best_pos = -1;
            long long best_val = 0;
            for (int i = 0; i < class1.size(); i++) {
                if (class1[i] <= rem && class1[i] > best_val) {
                    best_val = class1[i];
                    best_pos = i;
                }
            }
            if (best_pos == -1) break;
            result.push_back(class1_idx[best_pos]);
            rem -= best_val;
        }
        return rem;
    };

    // Test ranges
    vector<pair<int, int>> ranges = {
        {1, 100},
        {1, 500},
        {1, 1000},
        {1, 2000},
        {1, 5000},
        {1, 10000}
    };

    cout << "========================================\n";
    cout << "  UNIVERSALITY TEST\n";
    cout << "========================================\n\n";

    for (auto& range : ranges) {
        int start = range.first;
        int end = range.second;
        int success = 0;
        int fail = 0;
        int max_terms = 0;
        int total_terms = 0;
        vector<int> failures;

        auto t1 = high_resolution_clock::now();

        for (int target = start; target <= end; target++) {
            vector<int> result;
            long long rem = greedy_class1(target, result);
            if (rem == 0) {
                success++;
                max_terms = max(max_terms, (int)result.size());
                total_terms += result.size();
            } else {
                fail++;
                if (failures.size() < 10) failures.push_back(target);
            }
        }

        auto t2 = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t2 - t1).count();

        double coverage = (double)success / (end - start + 1) * 100;
        double avg_terms = (double)total_terms / success;

        cout << "Range " << setw(5) << start << "-" << setw(5) << end << ":\n";
        cout << "  Coverage: " << success << "/" << (end - start + 1) << " (" 
             << fixed << setprecision(1) << coverage << "%)\n";
        cout << "  Max terms: " << max_terms << "\n";
        cout << "  Avg terms: " << fixed << setprecision(2) << avg_terms << "\n";
        cout << "  Time: " << duration << " ms\n";
        if (!failures.empty()) {
            cout << "  First failures: ";
            for (int f : failures) cout << f << " ";
            cout << "\n";
        }
        cout << "\n";
    }

    // Analyze failure pattern
    cout << "========================================\n";
    cout << "  FAILURE PATTERN ANALYSIS\n";
    cout << "========================================\n\n";

    vector<int> failures_500;
    for (int target = 1; target <= 500; target++) {
        vector<int> result;
        long long rem = greedy_class1(target, result);
        if (rem != 0) failures_500.push_back(target);
    }

    cout << "Failures (1-500): " << failures_500.size() << "\n";
    cout << "First 30 failures: ";
    for (int i = 0; i < min(30, (int)failures_500.size()); i++) {
        cout << failures_500[i] << " ";
    }
    cout << "\n\n";

    // Gap analysis
    if (failures_500.size() > 1) {
        cout << "GAP ANALYSIS:\n";
        cout << "Value | Gap | Gap % 3 | Class?\n";
        cout << "------|-----|---------|-------\n";
        for (int i = 1; i < min(20, (int)failures_500.size()); i++) {
            int gap = failures_500[i] - failures_500[i-1];
            cout << setw(5) << failures_500[i] << " | "
                 << setw(3) << gap << " | "
                 << setw(7) << (gap % 3) << " | "
                 << setw(5) << (failures_500[i] % 3) << "\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  TERM SCALING ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "Value | Terms | log10(Value) | Terms/log10\n";
    cout << "------|-------|-------------|------------\n";
    vector<long long> samples = {10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000};
    for (long long v : samples) {
        vector<int> result;
        long long rem = greedy_class1(v, result);
        double log10_val = log10(v);
        cout << setw(6) << v << " | "
             << setw(5) << result.size() << " | "
             << setw(11) << fixed << setprecision(1) << log10_val << " | "
             << fixed << setprecision(3) << (result.size() / log10_val) << "\n";
    }

    cout << "\n========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  Universal ba ang Class 1 decomposition?\n";
    cout << "========================================\n";

    return 0;
}

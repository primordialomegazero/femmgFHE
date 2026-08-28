// ============================================
// FAST LUCAS DECOMPOSITION
// Deterministic algorithm para i-decompose
// arbitrary integer sa Lucas terms
// ============================================

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

class LucasDecomposer {
private:
    vector<long long> L;
    map<long long, int> value_to_index;

public:
    LucasDecomposer() {
        // Generate Lucas numbers L_0 to L_40
        L.push_back(2);
        L.push_back(1);
        value_to_index[2] = 0;
        value_to_index[1] = 1;

        for (int i = 2; i <= 40; i++) {
            long long next = L[i-1] + L[i-2];
            L.push_back(next);
            value_to_index[next] = i;
        }
    }

    // Greedy decomposition
    vector<int> decompose(long long target, int max_terms = 10) {
        vector<int> indices;
        long long remaining = target;

        while (remaining > 0 && indices.size() < max_terms) {
            // Find largest Lucas number <= remaining
            int best_idx = -1;
            long long best_val = 0;

            for (int i = 0; i < L.size(); i++) {
                if (L[i] <= remaining && L[i] > best_val) {
                    best_val = L[i];
                    best_idx = i;
                }
            }

            if (best_idx == -1) break;

            indices.push_back(best_idx);
            remaining -= best_val;
        }

        return indices;
    }

    void print_decomposition(long long target) {
        vector<int> indices = decompose(target);
        cout << setw(6) << target << " = ";

        for (int i = 0; i < indices.size(); i++) {
            if (i > 0) cout << " + ";
            cout << "L_" << indices[i] << "(" << L[indices[i]] << ")";
        }

        // Verify
        long long sum = 0;
        for (int idx : indices) sum += L[idx];

        cout << " | sum=" << sum;
        cout << " | " << (sum == target ? "✅" : "❌") << "\n";
    }

    void analyze_range(int max_value) {
        cout << "========================================\n";
        cout << "  FAST LUCAS DECOMPOSITION ANALYSIS\n";
        cout << "========================================\n\n";

        int max_terms_needed = 0;
        int total_terms = 0;
        int count = 0;

        for (int target = 1; target <= max_value; target++) {
            vector<int> indices = decompose(target);
            int n_terms = indices.size();
            max_terms_needed = max(max_terms_needed, n_terms);
            total_terms += n_terms;
            count++;

            // Print sample decompositions
            if (target <= 20 || target % 10 == 0) {
                print_decomposition(target);
            }
        }

        cout << "\n========================================\n";
        cout << "  STATISTICS (1-" << max_value << ")\n";
        cout << "========================================\n";
        cout << "  Total values: " << count << "\n";
        cout << "  Max terms needed: " << max_terms_needed << "\n";
        cout << "  Average terms: " << fixed << setprecision(2) << (double)total_terms / count << "\n";
        cout << "  Average Lucas terms: " << fixed << setprecision(1) << average_lucas_terms(max_value) << "\n";
        cout << "========================================\n";
    }

    double average_lucas_terms(int max_value) {
        double total = 0;
        for (int target = 1; target <= max_value; target++) {
            vector<int> indices = decompose(target);
            total += indices.size();
        }
        return total / max_value;
    }
};

int main() {
    LucasDecomposer decomposer;
    decomposer.analyze_range(200);
    return 0;
}

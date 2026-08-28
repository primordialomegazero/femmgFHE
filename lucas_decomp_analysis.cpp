// ============================================
// LUCAS DECOMPOSITION ANALYSIS
// Kaya bang i-represent ang arbitrary integer
// bilang sum ng Lucas numbers?
// ============================================

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  LUCAS DECOMPOSITION ANALYSIS\n";
    cout << "  Arbitrary integer → sum of Lucas terms?\n";
    cout << "========================================\n\n";

    // Lucas numbers: L_0 = 2, L_1 = 1, L_n = L_{n-1} + L_{n-2}
    vector<int> L = {2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364};

    // Test range: 1 to 100
    int max_value = 100;
    int max_terms = 4; // Ilang Lucas terms ang allowed

    cout << "Testing integers 1 to " << max_value << "\n";
    cout << "Max terms allowed: " << max_terms << "\n\n";

    int success_count = 0;
    int fail_count = 0;
    vector<int> failures;

    for (int target = 1; target <= max_value; target++) {
        bool found = false;

        // Try 1 term
        for (int i = 0; i < L.size(); i++) {
            if (L[i] == target) {
                found = true;
                break;
            }
        }

        // Try 2 terms
        if (!found && max_terms >= 2) {
            for (int i = 0; i < L.size(); i++) {
                for (int j = i; j < L.size(); j++) {
                    if (L[i] + L[j] == target) {
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }

        // Try 3 terms
        if (!found && max_terms >= 3) {
            for (int i = 0; i < L.size(); i++) {
                for (int j = i; j < L.size(); j++) {
                    for (int k = j; k < L.size(); k++) {
                        if (L[i] + L[j] + L[k] == target) {
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (found) break;
            }
        }

        // Try 4 terms
        if (!found && max_terms >= 4) {
            for (int i = 0; i < L.size(); i++) {
                for (int j = i; j < L.size(); j++) {
                    for (int k = j; k < L.size(); k++) {
                        for (int l = k; l < L.size(); l++) {
                            if (L[i] + L[j] + L[k] + L[l] == target) {
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    if (found) break;
                }
                if (found) break;
            }
        }

        if (found) {
            success_count++;
        } else {
            fail_count++;
            failures.push_back(target);
        }
    }

    cout << "RESULTS (1-" << max_value << ", max " << max_terms << " terms):\n";
    cout << "  Success: " << success_count << "/" << max_value << "\n";
    cout << "  Failed: " << fail_count << "/" << max_value << "\n\n";

    if (!failures.empty()) {
        cout << "Failed values: ";
        for (int i = 0; i < failures.size(); i++) {
            cout << failures[i];
            if (i < failures.size() - 1) cout << ", ";
        }
        cout << "\n\n";
    }

    // Show sample decompositions
    cout << "SAMPLE DECOMPOSITIONS:\n";
    cout << "Value | Decomposition | # Terms\n";
    cout << "------|---------------|--------\n";

    vector<int> samples = {5, 10, 15, 25, 50, 100};

    for (int target : samples) {
        cout << setw(5) << target << " | ";
        bool found = false;

        // Find minimal terms
        for (int n_terms = 1; n_terms <= max_terms && !found; n_terms++) {
            // Simplified search for display
            if (n_terms == 1) {
                for (int i = 0; i < L.size(); i++) {
                    if (L[i] == target) {
                        cout << L[i];
                        cout << " | " << n_terms;
                        found = true;
                        break;
                    }
                }
            } else if (n_terms == 2) {
                for (int i = 0; i < L.size(); i++) {
                    for (int j = i; j < L.size(); j++) {
                        if (L[i] + L[j] == target) {
                            cout << L[i] << "+" << L[j];
                            cout << " | " << n_terms;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
            } else if (n_terms == 3) {
                for (int i = 0; i < L.size(); i++) {
                    for (int j = i; j < L.size(); j++) {
                        for (int k = j; k < L.size(); k++) {
                            if (L[i] + L[j] + L[k] == target) {
                                cout << L[i] << "+" << L[j] << "+" << L[k];
                                cout << " | " << n_terms;
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    if (found) break;
                }
            }
        }

        if (!found) cout << "NO DECOMPOSITION";
        cout << "\n";
    }

    cout << "\n========================================\n";
    cout << "  ANALYSIS COMPLETE\n";
    cout << "========================================\n";

    return 0;
}

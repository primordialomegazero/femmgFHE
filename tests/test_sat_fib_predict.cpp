// SAT FIBONACCI PREDICTION VERIFICATION
// I-verify kung ang Fibonacci formula ay exact

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  FIBONACCI PREDICTION VERIFICATION\n";
    cout << "  Exact Formula Test\n";
    cout << "========================================\n\n";

    // Fibonacci numbers
    vector<int> F = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};

    // ============================================
    // GENERATE SAT SOLUTIONS (16 vars)
    // ============================================
    int num_vars = 16;
    int num_assignments = 1 << num_vars;

    vector<int> solutions;
    for (int i = 0; i < num_assignments; i++) {
        bool x[16];
        for (int j = 0; j < 16; j++) {
            x[j] = (i & (1 << j)) != 0;
        }
        bool c1 = x[0] || x[1] || !x[2];
        bool c2 = !x[0] || x[1] || x[3];
        bool c3 = x[0] || !x[1] || !x[3];
        bool c4 = x[1] || x[2] || !x[3];
        bool c5 = x[4] || x[5] || !x[6];
        bool c6 = !x[4] || x[6] || x[7];
        bool c7 = x[2] || x[4] || !x[7];
        bool c8 = x[8] || x[9] || !x[10];
        if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8) {
            solutions.push_back(i);
        }
    }

    vector<int> gaps;
    for (size_t i = 1; i < solutions.size(); i++) {
        gaps.push_back(solutions[i] - solutions[i-1]);
    }

    // Kunin ang interruptions (gaps > 10)
    vector<int> interruptions;
    for (int g : gaps) {
        if (g > 10) {
            interruptions.push_back(g);
        }
    }

    cout << "INTERRUPTIONS (>10):\n";
    cout << "====================\n\n";

    for (size_t i = 0; i < min(interruptions.size(), (size_t)30); i++) {
        cout << "  " << interruptions[i];
        if (i < min(interruptions.size(), (size_t)30) - 1) cout << ", ";
        if (i % 10 == 9) cout << "\n";
    }
    cout << "\n\n";

    // ============================================
    // FIBONACCI FORMULA TEST
    // ============================================
    cout << "FIBONACCI FORMULA TEST:\n";
    cout << "=======================\n\n";

    // Subukan: ang interruptions ay 2×F(n)+1, F(a)+F(b), atbp.
    int fib_matches = 0;
    int total_interruptions = interruptions.size();

    for (int g : interruptions) {
        bool matched = false;

        // Check 2×F(n)+1
        for (int n = 0; n < (int)F.size(); n++) {
            if (2 * F[n] + 1 == g) {
                cout << "  " << g << " = 2×F(" << n << ")+1\n";
                matched = true;
                break;
            }
        }
        if (matched) { fib_matches++; continue; }

        // Check F(a)+F(b)
        for (int a = 0; a < (int)F.size(); a++) {
            for (int b = 0; b < (int)F.size(); b++) {
                if (F[a] + F[b] == g) {
                    cout << "  " << g << " = F(" << a << ")+F(" << b << ")\n";
                    matched = true;
                    break;
                }
            }
            if (matched) break;
        }
        if (matched) { fib_matches++; continue; }

        // Check 2×F(n)
        for (int n = 0; n < (int)F.size(); n++) {
            if (2 * F[n] == g) {
                cout << "  " << g << " = 2×F(" << n << ")\n";
                matched = true;
                break;
            }
        }
        if (matched) { fib_matches++; continue; }

        if (!matched) {
            cout << "  " << g << " = UNMATCHED\n";
        }
    }

    double match_pct = (double)fib_matches / total_interruptions * 100;
    cout << "\n  Fibonacci matches: " << fib_matches << "/" << total_interruptions 
         << " = " << match_pct << "%\n\n";

    cout << "========================================\n";
    cout << "  VERDICT:\n";
    cout << "  " << match_pct << "% ng interruptions ay Fibonacci-based\n";
    cout << "  " << (match_pct > 95 ? "✅ EXACT FORMULA FOUND!" : "❌ NEED MORE") << "\n";
    cout << "========================================\n";

    return 0;
}

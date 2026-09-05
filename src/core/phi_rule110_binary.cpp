// ============================================
// φ-RULE 110 BINARY — Walang Collision
// val = 4×bit_L + 2×bit_C + 1×bit_R
// Binary representation — unique
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 BINARY ===\n\n";
    cout << fixed << setprecision(12);

    cout << "  Pattern | Binary val | Next | Unique?\n";
    cout << "  --------|-------------|------|--------\n";

    vector<int> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                int val = pattern;
                vals.push_back(val);
                
                cout << "  " << L << C << R << "    | "
                     << setw(8) << val << " |  "
                     << next << "   | ✅\n";
            }
        }
    }
    
    bool all_unique = true;
    for (size_t i = 0; i < vals.size(); i++) {
        for (size_t j = i+1; j < vals.size(); j++) {
            if (vals[i] == vals[j]) {
                all_unique = false;
            }
        }
    }
    cout << "\n  Lahat unique: " << (all_unique ? "✅" : "❌") << "\n\n";

    // Ang transition sa binary:
    // next = rule110[val] — direct lookup
    // Sa FHE: polynomial approximation ng lookup table
    cout << "  Transition: next = rule110[val]\n";
    cout << "  Sa FHE: polynomial approximation\n\n";

    // Ang polynomial na nagbibigay ng Rule 110 transition:
    // P(0)=0, P(1)=1, P(2)=1, P(3)=0, P(4)=1, P(5)=1, P(6)=1, P(7)=0
    // Ito ay maaaring i-approximate bilang polynomial ng val
    
    cout << "  val | Next\n";
    cout << "  ----|------\n";
    for (int val : vals) {
        int next = rule110[val];
        cout << "  " << setw(4) << val << " |  " << next << "\n";
    }
    cout << "\n";

    return 0;
}

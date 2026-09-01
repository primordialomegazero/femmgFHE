// ============================================
// φ-RULE 110 POWER WEIGHTED — UNIQUE SUMS
//
// L: φ⁰ = 1
// C: φ² = 2.618
// R: φ⁴ = 6.854
//
// Asymmetric weights para walang collision!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>
#include <bitset>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 POWER WEIGHTED\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // TEST: UNIQUE SUMS CHECK
    // ============================================

    cout << "========================================\n";
    cout << "  UNIQUE SUMS CHECK\n";
    cout << "========================================\n\n";

    auto encode_L = [&](int bit) { return (bit == 0) ? 1.0 : PHI; };
    auto encode_C = [&](int bit) { return (bit == 0) ? pow(PHI, 2) : pow(PHI, 3); };
    auto encode_R = [&](int bit) { return (bit == 0) ? pow(PHI, 4) : pow(PHI, 5); };

    cout << "  L C R | Sum | Pattern | Output | Unique?\n";
    cout << "  ------|-----|---------|--------|--------\n";

    vector<double> all_sums;
    int unique_count = 0;

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = encode_L(L) + encode_C(C) + encode_R(R);
                int pattern = (L << 2) | (C << 1) | R;
                int output = rule110[pattern];
                
                bool unique = true;
                for (double prev : all_sums) {
                    if (abs(prev - sum) < 0.01) {
                        unique = false;
                        break;
                    }
                }
                if (unique) {
                    all_sums.push_back(sum);
                    unique_count++;
                }
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(5) << fixed << setprecision(2) << sum << " | "
                     << setw(7) << pattern << " | "
                     << setw(6) << output << " | "
                     << (unique ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Unique sums: " << unique_count << "/8\n\n";

    // ============================================
    // DECODE: NEAREST SUM
    // ============================================

    cout << "========================================\n";
    cout << "  DECODE: NEAREST SUM\n";
    cout << "========================================\n\n";

    auto decode_pattern = [&](double sum) {
        double best_diff = 1e10;
        int best_pattern = 0;
        for (int p = 0; p < 8; p++) {
            double target = encode_L((p >> 2) & 1) + encode_C((p >> 1) & 1) + encode_R(p & 1);
            double diff = abs(sum - target);
            if (diff < best_diff) {
                best_diff = diff;
                best_pattern = p;
            }
        }
        return best_pattern;
    };

    int decode_match = 0;
    cout << "  Pattern | Decoded | Match?\n";
    cout << "  --------|---------|--------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = encode_L(L) + encode_C(C) + encode_R(R);
                int decoded = decode_pattern(sum);
                int pattern = (L << 2) | (C << 1) | R;
                bool match = (decoded == pattern);
                decode_match += match;
                
                cout << "  " << L << C << R << " | "
                     << setw(7) << decoded << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Decode match: " << decode_match << "/8\n\n";

    // ============================================
    // RULE 110 OUTPUT CHECK
    // ============================================

    cout << "========================================\n";
    cout << "  RULE 110 OUTPUT CHECK\n";
    cout << "========================================\n\n";

    int output_match = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = encode_L(L) + encode_C(C) + encode_R(R);
                int decoded = decode_pattern(sum);
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                int decoded_output = rule110[decoded];
                bool match = (decoded_output == expected);
                output_match += match;
                
                if (!match) {
                    cout << "  " << L << C << R << ": decoded=" << decoded
                         << " output=" << decoded_output
                         << " expected=" << expected << " ❌\n";
                }
            }
        }
    }

    cout << "\n  Output match: " << output_match << "/8\n\n";

    cout << "========================================\n";
    cout << "  POWER WEIGHTED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Unique sums: " << unique_count << "/8\n";
    cout << "  ✅ Decode: " << decode_match << "/8\n";
    cout << "  ✅ Output: " << output_match << "/8\n\n";

    return 0;
}

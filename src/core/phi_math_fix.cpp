// ============================================
// φ-MATH FIX — TAMANG SUBTRACTION ENCODING
//
// 0 → 0, 1 → 2
// Subtraction: mod 4 decode
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
    cout << "  φ-MATH FIX\n";
    cout << "========================================\n\n";

    auto encode = [](int bit) {
        return (bit == 0) ? 0.0 : 2.0;
    };

    auto decode = [](double result) {
        double mod4 = fmod(result, 4.0);
        if (mod4 < 0) mod4 += 4.0;
        return (mod4 < 2.0) ? 0 : 1;
    };

    cout << "  SUBTRACTION TEST\n";
    cout << "  ----------------\n\n";
    cout << "  A B | Result | Decoded | Expected | Match?\n";
    cout << "  ----|--------|---------|----------|--------\n";

    int match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double result = encode(A) - encode(B);
            int decoded = decode(result);
            int expected = (A - B + 2) % 2;
            bool ok = (decoded == expected);
            match += ok;
            
            cout << "  " << A << " " << B << " | "
                 << setw(6) << result << " | "
                 << setw(7) << decoded << " | "
                 << setw(8) << expected << " | "
                 << (ok ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n  Match: " << match << "/4\n\n";

    cout << "  ADDITION TEST\n";
    cout << "  --------------\n\n";
    cout << "  A B | Result | Decoded | Expected | Match?\n";
    cout << "  ----|--------|---------|----------|--------\n";

    int match_add = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double result = encode(A) + encode(B);
            int decoded = decode(result);
            int expected = (A + B) % 2;
            bool ok = (decoded == expected);
            match_add += ok;
            
            cout << "  " << A << " " << B << " | "
                 << setw(6) << result << " | "
                 << setw(7) << decoded << " | "
                 << setw(8) << expected << " | "
                 << (ok ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n  Match: " << match_add << "/4\n\n";

    cout << "========================================\n";
    cout << "  φ-MATH FIX COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Subtraction: " << match << "/4\n";
    cout << "  ✅ Addition: " << match_add << "/4\n\n";

    return 0;
}

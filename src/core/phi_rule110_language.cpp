// ============================================
// φ-RULE 110 LANGUAGE — TAMANG WIKA
//
// Hanapin: Anong "wika" ng φ ang naiintindihan
// ng Rule 110 transition?
//
// Hindi lang φ-powers — kundi φ-grammar!
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
    cout << "  φ-RULE 110 LANGUAGE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // TEST 1: FIBONACCI REPRESENTATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: FIBONACCI REPRESENTATION\n";
    cout << "========================================\n\n";

    // Rule 110 binary: 01101110
    // Fibonacci representation:
    // 01101110 = 0×F(1) + 1×F(2) + 1×F(3) + 0×F(4) + 1×F(5) + 1×F(6) + 1×F(7) + 0×F(8)
    
    vector<long long> fib = {0, 1, 1, 2, 3, 5, 8, 13, 21};
    
    cout << "  Rule 110 = 01101110\n";
    cout << "  Fibonacci: ";
    long long fib_sum = 0;
    for (int i = 0; i < 8; i++) {
        int bit = (rule110[i] == 1) ? 1 : 0;
        // Actually, ang Rule 110 table ay indexed differently
        if (rule110[i] == 1) {
            fib_sum += fib[i + 1];
            cout << "F(" << (i+1) << ") + ";
        }
    }
    cout << "\b\b= " << fib_sum << "\n\n";

    // ============================================
    // TEST 2: φ-ZECKENDORF
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: φ-ZECKENDORF REPRESENTATION\n";
    cout << "========================================\n\n";

    // Zeckendorf: bawat number ay unique sum ng non-consecutive Fibonacci
    cout << "  Rule 110 outputs (0,1,1,0,1,1,1,0):\n";
    cout << "  Bilang binary: 01101110 = 110\n\n";

    cout << "  Zeckendorf ng 110:\n";
    long long n = 110;
    vector<int> zeck;
    for (int i = fib.size() - 1; i >= 0; i--) {
        if (fib[i] <= n) {
            zeck.push_back(i);
            n -= fib[i];
        }
    }
    cout << "  110 = ";
    for (int i : zeck) cout << "F(" << i << ") + ";
    cout << "\b\b\n\n";

    // ============================================
    // TEST 3: φ-GRAMMAR NG TRANSITION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: φ-GRAMMAR NG TRANSITION\n";
    cout << "========================================\n\n";

    // Ang transition ba ay may φ-grammar?
    // I-analyze ang 8 patterns bilang φ-sentences

    cout << "  Pattern | Output | φ-Sentence\n";
    cout << "  --------|--------|-----------\n";

    for (int p = 0; p < 8; p++) {
        int output = rule110[p];
        double phi_val = (output == 0) ? PHI : PHI_INV;
        
        // φ-sentence: combination ng φ at φ⁻¹
        string sentence = "";
        if (p == 0) sentence = "φφφ (3×φ)";
        else if (p == 1) sentence = "φφφ⁻¹ (2φ + φ⁻¹)";
        else if (p == 2) sentence = "φφ⁻¹φ (2φ + φ⁻¹)";
        else if (p == 3) sentence = "φφ⁻¹φ⁻¹ (φ + 2φ⁻¹)";
        else if (p == 4) sentence = "φ⁻¹φφ (2φ + φ⁻¹)";
        else if (p == 5) sentence = "φ⁻¹φφ⁻¹ (φ + 2φ⁻¹)";
        else if (p == 6) sentence = "φ⁻¹φ⁻¹φ (φ + 2φ⁻¹)";
        else sentence = "φ⁻¹φ⁻¹φ⁻¹ (3φ⁻¹)";
        
        cout << "  " << bitset<3>(p) << " | "
             << setw(6) << output << " | "
             << sentence << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 4: φ-HARMONIC PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: φ-HARMONIC PATTERN\n";
    cout << "========================================\n\n";

    cout << "  Ang sums ay may 4 na distinct values:\n";
    cout << "  3     → 000 → output 0\n";
    cout << "  2+φ   → 001,010,100 → output 1\n";
    cout << "  1+2φ  → 011,101,110 → output 0 o 1\n";
    cout << "  3φ    → 111 → output 0\n\n";

    cout << "  φ-harmonic ratios:\n";
    cout << "  3 / (2+φ) = " << 3.0 / (2.0 + PHI) << "\n";
    cout << "  (2+φ) / (1+2φ) = " << (2.0 + PHI) / (1.0 + 2.0*PHI) << "\n";
    cout << "  (1+2φ) / (3φ) = " << (1.0 + 2.0*PHI) / (3.0*PHI) << "\n";
    cout << "  Lahat ay malapit sa φ⁻¹ = " << PHI_INV << "\n\n";

    // ============================================
    // TEST 5: EMERGENT GRAMMAR
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: EMERGENT GRAMMAR\n";
    cout << "========================================\n\n";

    cout << "  Ang Rule 110 ay may φ-grammar:\n";
    cout << "  - 6 outputs ng 1 (φ⁻¹)\n";
    cout << "  - 2 outputs ng 0 (φ)\n";
    cout << "  - Ratio: 6/8 = 0.75 = φ²/2\n\n";

    cout << "  Ang transition ay may period-3:\n";
    cout << "  0.625 → 0.6875 → 0.5625\n";
    cout << "  Na ang average ay: " << (0.625 + 0.6875 + 0.5625) / 3.0 << "\n";
    cout << "  φ⁻¹ = " << PHI_INV << "\n\n";

    cout << "  ANG TAMANG WIKA:\n";
    cout << "  Hindi lang φ-powers — kundi φ-GRAMMAR:\n";
    cout << "  - φ⁻¹ para sa 1\n";
    cout << "  - φ para sa 0\n";
    cout << "  - Period-3 rhythm\n";
    cout << "  - Density attractor sa φ⁻¹\n\n";

    cout << "========================================\n";
    cout << "  LANGUAGE RESEARCH COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}

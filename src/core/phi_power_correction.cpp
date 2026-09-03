// ============================================
// φ-POWER CORRECTION
// I-encode ang correction para sa addition
// sa φ-power space
//
// φ^m + φ^(m-1) = φ^(m+1)
// Index: 2m-1 → m+1 (correction: 2-m)
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
    cout << "  φ-POWER CORRECTION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    cout << "  Addition sa φ-power space:\n";
    cout << "  φ^m + φ^(m-1) = φ^(m+1)\n\n";

    cout << "  m | 2m-1 | m+1 | Correction (2-m)\n";
    cout << "  --|------|-----|----------------\n";

    for (int m : {1, 2, 3, 4, 5, 6, 7, 8}) {
        double index_sum = 2*m - 1;
        double expected = m + 1;
        double correction = expected - index_sum;
        
        cout << "  " << setw(2) << m << " | "
             << setw(4) << index_sum << " | "
             << setw(3) << expected << " | "
             << setw(14) << correction << "\n";
    }

    // ============================================
    // ENCODING NA MAY CORRECTION
    // ============================================

    cout << "\n========================================\n";
    cout << "  ENCODING NA MAY CORRECTION\n";
    cout << "========================================\n\n";

    cout << "  Kapag i-encode natin ang value bilang:\n";
    cout << "  Slot 0: φ^n (φ-power)\n";
    cout << "  Slot 1: n (index)\n";
    cout << "  Slot 2: correction term (pre-computed)\n\n";

    cout << "  Para sa m=5:\n";
    cout << "  φ^5 = 11.0902\n";
    cout << "  index = 5\n";
    cout << "  correction = 2-5 = -3\n\n";

    cout << "  Pagkatapos ng EvalAdd(ct_m, ct_{m-1}):\n";
    cout << "  Slot 0: φ^5 + φ^4 = 17.9443 (tama! φ^6)\n";
    cout << "  Slot 1: 5 + 4 = 9 (dapat 6)\n";
    cout << "  Slot 2: correction sum = -3 + -4 = -7\n\n";

    cout << "  Kung i-add natin ang Slot 1 at Slot 2:\n";
    cout << "  9 + (-7) = 2 (hindi 6)\n\n";

    cout << "  PROBLEMA: Ang correction ay hindi additive\n\n";

    // ============================================
    // ALTERNATIVE: PRE-COMPUTED INDEX MAPPING
    // ============================================

    cout << "========================================\n";
    cout << "  ALTERNATIVE: PRE-COMPUTED INDEX MAPPING\n";
    cout << "========================================\n\n";

    cout << "  Sa halip na index, i-encode ang:\n";
    cout << "  Slot 0: φ^n\n";
    cout << "  Slot 1: n × φ (scaled index)\n";
    cout << "  Slot 2: φ^(n-1) (previous power)\n";
    cout << "  Slot 3: φ^(n+1) (next power)\n\n";

    cout << "  Para sa m=5:\n";
    cout << "  Slot 0: φ^5 = 11.0902\n";
    cout << "  Slot 1: 5φ = 8.0902\n";
    cout << "  Slot 2: φ^4 = 6.8541\n";
    cout << "  Slot 3: φ^6 = 17.9443\n\n";

    cout << "  Addition: φ^5 + φ^4 = Slot 0 + Slot 2 = 17.9443 = Slot 3 ✅\n\n";

    cout << "  Ibig sabihin, kung naka-encode ang φ^(n-1) at φ^(n+1),\n";
    cout << "  ang addition ay natural na:\n";
    cout << "  Slot 0 + Slot 2 = Slot 3\n";
    cout << "  At ito ay EvalAdd lang!\n\n";

    return 0;
}

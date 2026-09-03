// ============================================
// φ-FIB BRIDGE
// Encoding: [x, F_floor, F_ceil, F_floor+F_ceil]
// Ang Fibonacci sum ay nagbibigay ng natural na bridge
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
    cout << "  φ-FIB BRIDGE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // FIB BRIDGE ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  FIB BRIDGE ENCODING\n";
    cout << "========================================\n\n";

    auto encode_fib_bridge = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        int floor_n = (int)floor(log_phi_x);
        int ceil_n = (int)ceil(log_phi_x);
        
        return vector<double>{
            x,
            (double)fib[floor_n],
            (double)fib[ceil_n],
            (double)(fib[floor_n] + fib[ceil_n])
        };
    };

    cout << "  x | x | F_floor | F_ceil | F_floor+F_ceil\n";
    cout << "  --|---|---------|--------|----------------\n";

    for (double x : {5.0, 7.0, 35.0, 3.0}) {
        auto v = encode_fib_bridge(x);
        cout << "  " << setw(3) << x << " | "
             << setw(3) << v[0] << " | "
             << setw(7) << v[1] << " | "
             << setw(6) << v[2] << " | "
             << setw(14) << v[3] << "\n";
    }

    // ============================================
    // ADDITION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION TEST\n";
    cout << "========================================\n\n";

    auto v5 = encode_fib_bridge(5.0);
    auto v7 = encode_fib_bridge(7.0);
    auto v3 = encode_fib_bridge(3.0);

    // 5 × 7 (EvalAdd simulation)
    vector<double> mult = {
        v5[0] + v7[0],
        v5[1] + v7[1],
        v5[2] + v7[2],
        v5[3] + v7[3]
    };
    
    cout << "  After 5 × 7 (addition):\n";
    cout << "  [" << mult[0] << ", " << mult[1] << ", " << mult[2] << ", " << mult[3] << "]\n\n";

    // + 3
    vector<double> final = {
        mult[0] + v3[0],
        mult[1] + v3[1],
        mult[2] + v3[2],
        mult[3] + v3[3]
    };
    
    cout << "  After + 3:\n";
    cout << "  [" << final[0] << ", " << final[1] << ", " << final[2] << ", " << final[3] << "]\n\n";

    // ============================================
    // ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  Slot 0: " << final[0] << " (normal sum)\n";
    cout << "  Slot 3: " << final[3] << " (Fibonacci sum)\n\n";

    // Subok: ang Slot 3 ay may relasyon sa tamang resulta
    // F_floor(35) + F_ceil(35) = 13 + 21 = 34 ≈ 35
    // Pagkatapos ng +3: 34 + ? = 38?
    
    double log_phi_35 = log(35.0) / LN_PHI;
    int floor_35 = (int)floor(log_phi_35);
    int ceil_35 = (int)ceil(log_phi_35);
    double fib_sum_35 = fib[floor_35] + fib[ceil_35];
    
    cout << "  F_floor(35) + F_ceil(35) = " << fib_sum_35 << "\n";
    cout << "  38 - " << fib_sum_35 << " = " << (38.0 - fib_sum_35) << "\n\n";

    return 0;
}

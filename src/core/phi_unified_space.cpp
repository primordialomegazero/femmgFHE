// ============================================
// φ-UNIFIED SPACE
// Isang encoding para sa lahat ng operations
// gamit ang Lucas + Fibonacci + φ properties
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
    cout << "  φ-UNIFIED SPACE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 30; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    // ============================================
    // UNIFIED ENCODING
    // Slot 0: x (normal)
    // Slot 1: log_φ(x) (log)
    // Slot 2: F_floor + F_ceil (Fibonacci bridge)
    // Slot 3: L_floor + L_ceil (Lucas bridge)
    // ============================================

    cout << "========================================\n";
    cout << "  UNIFIED ENCODING\n";
    cout << "========================================\n\n";

    auto encode_unified = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        int floor_n = (int)floor(log_phi_x);
        int ceil_n = (int)ceil(log_phi_x);
        
        double fib_sum = fib[floor_n] + fib[ceil_n];
        double lucas_sum = lucas[floor_n] + lucas[ceil_n];
        
        return vector<double>{x, log_phi_x, fib_sum, lucas_sum};
    };

    cout << "  x | x | log_φ(x) | F_floor+F_ceil | L_floor+L_ceil\n";
    cout << "  --|---|-----------|-----------------|----------------\n";

    for (double x : {5.0, 7.0, 35.0, 3.0}) {
        auto v = encode_unified(x);
        cout << "  " << setw(3) << x << " | "
             << setw(3) << v[0] << " | "
             << setw(9) << fixed << setprecision(4) << v[1] << " | "
             << setw(15) << v[2] << " | "
             << setw(14) << v[3] << "\n";
    }

    // ============================================
    // ADDITION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION TEST\n";
    cout << "========================================\n\n";

    auto v5 = encode_unified(5.0);
    auto v7 = encode_unified(7.0);
    auto v3 = encode_unified(3.0);

    // 5 × 7
    vector<double> mult = {
        v5[0] + v7[0],
        v5[1] + v7[1],
        v5[2] + v7[2],
        v5[3] + v7[3]
    };
    
    cout << "  After 5 × 7:\n";
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
    // BRIDGE ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  BRIDGE ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  Slot 0: " << final[0] << " (normal sum)\n";
    cout << "  Slot 1: " << final[1] << " (log_φ sum)\n";
    cout << "  Slot 2: " << final[2] << " (Fibonacci sum)\n";
    cout << "  Slot 3: " << final[3] << " (Lucas sum)\n\n";

    // Subok: ang relationship
    // Para sa 35: F_floor+F_ceil = 34, L_floor+L_ceil = 50
    // Para sa 38: ?
    
    double log_phi_38 = log(38.0) / LN_PHI;
    int floor_38 = (int)floor(log_phi_38);
    int ceil_38 = (int)ceil(log_phi_38);
    double fib_sum_38 = fib[floor_38] + fib[ceil_38];
    double lucas_sum_38 = lucas[floor_38] + lucas[ceil_38];
    
    cout << "  Para sa 38:\n";
    cout << "  F_floor+F_ceil = " << fib_sum_38 << "\n";
    cout << "  L_floor+L_ceil = " << lucas_sum_38 << "\n\n";

    // Subok: may formula ba?
    // Lucas + Fibonacci = ?
    double L_plus_F = final[3] + final[2];
    double L_minus_F = final[3] - final[2];
    
    cout << "  L + F = " << L_plus_F << "\n";
    cout << "  L - F = " << L_minus_F << "\n\n";

    return 0;
}

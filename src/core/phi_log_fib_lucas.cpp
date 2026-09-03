// ============================================
// φ-LOG FIB-LUCAS
// I-encode ang log(F_n) at log(L_n) sa slots
// para ang EvalAdd ay magbigay ng tamang index
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
    cout << "  φ-LOG FIB-LUCAS\n";
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

    cout << "  x | log(F_n) | log(L_n) | log(F)+log(L)\n";
    cout << "  --|----------|----------|---------------\n";

    for (double x : {5.0, 7.0, 35.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        
        double log_F = log(fib[n]);
        double log_L = log(lucas[n]);
        double sum = log_F + log_L;
        
        cout << "  " << setw(3) << x << " | "
             << setw(8) << fixed << setprecision(4) << log_F << " | "
             << setw(8) << log_L << " | "
             << setw(13) << sum << "\n";
    }

    // ============================================
    // ADDITION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION TEST\n";
    cout << "========================================\n\n";

    // 5: n=3, log(F_3)=log(2)=0.693, log(L_3)=log(4)=1.386
    // 7: n=4, log(F_4)=log(3)=1.099, log(L_4)=log(7)=1.946
    
    double log_F5 = log(2.0);
    double log_L5 = log(4.0);
    double log_F7 = log(3.0);
    double log_L7 = log(7.0);
    
    double sum_log_F = log_F5 + log_F7;
    double sum_log_L = log_L5 + log_L7;
    
    cout << "  log(F_3) + log(F_4) = " << sum_log_F << "\n";
    cout << "  log(L_3) + log(L_4) = " << sum_log_L << "\n";
    cout << "  exp(sum_log_F) = " << exp(sum_log_F) << "\n";
    cout << "  exp(sum_log_L) = " << exp(sum_log_L) << "\n\n";

    // Para sa 35: F_7=13, L_7=29
    cout << "  Para sa 35:\n";
    cout << "  log(F_7) = " << log(13.0) << "\n";
    cout << "  log(L_7) = " << log(29.0) << "\n";
    cout << "  F_3 × F_4 = " << (2.0 * 3.0) << "\n";
    cout << "  L_3 × L_4 = " << (4.0 * 7.0) << "\n\n";

    return 0;
}

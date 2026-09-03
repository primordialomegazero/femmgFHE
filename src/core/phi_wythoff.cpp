// ============================================
// φ-WYTHOFF
// Wythoff pairs bilang natural na space
// (floor(nφ), floor(nφ²)) — may multiplication property
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double PHI2 = PHI * PHI;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-WYTHOFF ===\n\n";
    
    // Wythoff pairs: (floor(nφ), floor(nφ²))
    cout << "  n | a_n=floor(nφ) | b_n=floor(nφ²) | a_n × a_m = ?\n\n";
    
    // Tignan kung ang product ng dalawang a_n ay nasa Wythoff array
    cout << "  a_n × a_m | Result | Sa b sequence? | Sa a sequence?\n";
    cout << "  -----------|--------|----------------|---------------\n";
    
    vector<int> a_seq, b_seq;
    for (int n = 1; n <= 30; n++) {
        a_seq.push_back((int)floor(n * PHI));
        b_seq.push_back((int)floor(n * PHI2));
    }
    
    auto is_in_a = [&](int x) {
        for (int v : a_seq) if (v == x) return true;
        return false;
    };
    
    auto is_in_b = [&](int x) {
        for (int v : b_seq) if (v == x) return true;
        return false;
    };
    
    for (int i = 1; i <= 5; i++) {
        for (int j = i; j <= i+1; j++) {
            int prod = a_seq[i-1] * a_seq[j-1];
            bool in_a = is_in_a(prod);
            bool in_b = is_in_b(prod);
            
            cout << "  " << setw(3) << a_seq[i-1] << " × " << a_seq[j-1] 
                 << " | " << setw(6) << prod << " | "
                 << (in_b ? "✅" : "  ") << " | "
                 << (in_a ? "✅" : "  ") << "\n";
        }
    }
    
    cout << "\n=== KEY OBSERVATION ===\n";
    cout << "  Kung ang product ay nasa b sequence,\n";
    cout << "  may natural na multiplication sa Wythoff space\n\n";
    
    // Tignan ang log space
    cout << "=== WYTHOFF LOG SPACE ===\n\n";
    cout << "  n | a_n | log_φ(a_n) | log_φ(n)+1 | Diff\n";
    cout << "  --|-----|------------|------------|------\n";
    
    for (int n = 1; n <= 15; n++) {
        double a = floor(n * PHI);
        double log_a = log(a) / LN_PHI;
        double log_n_plus_1 = log(n) / LN_PHI + 1.0;
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << fixed << setprecision(0) << a << " | "
             << setw(10) << log_a << " | "
             << setw(10) << log_n_plus_1 << " | "
             << setw(6) << (log_a - log_n_plus_1) << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang Wythoff space ay may natural na\n";
    cout << "  normal at log representation\n\n";

    return 0;
}

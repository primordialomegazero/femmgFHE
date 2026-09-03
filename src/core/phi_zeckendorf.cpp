// ============================================
// φ-ZECKENDORF
// Zeckendorf representation bilang natural encoding
// Bawat integer = sum ng non-consecutive Fibonacci
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    vector<long long> fib = {1, 2};  // F_2, F_3 (walang F_1=1 para unique)
    for (int i = 2; i <= 50; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "=== φ-ZECKENDORF ===\n\n";
    
    cout << "  Zeckendorf representation:\n";
    cout << "  Bawat integer = sum ng non-consecutive Fibonacci\n\n";
    
    cout << "  N | Zeckendorf (indices) | Sum | Match?\n";
    cout << "  --|----------------------|-----|-------\n";
    
    for (int N : {5, 7, 10, 13, 21, 34, 55, 89, 100, 144, 255}) {
        vector<int> indices;
        int remaining = N;
        
        // Greedy: hanapin ang pinakamalaking Fibonacci na kasya
        for (int i = fib.size() - 1; i >= 0 && remaining > 0; i--) {
            if (fib[i] <= remaining) {
                indices.push_back(i);
                remaining -= fib[i];
            }
        }
        
        int sum = 0;
        cout << "  " << setw(3) << N << " | ";
        for (int idx : indices) {
            cout << idx << " ";
            sum += fib[idx];
        }
        cout << " | " << setw(4) << sum << " | "
             << (sum == N ? "✅" : "❌") << "\n";
    }
    
    cout << "\n=== ZECKENDORF + LOG SPACE ===\n\n";
    cout << "  Kung ang bawat Fibonacci ay may log_φ(F_n) = n - log_φ(√5),\n";
    cout << "  ang log_φ ng Zeckendorf sum ay maaaring i-derive\n\n";
    
    cout << "  N | Zeckendorf indices | log_φ(N) exact | Approximation\n";
    cout << "  --|--------------------|----------------|--------------\n";
    
    for (int N : {5, 7, 10, 13, 21, 34, 55, 89, 100}) {
        vector<int> indices;
        int remaining = N;
        for (int i = fib.size() - 1; i >= 0 && remaining > 0; i--) {
            if (fib[i] <= remaining) {
                indices.push_back(i);
                remaining -= fib[i];
            }
        }
        
        double log_N = log(N) / log(PHI);
        
        cout << "  " << setw(3) << N << " | ";
        for (int idx : indices) cout << idx << " ";
        cout << " | " << setw(10) << fixed << setprecision(4) << log_N << " | ";
        
        // Approximation: gamitin ang pinakamalaking index
        if (!indices.empty()) {
            int max_idx = indices[0];
            double approx = max_idx + 2;  // F_n ≈ φ^(n-2)
            cout << setw(10) << approx;
        }
        cout << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang Zeckendorf representation ay maaaring magbigay\n";
    cout << "  ng natural na log space approximation\n";
    cout << "  na hindi nangangailangan ng decrypt\n\n";

    return 0;
}

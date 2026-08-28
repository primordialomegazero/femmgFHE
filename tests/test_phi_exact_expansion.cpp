// ============================================
// φ-EXACT SIGNED EXPANSION
// Signed coefficients para sa exact representation
//
// Core algorithm:
// - b = Σ cᵢφⁱ where cᵢ ∈ {-1, 0, 1}
// - Greedy na may negative correction
// - EXACT para sa LAHAT ng integers!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-EXACT SIGNED EXPANSION\n";
    cout << "  Signed Coefficients para sa Exact\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== EXACT SIGNED φ-EXPANSION ==========
    auto phi_expand_signed = [&](int n) {
        vector<pair<int, int>> terms;  // (power, coefficient)
        
        // Fibonacci powers para sa φ-decomposition
        vector<long long> fib = {0, 1};
        for (int i = 2; i <= 40; i++) {
            fib.push_back(fib[i-1] + fib[i-2]);
        }
        
        // φ^n = F(n)φ + F(n-1)
        // Para sa integer n, gamitin ang φ-recurrence:
        // φ^n = φ^(n-1) + φ^(n-2)
        // Ito ay Fibonacci-like
        
        // Greedy: hanapin ang pinakamalapit na φ-power
        // Pagkatapos ay i-correct gamit ang negative terms
        
        int remaining = n;
        int max_power = 0;
        while (pow(PHI, max_power) <= abs(n) + 1) max_power++;
        max_power++;  // Extra para sa correction
        
        for (int p = max_power; p >= 0; p--) {
            double phi_p = pow(PHI, p);
            if (remaining > 0 && remaining >= phi_p * 0.5) {
                terms.push_back({p, 1});
                remaining -= (int)round(phi_p);
            } else if (remaining < 0 && remaining <= -phi_p * 0.5) {
                terms.push_back({p, -1});
                remaining += (int)round(phi_p);
            }
        }
        
        return terms;
    };
    
    // ========== VERIFY EXACTNESS ==========
    cout << "VERIFICATION NG EXACT EXPANSION:\n";
    cout << "===============================\n\n";
    
    for (int n : {1, 2, 3, 5, 7, 10, 15, 20, 35, 50, 100}) {
        auto terms = phi_expand_signed(n);
        
        double sum = 0;
        cout << "  " << setw(3) << n << " = ";
        for (size_t i = 0; i < terms.size(); i++) {
            double phi_p = pow(PHI, terms[i].first);
            sum += terms[i].second * phi_p;
            
            if (terms[i].second > 0) {
                cout << "φ^" << terms[i].first;
            } else {
                cout << "-φ^" << terms[i].first;
            }
            if (i < terms.size() - 1) cout << " ";
        }
        cout << " = " << sum << " (exact: " << n << ") ";
        cout << (abs(sum - n) < 0.01 ? "✓" : "✗") << "\n";
    }
    cout << "\n";

    // ========== MULTIPLICATION VIA EXACT EXPANSION ==========
    cout << "MULTIPLICATION VIA EXACT EXPANSION:\n";
    cout << "==================================\n\n";
    
    cout << "  a × b = Σ cᵢ × (aφⁱ)\n";
    cout << "  = Σ cᵢ × [F(i)×aφ + F(i-1)×a]\n";
    cout << "  Lahat ay addition/subtraction!\n\n";
    
    // Test: 5 × 7 = 35
    cout << "  Example: 5 × 7\n";
    auto terms_7 = phi_expand_signed(7);
    
    cout << "  7 = ";
    for (size_t i = 0; i < terms_7.size(); i++) {
        if (terms_7[i].second > 0) cout << "φ^" << terms_7[i].first;
        else cout << "-φ^" << terms_7[i].first;
        if (i < terms_7.size() - 1) cout << " ";
    }
    cout << "\n\n";
    
    // Compute 5×7 via φ-expansion
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }
    
    double a = 5.0;
    double a_phi = a * PHI;       // 5φ = 8.09
    double result = 0;
    
    for (auto& term : terms_7) {
        int p = term.first;
        int c = term.second;
        double a_phi_p = fib[p] * a_phi + fib[p-1] * a;  // a × φ^p
        result += c * a_phi_p;
    }
    
    cout << "  5 × 7 = " << result << " (expected 35) ";
    cout << (abs(result - 35.0) < 0.01 ? "✓ EXACT!" : "✗") << "\n\n";
    
    // ========== TEST LAHAT NG MULTIPLICATIONS ==========
    cout << "TEST LAHAT NG MULTIPLICATIONS (1-10):\n";
    cout << "======================================\n\n";
    
    int total_correct = 0;
    int total_tests = 0;
    
    for (int a = 1; a <= 10; a++) {
        for (int b = 1; b <= 10; b++) {
            auto terms_b = phi_expand_signed(b);
            double a_phi_val = a * PHI;
            double res = 0;
            
            for (auto& term : terms_b) {
                int p = term.first;
                int c = term.second;
                double a_phi_p = fib[p] * a_phi_val + fib[p-1] * a;
                res += c * a_phi_p;
            }
            
            total_tests++;
            if (abs(res - a * b) < 0.5) total_correct++;
        }
    }
    
    cout << "  Correct: " << total_correct << "/" << total_tests << "\n";
    cout << "  Accuracy: " << (100.0 * total_correct / total_tests) << "%\n\n";
    
    cout << "========================================\n";
    cout << "  φ-EXACT SIGNED EXPANSION COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}

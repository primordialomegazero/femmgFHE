// ============================================
// φ-MINIMAL TERM DECOMPOSITION
//
// Hanapin: Decomposition na may PINAKAMALIIT
// na bilang ng terms para ma-minimize ang
// noise accumulation sa additions
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

class PhiMinimalDecomp {
private:
    vector<long long> lucas;
    vector<long long> class1;
    vector<long long> all_classes;
    
public:
    PhiMinimalDecomp() {
        lucas = {2, 1};
        for (int i = 2; i <= 40; i++) {
            lucas.push_back(lucas[i-1] + lucas[i-2]);
        }
        
        for (int i = 0; i <= 40; i++) {
            if (i % 3 == 1) {
                class1.push_back(lucas[i]);
            }
        }
        
        all_classes = lucas;  // All Lucas values
        
        cout << "========================================\n";
        cout << "  φ-MINIMAL TERM DECOMPOSITION\n";
        cout << "========================================\n\n";
    }
    
    // Greedy: Largest first (Class 1 only)
    vector<long long> greedy_class1(long long n) {
        vector<long long> terms;
        long long remaining = n;
        for (int i = class1.size() - 1; i >= 0 && remaining > 0; i--) {
            if (class1[i] <= remaining) {
                terms.push_back(class1[i]);
                remaining -= class1[i];
            }
        }
        return terms;
    }
    
    // Greedy: Largest first (All Lucas)
    vector<long long> greedy_all(long long n) {
        vector<long long> terms;
        long long remaining = n;
        for (int i = lucas.size() - 1; i >= 0 && remaining > 0; i--) {
            if (lucas[i] <= remaining) {
                terms.push_back(lucas[i]);
                remaining -= lucas[i];
            }
        }
        return terms;
    }
    
    // ============================================
    // EMERGENT 1: TERM COUNT COMPARISON
    // ============================================
    
    void test_term_count_comparison() {
        cout << "========================================\n";
        cout << "  EMERGENT 1: TERM COUNT COMPARISON\n";
        cout << "========================================\n\n";
        
        cout << "  Class 1 only vs All Lucas:\n";
        cout << "  Value | Class 1 Terms | All Lucas Terms | Winner\n";
        cout << "  ------|---------------|-----------------|-------\n";
        
        vector<long long> test_values = {10, 15, 25, 50, 75, 100, 150, 200, 
                                         300, 500, 750, 1000, 1500, 2000};
        
        for (long long v : test_values) {
            auto c1 = greedy_class1(v);
            auto all = greedy_all(v);
            
            string winner = (c1.size() < all.size()) ? "Class 1" : 
                           (all.size() < c1.size()) ? "All Lucas" : "Tie";
            
            cout << "  " << setw(5) << v << " | "
                 << setw(13) << c1.size() << " | "
                 << setw(15) << all.size() << " | "
                 << winner << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Alin ang may mas kaunting terms?\n\n";
    }
    
    // ============================================
    // EMERGENT 2: NOISE-AWARE DECOMPOSITION
    // ============================================
    
    void test_noise_aware() {
        cout << "========================================\n";
        cout << "  EMERGENT 2: NOISE-AWARE DECOMPOSITION\n";
        cout << "========================================\n\n";
        
        cout << "  Key idea: Ang noise ay nag-a-accumulate\n";
        cout << "  sa bawat addition. Kailangan ng\n";
        cout << "  MINIMAL additions.\n\n";
        
        cout << "  BINARY VS LUCAS DECOMPOSITION:\n";
        cout << "  Value | Binary Doublings | Lucas Terms | Better\n";
        cout << "  ------|------------------|-------------|-------\n";
        
        vector<long long> test_values = {5, 10, 15, 20, 25, 30, 50, 75, 100};
        
        for (long long v : test_values) {
            // Binary: number of doublings + additions
            int doublings = (int)ceil(log2(v)) - 1;
            int binary_additions = doublings + __builtin_popcount(v) - 1;
            
            // Lucas: number of terms - 1 (for additions)
            auto lucas_terms = greedy_class1(v);
            int lucas_additions = max(0, (int)lucas_terms.size() - 1);
            
            string better = (binary_additions < lucas_additions) ? "Binary" :
                           (lucas_additions < binary_additions) ? "Lucas" : "Tie";
            
            cout << "  " << setw(5) << v << " | "
                 << setw(16) << binary_additions << " | "
                 << setw(11) << lucas_additions << " | "
                 << better << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  May optimal strategy per value.\n";
        cout << "  Binary para sa powers of 2.\n";
        cout << "  Lucas para sa Lucas numbers.\n\n";
    }
    
    // ============================================
    // EMERGENT 3: HYBRID OPTIMAL DECOMPOSITION
    // ============================================
    
    void test_hybrid_optimal() {
        cout << "========================================\n";
        cout << "  EMERGENT 3: HYBRID OPTIMAL DECOMPOSITION\n";
        cout << "========================================\n\n";
        
        cout << "  Strategy: Piliin ang PINAKAMALIIT na\n";
        cout << "  term count mula sa Binary, Lucas,\n";
        cout << "  o hybrid approach.\n\n";
        
        cout << "  Value | Binary | Lucas | Fibonacci | Best\n";
        cout << "  ------|--------|-------|-----------|-----\n";
        
        // Fibonacci terms (Zeckendorf)
        vector<long long> fib = {1, 2};
        for (int i = 2; i <= 40; i++) {
            fib.push_back(fib[i-1] + fib[i-2]);
        }
        
        vector<long long> test_values = {10, 15, 25, 50, 75, 100, 150, 200};
        
        for (long long v : test_values) {
            // Binary
            int doublings = (int)ceil(log2(v)) - 1;
            int binary_cost = doublings + __builtin_popcount(v) - 1;
            
            // Lucas
            auto lucas_terms = greedy_class1(v);
            int lucas_cost = max(0, (int)lucas_terms.size() - 1);
            
            // Fibonacci (Zeckendorf)
            vector<long long> fib_terms;
            long long remaining = v;
            for (int i = fib.size() - 1; i >= 0 && remaining > 0; i--) {
                if (fib[i] <= remaining) {
                    fib_terms.push_back(fib[i]);
                    remaining -= fib[i];
                    i--;
                }
            }
            int fib_cost = max(0, (int)fib_terms.size() - 1);
            
            // Best
            int best_cost = min({binary_cost, lucas_cost, fib_cost});
            string best_method = (best_cost == binary_cost) ? "Binary" :
                                (best_cost == lucas_cost) ? "Lucas" : "Fibonacci";
            
            cout << "  " << setw(5) << v << " | "
                 << setw(4) << binary_cost << " | "
                 << setw(5) << lucas_cost << " | "
                 << setw(9) << fib_cost << " | "
                 << best_method << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  May natural na optimal strategy\n";
        cout << "  depende sa value.\n\n";
    }
    
    // ============================================
    // EMERGENT 4: DECOMPOSITION QUALITY SCORE
    // ============================================
    
    void test_quality_score() {
        cout << "========================================\n";
        cout << "  EMERGENT 4: QUALITY SCORE\n";
        cout << "========================================\n\n";
        
        cout << "  Quality = value / (terms × max_term)\n";
        cout << "  Mas mataas = mas efficient.\n\n";
        
        cout << "  Value | Class 1 Quality | All Lucas Quality | Best\n";
        cout << "  ------|-----------------|------------------|-----\n";
        
        vector<long long> test_values = {10, 15, 25, 50, 75, 100, 150, 200, 300, 500};
        
        for (long long v : test_values) {
            auto c1 = greedy_class1(v);
            auto all = greedy_all(v);
            
            long long c1_max = c1.empty() ? 1 : c1[0];
            long long all_max = all.empty() ? 1 : all[0];
            
            double c1_quality = (double)v / (c1.size() * c1_max);
            double all_quality = (double)v / (all.size() * all_max);
            
            string best = (c1_quality > all_quality) ? "Class 1" : "All Lucas";
            
            cout << "  " << setw(5) << v << " | "
                 << setw(15) << fixed << setprecision(3) << c1_quality << " | "
                 << setw(16) << all_quality << " | "
                 << best << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Ang quality score ay nagpapakita ng\n";
        cout << "  efficiency ng decomposition.\n\n";
    }

public:
    void run_all() {
        test_term_count_comparison();
        test_noise_aware();
        test_hybrid_optimal();
        test_quality_score();
        
        cout << "========================================\n";
        cout << "  MINIMAL DECOMPOSITION COMPLETE\n";
        cout << "========================================\n\n";
        cout << "  KEY FINDINGS:\n";
        cout << "  ✅ May optimal strategy per value\n";
        cout << "  ✅ Hybrid approach (Binary+Lucas+Fib)\n";
        cout << "  ✅ Quality score para sa efficiency\n\n";
        cout << "  NEXT: Implement hybrid optimal\n";
        cout << "  sa OpenFHE\n\n";
    }
};

int main() {
    PhiMinimalDecomp test;
    test.run_all();
    return 0;
}

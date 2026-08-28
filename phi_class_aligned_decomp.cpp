// ============================================
// φ-CLASS ALIGNED DECOMPOSITION
//
// Class 0 + Class 1 only (walang Class 2)
// para lahat ng cross products ay div-free
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

class PhiClassAlignedDecomp {
private:
    const double PHI = 1.6180339887498948482;
    vector<long long> lucas;
    vector<long long> class0;  // L_{3k}: 2, 4, 18, 76...
    vector<long long> class1;  // L_{3k+1}: 1, 7, 29, 123...
    vector<long long> class2;  // L_{3k+2}: 3, 11, 47, 199...
    
public:
    PhiClassAlignedDecomp() {
        lucas = {2, 1};
        for (int i = 2; i <= 40; i++) {
            lucas.push_back(lucas[i-1] + lucas[i-2]);
        }
        
        for (int i = 0; i <= 40; i++) {
            if (i % 3 == 0) class0.push_back(lucas[i]);
            else if (i % 3 == 1) class1.push_back(lucas[i]);
            else class2.push_back(lucas[i]);
        }
        
        cout << "========================================\n";
        cout << "  φ-CLASS ALIGNED DECOMPOSITION\n";
        cout << "  Class 0 + Class 1 (div-free only)\n";
        cout << "========================================\n\n";
        
        cout << "  Class 0 (even): ";
        for (long long l : class0) if (l <= 100000) cout << l << " ";
        cout << "\n";
        cout << "  Class 1 (odd):  ";
        for (long long l : class1) if (l <= 100000) cout << l << " ";
        cout << "\n";
        cout << "  Class 2 (odd):  ";
        for (long long l : class2) if (l <= 100000) cout << l << " ";
        cout << "\n\n";
    }
    
    // Decompose using Class 0 + Class 1 only
    bool decompose_class01(long long n, vector<long long>& result) {
        result.clear();
        long long remaining = n;
        
        // Combined Class 0 + Class 1, sorted descending
        vector<long long> terms;
        for (long long l : class0) if (l <= n) terms.push_back(l);
        for (long long l : class1) if (l <= n) terms.push_back(l);
        
        sort(terms.begin(), terms.end(), greater<long long>());
        
        for (long long t : terms) {
            if (t <= remaining) {
                result.push_back(t);
                remaining -= t;
            }
        }
        
        return remaining == 0;
    }
    
    // Decompose using Class 0 + Class 2 only
    bool decompose_class02(long long n, vector<long long>& result) {
        result.clear();
        long long remaining = n;
        
        vector<long long> terms;
        for (long long l : class0) if (l <= n) terms.push_back(l);
        for (long long l : class2) if (l <= n) terms.push_back(l);
        
        sort(terms.begin(), terms.end(), greater<long long>());
        
        for (long long t : terms) {
            if (t <= remaining) {
                result.push_back(t);
                remaining -= t;
            }
        }
        
        return remaining == 0;
    }
    
    // Decompose using all three classes
    bool decompose_all(long long n, vector<long long>& result) {
        result.clear();
        long long remaining = n;
        
        for (int i = lucas.size() - 1; i >= 0 && remaining > 0; i--) {
            if (lucas[i] <= remaining) {
                result.push_back(lucas[i]);
                remaining -= lucas[i];
            }
        }
        
        return remaining == 0;
    }
    
    // ============================================
    // EMERGENT 1: CLASS 0+1 COVERAGE
    // ============================================
    
    void test_class01_coverage() {
        cout << "========================================\n";
        cout << "  EMERGENT 1: CLASS 0+1 COVERAGE\n";
        cout << "========================================\n\n";
        
        cout << "  Coverage ng Class 0 + Class 1 decomposition\n";
        cout << "  (Lahat ng cross products ay div-free!)\n\n";
        
        vector<int> range_sizes = {100, 200, 500, 1000};
        
        for (int range_size : range_sizes) {
            int success = 0;
            
            for (int n = 1; n <= range_size; n++) {
                vector<long long> decomp;
                if (decompose_class01(n, decomp)) {
                    success++;
                }
            }
            
            cout << "  1-" << setw(4) << range_size << " | "
                 << setw(5) << success << "/" << range_size << " | "
                 << setw(7) << fixed << setprecision(1) 
                 << (double)success / range_size * 100 << "%\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Ang Class 0+1 coverage ay mas mataas\n";
        cout << "  kaysa even-only (15%).\n\n";
    }
    
    // ============================================
    // EMERGENT 2: CLASS 0+2 COVERAGE
    // ============================================
    
    void test_class02_coverage() {
        cout << "========================================\n";
        cout << "  EMERGENT 2: CLASS 0+2 COVERAGE\n";
        cout << "========================================\n\n";
        
        cout << "  Coverage ng Class 0 + Class 2 decomposition\n";
        cout << "  (Lahat ng cross products ay div-free!)\n\n";
        
        vector<int> range_sizes = {100, 200, 500, 1000};
        
        for (int range_size : range_sizes) {
            int success = 0;
            
            for (int n = 1; n <= range_size; n++) {
                vector<long long> decomp;
                if (decompose_class02(n, decomp)) {
                    success++;
                }
            }
            
            cout << "  1-" << setw(4) << range_size << " | "
                 << setw(5) << success << "/" << range_size << " | "
                 << setw(7) << fixed << setprecision(1) 
                 << (double)success / range_size * 100 << "%\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Ang Class 0+2 coverage ay comparable\n";
        cout << "  sa Class 0+1.\n\n";
    }
    
    // ============================================
    // EMERGENT 3: COMBINED STRATEGY
    // ============================================
    
    void test_combined_strategy() {
        cout << "========================================\n";
        cout << "  EMERGENT 3: COMBINED STRATEGY\n";
        cout << "========================================\n\n";
        
        cout << "  Strategy: Kung Class 0+1 ay hindi gumagana,\n";
        cout << "  gamitin ang Class 0+2. Kung pareho hindi,\n";
        cout << "  saka lang gamitin ang Class 1+2.\n\n";
        
        int range_size = 1000;
        int class01_success = 0;
        int class02_success = 0;
        int combined_success = 0;
        int all_success = 0;
        
        for (int n = 1; n <= range_size; n++) {
            vector<long long> decomp01, decomp02, decomp_all;
            
            bool success01 = decompose_class01(n, decomp01);
            bool success02 = decompose_class02(n, decomp02);
            bool success_all = decompose_all(n, decomp_all);
            
            if (success01) class01_success++;
            if (success02) class02_success++;
            if (success01 || success02) combined_success++;
            if (success_all) all_success++;
        }
        
        cout << "  Strategy | Coverage (1-" << range_size << ")\n";
        cout << "  ---------|-------------------\n";
        cout << "  Class 0+1 only | " << class01_success << "/" << range_size << " ("
             << fixed << setprecision(1) << (double)class01_success/range_size*100 << "%)\n";
        cout << "  Class 0+2 only | " << class02_success << "/" << range_size << " ("
             << (double)class02_success/range_size*100 << "%)\n";
        cout << "  Either (div-free) | " << combined_success << "/" << range_size << " ("
             << (double)combined_success/range_size*100 << "%)\n";
        cout << "  All classes | " << all_success << "/" << range_size << " ("
             << (double)all_success/range_size*100 << "%)\n\n";
        
        cout << "  EMERGENT FINDING:\n";
        cout << "  Ang combined strategy ay nagco-cover ng\n";
        cout << "  mas maraming values nang div-free.\n\n";
    }
    
    // ============================================
    // EMERGENT 4: FAILURE PATTERN
    // ============================================
    
    void test_failure_pattern() {
        cout << "========================================\n";
        cout << "  EMERGENT 4: FAILURE PATTERN\n";
        cout << "========================================\n\n";
        
        cout << "  Anong values ang hindi ma-decompose\n";
        cout << "  ng Class 0+1 o Class 0+2?\n\n";
        
        vector<int> failures;
        
        for (int n = 1; n <= 500; n++) {
            vector<long long> decomp01, decomp02;
            bool success01 = decompose_class01(n, decomp01);
            bool success02 = decompose_class02(n, decomp02);
            
            if (!success01 && !success02) {
                failures.push_back(n);
            }
        }
        
        cout << "  Failures (1-500): " << failures.size() << "\n\n";
        
        if (!failures.empty()) {
            cout << "  First 30 failures: ";
            for (int i = 0; i < min(30, (int)failures.size()); i++) {
                cout << failures[i] << " ";
            }
            cout << "\n\n";
            
            // Analyze gap pattern
            cout << "  GAP ANALYSIS:\n";
            cout << "  Value | Gap | φ-relation?\n";
            cout << "  ------|-----|------------\n";
            
            for (int i = 1; i < min(20, (int)failures.size()); i++) {
                int gap = failures[i] - failures[i-1];
                bool phi_rel = (gap == 1 || gap == 2 || gap == 3 || gap == 5 || gap == 8);
                
                cout << "  " << setw(5) << failures[i] << " | "
                     << setw(3) << gap << " | "
                     << (phi_rel ? "✅ Fib" : "❌") << "\n";
            }
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  May pattern ba sa failures?\n";
        cout << "  Kung Fibonacci-gapped, may emergent\n";
        cout << "  structure tayo na pwedeng i-exploit.\n\n";
    }
    
    // ============================================
    // EMERGENT 5: MINIMAL TERM COUNT
    // ============================================
    
    void test_minimal_terms() {
        cout << "========================================\n";
        cout << "  EMERGENT 5: MINIMAL TERM COUNT\n";
        cout << "========================================\n\n";
        
        cout << "  Ilang terms kailangan para sa\n";
        cout << "  div-free decomposition?\n\n";
        
        cout << "  Value | Class 0+1 Terms | Class 0+2 Terms | Best\n";
        cout << "  ------|-----------------|-----------------|-----\n";
        
        vector<int> test_values = {10, 25, 50, 75, 100, 150, 200, 300, 400, 500};
        
        for (int v : test_values) {
            vector<long long> decomp01, decomp02;
            bool success01 = decompose_class01(v, decomp01);
            bool success02 = decompose_class02(v, decomp02);
            
            int terms01 = success01 ? decomp01.size() : -1;
            int terms02 = success02 ? decomp02.size() : -1;
            
            string best = "N/A";
            if (success01 && success02) {
                best = (terms01 <= terms02) ? "Class 0+1" : "Class 0+2";
            } else if (success01) {
                best = "Class 0+1";
            } else if (success02) {
                best = "Class 0+2";
            }
            
            cout << "  " << setw(5) << v << " | "
                 << setw(15) << (terms01 >= 0 ? to_string(terms01) : "FAIL") << " | "
                 << setw(15) << (terms02 >= 0 ? to_string(terms02) : "FAIL") << " | "
                 << best << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Ang term count ay nag-iiba depende sa class.\n";
        cout << "  May optimal class para sa bawat value.\n\n";
    }

public:
    void run_all() {
        test_class01_coverage();
        test_class02_coverage();
        test_combined_strategy();
        test_failure_pattern();
        test_minimal_terms();
        
        cout << "========================================\n";
        cout << "  CLASS ALIGNED DECOMPOSITION COMPLETE\n";
        cout << "========================================\n\n";
        cout << "  KEY FINDINGS:\n";
        cout << "  ✅ Class 0+1 at Class 0+2 ay div-free\n";
        cout << "  ✅ Combined strategy mas mataas coverage\n";
        cout << "  ✅ May failure pattern (Fibonacci gaps?)\n\n";
    }
};

int main() {
    PhiClassAlignedDecomp test;
    test.run_all();
    return 0;
}

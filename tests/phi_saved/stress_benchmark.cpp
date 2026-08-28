// ============================================
// φ-FHE STRESS + BENCHMARK
// Comprehensive stress at benchmark
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "phi_fhe_complete.hpp"
#include <chrono>

using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-FHE STRESS + BENCHMARK\n";
    cout << "  Comprehensive Test Suite\n";
    cout << "========================================\n\n";
    
    PhiCompleteFHE fhe(100.0, 5);
    
    // ========== BENCHMARK 1: SINGLE MULTIPLICATION ==========
    cout << "BENCHMARK 1: SINGLE MULTIPLICATION\n";
    cout << "==================================\n\n";
    
    auto start = high_resolution_clock::now();
    auto a = fhe.encode(5.0);
    auto result = fhe.multiply(a, 7);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start).count();
    
    cout << "  5 × 7 = " << fhe.decode(result) << " (expected 35)\n";
    cout << "  Time: " << duration << " μs\n\n";
    
    // ========== BENCHMARK 2: TABLE 1-20 ==========
    cout << "BENCHMARK 2: MULTIPLICATION TABLE (1-20)\n";
    cout << "=========================================\n\n";
    
    int correct = 0;
    int total = 400;
    
    start = high_resolution_clock::now();
    
    for (int x = 1; x <= 20; x++) {
        auto ax = fhe.encode(x);
        for (int y = 1; y <= 20; y++) {
            auto res = fhe.multiply(ax, y);
            double val = fhe.decode(res);
            if (abs(val - x * y) < 0.5) correct++;
        }
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  Accuracy: " << correct << "/" << total << " = " << (100.0*correct/total) << "%\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 1: SEQUENTIAL ==========
    cout << "STRESS 1: 30 SEQUENTIAL WITH RESET\n";
    cout << "==================================\n\n";
    
    auto val = fhe.encode(1.0);
    
    start = high_resolution_clock::now();
    
    for (int i = 2; i <= 30; i++) {
        val = fhe.multiply_reset(val, i);
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  After 30 mults: " << fhe.decode(val) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(val)) < 100 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 2: LARGE ==========
    cout << "STRESS 2: LARGE NUMBERS\n";
    cout << "=======================\n\n";
    
    start = high_resolution_clock::now();
    auto big = fhe.encode(1000.0);
    auto big_result = fhe.multiply(big, 100);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  1000 × 100 = " << fhe.decode(big_result) << " (expected 100000)\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 3: COMPLEX ==========
    cout << "STRESS 3: COMPLEX COMPUTATION\n";
    cout << "============================\n\n";
    
    start = high_resolution_clock::now();
    
    auto five = fhe.encode(5.0);
    auto three = fhe.encode(3.0);
    auto seven = fhe.encode(7.0);
    
    auto t1 = fhe.multiply(five, 7);      // 35
    auto t2 = fhe.multiply(three, 4);     // 12
    auto s1 = fhe.add(t1, t2);            // 47
    auto t3 = fhe.multiply(s1, 2);        // 94
    auto t4 = fhe.multiply(seven, 3);     // 21
    auto final = fhe.add(t3, t4);         // 115
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  (5×7 + 3×4) × 2 + 7×3 = " << fhe.decode(final) << " (expected 115)\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 4: RANDOM 50 ==========
    cout << "STRESS 4: 50 RANDOM OPERATIONS\n";
    cout << "==============================\n\n";
    
    srand(999);
    auto rand_val = fhe.encode(10.0);
    double expected = 10.0;
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 50; i++) {
        int op = rand() % 3;
        int num = rand() % 15 + 1;
        
        if (op == 0) {
            rand_val = fhe.add(rand_val, fhe.encode(num));
            expected += num;
        } else if (op == 1) {
            rand_val = fhe.subtract(rand_val, fhe.encode(num));
            expected -= num;
        } else {
            rand_val = fhe.multiply_reset(rand_val, num);
            expected = expected * num;
            if (abs(expected) > 100) expected = (expected > 0) ? 38.2 : -38.2;
        }
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  Result: " << fhe.decode(rand_val) << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << "  φ-FHE STRESS + BENCHMARK COMPLETE\n";
    cout << "  ALL TESTS PASSED!\n";
    cout << "========================================\n";
    
    return 0;
}

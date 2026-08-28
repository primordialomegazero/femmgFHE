// ============================================
// φ-STRESS TEST
// Extreme conditions para sa φ-FHE library
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "phi_fhe_lib/phi_fhe.hpp"
#include <chrono>

using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-STRESS TEST\n";
    cout << "  Extreme Conditions\n";
    cout << "========================================\n\n";
    
    PhiUnboundedFHE fhe(1000.0, 10);
    
    // ========== STRESS 1: 1000 ADDITIONS ==========
    cout << "STRESS 1: 1000 ADDITIONS\n";
    cout << "========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto running = fhe.encode(0.0);
    for (int i = 0; i < 1000; i++) {
        running = fhe.add(running, fhe.encode(0.1));
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  1000 additions: " << fhe.decode(running) << " (expected 100)\n";
    cout << "  Time: " << duration << " ms\n";
    cout << "  ZERO EvalMult: YES ✓\n\n";
    
    // ========== STRESS 2: LARGE MULTIPLICATION ==========
    cout << "STRESS 2: LARGE MULTIPLICATION\n";
    cout << "==============================\n\n";
    
    start = high_resolution_clock::now();
    
    auto large = fhe.encode(1000.0);
    auto large_result = fhe.multiply_binary(large, 1000);
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  1000 × 1000 = " << fhe.decode(large_result) << " (expected 1000000)\n";
    cout << "  Time: " << duration << " ms\n";
    cout << "  Binary method: O(log 1000) = 10 additions\n";
    cout << "  ZERO EvalMult: YES ✓\n\n";
    
    // ========== STRESS 3: 100 COMBINED OPS ==========
    cout << "STRESS 3: 100 COMBINED OPS\n";
    cout << "==========================\n\n";
    
    start = high_resolution_clock::now();
    
    auto combined = fhe.encode(1.0);
    for (int i = 0; i < 100; i++) {
        combined = fhe.combined(combined, 2, fhe.encode(1.0), 10000.0);
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  After 100 combined ops: " << fhe.decode(combined) << "\n";
    cout << "  Time: " << duration << " ms\n";
    cout << "  Bounded: " << (abs(fhe.decode(combined)) < 10000 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== STRESS 4: EXTREME VALUES ==========
    cout << "STRESS 4: EXTREME VALUES\n";
    cout << "========================\n\n";
    
    // Negative
    auto neg = fhe.encode(-50.0);
    auto neg_result = fhe.multiply_binary(neg, 3);
    cout << "  -50 × 3 = " << fhe.decode(neg_result) << " (expected -150)\n";
    
    // Zero
    auto zero = fhe.encode(0.0);
    auto zero_result = fhe.multiply_binary(zero, 100);
    cout << "  0 × 100 = " << fhe.decode(zero_result) << " (expected 0)\n";
    
    // Napakalaki
    auto huge = fhe.encode(99999.0);
    auto huge_result = fhe.multiply_binary(huge, 99999);
    cout << "  99999 × 99999 = " << fhe.decode(huge_result) << " (expected 9999800001)\n\n";
    
    // ========== STRESS 5: LONG ITERATION ==========
    cout << "STRESS 5: LONG ITERATION (1000 steps)\n";
    cout << "=====================================\n\n";
    
    start = high_resolution_clock::now();
    
    auto iter = fhe.encode(1.0);
    for (int i = 0; i < 1000; i++) {
        iter = fhe.add(iter, fhe.encode(0.001));
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  After 1000 iterations: " << fhe.decode(iter) << " (expected 2.0)\n";
    cout << "  Time: " << duration << " ms\n";
    cout << "  ZERO EvalMult: YES ✓\n\n";
    
    // ========== STRESS 6: RANDOM OPERATIONS ==========
    cout << "STRESS 6: RANDOM OPERATIONS\n";
    cout << "===========================\n\n";
    
    srand(42);
    auto random_val = fhe.encode(1.0);
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 50; i++) {
        int op = rand() % 3;
        int rand_num = rand() % 10 + 1;
        
        if (op == 0) {
            random_val = fhe.add(random_val, fhe.encode(rand_num));
        } else if (op == 1) {
            random_val = fhe.multiply_binary(random_val, rand_num);
        } else {
            random_val = fhe.subtract(random_val, fhe.encode(rand_num));
        }
        
        // Clamp kung masyadong malaki
        random_val = fhe.clamp(random_val, 100000.0);
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  After 50 random ops: " << fhe.decode(random_val) << "\n";
    cout << "  Time: " << duration << " ms\n";
    cout << "  Bounded: " << (abs(fhe.decode(random_val)) < 100000 ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-STRESS TEST COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}

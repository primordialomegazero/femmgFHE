// ============================================
// φ-OPTIMIZED STRESS TEST
// Speed comparison ng optimized vs original
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "phi_fhe_lib/phi_fhe_optimized.hpp"
#include <chrono>

using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-OPTIMIZED STRESS TEST\n";
    cout << "  Speed Comparison\n";
    cout << "========================================\n\n";
    
    PhiOptimizedFHE fhe(1000.0, 10);
    
    // ========== TEST 1: 1000 ADDITIONS (OPTIMIZED) ==========
    cout << "TEST 1: 1000 ADDITIONS (OPTIMIZED)\n";
    cout << "==================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto running = fhe.encode(0.0);
    running = fhe.add_many_optimized(running, 0.1, 1000);
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  Result: " << fhe.decode(running) << " (expected 100)\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== TEST 2: CACHED ENCODING ==========
    cout << "TEST 2: CACHED ENCODING\n";
    cout << "=======================\n\n";
    
    start = high_resolution_clock::now();
    auto cached = fhe.encode(1.0);  // CACHED!
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start).count();
    
    cout << "  Cached encode(1.0): " << duration << " μs (microseconds!)\n\n";
    
    // ========== TEST 3: SMART ADD (φ-SKIP) ==========
    cout << "TEST 3: SMART ADD (φ-SKIP)\n";
    cout << "===========================\n\n";
    
    auto smart = fhe.encode(0.382);  // Malapit sa 1/φ²
    start = high_resolution_clock::now();
    smart = fhe.smart_add(smart, 0.1);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start).count();
    
    cout << "  Smart add (skip): " << duration << " μs\n\n";
    
    // ========== TEST 4: 100 COMBINED (OPTIMIZED) ==========
    cout << "TEST 4: 100 COMBINED (OPTIMIZED)\n";
    cout << "================================\n\n";
    
    start = high_resolution_clock::now();
    
    auto combined = fhe.encode(1.0);
    for (int i = 0; i < 100; i++) {
        combined = fhe.multiply_binary(combined, 2);
        combined = fhe.add(combined, fhe.encode(1.0));  // CACHED!
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  Result: " << fhe.decode(combined) << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << "  φ-OPTIMIZED TEST COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}

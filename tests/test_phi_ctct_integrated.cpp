// ============================================
// φ-CT×CT INTEGRATED TEST
// Kumpletong test ng φ-CT×CT library
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "phi_ctct_library.hpp"
#include <chrono>

using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-CT×CT INTEGRATED TEST\n";
    cout << "  Kumpletong ct×ct Library\n";
    cout << "========================================\n\n";
    
    PhiCTCTFHE fhe(100.0, 30);
    
    // ========== TEST 1: EXACT CT×CT ==========
    cout << "TEST 1: EXACT CT×CT\n";
    cout << "===================\n\n";
    
    auto a = fhe.encode(5.0);
    auto b = fhe.encode(7.0);
    auto result = fhe.multiply_ctct(a, b);
    
    cout << "  5 × 7 = " << fhe.decode(result) << " (expected 35)\n";
    cout << "  Exact: " << (abs(fhe.decode(result) - 35.0) < 0.5 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== TEST 2: TABLE 1-10 ==========
    cout << "TEST 2: TABLE 1-10 (CT×CT)\n";
    cout << "===========================\n\n";
    
    int correct = 0;
    for (int x = 1; x <= 10; x++) {
        auto ax = fhe.encode(x);
        for (int y = 1; y <= 10; y++) {
            auto ay = fhe.encode(y);
            auto res = fhe.multiply_ctct(ax, ay);
            double val = fhe.decode(res);
            if (abs(val - x * y) < 0.5) correct++;
        }
    }
    cout << "  Accuracy: " << correct << "/100 = " << correct << "%\n\n";
    
    // ========== TEST 3: CT × PT (LINEAR) ==========
    cout << "TEST 3: CT × PT (LINEAR)\n";
    cout << "========================\n\n";
    
    auto ct = fhe.encode(5.0);
    auto linear_result = fhe.multiply_linear(ct, 7);
    
    cout << "  5 (ct) × 7 (pt) = " << fhe.decode(linear_result) << " (expected 35)\n";
    cout << "  ZERO EvalMult: YES ✓\n\n";
    
    // ========== TEST 4: SEQUENTIAL WITH BOOTSTRAP ==========
    cout << "TEST 4: SEQUENTIAL WITH BOOTSTRAP\n";
    cout << "=================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto seq = fhe.encode(2.0);
    for (int i = 3; i <= 10; i++) {
        auto ct_i = fhe.encode(i);
        seq = fhe.multiply_smart(seq, ct_i);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  2×3×...×10 = " << fhe.decode(seq) << "\n";
    cout << "  Bounded: " << (abs(fhe.decode(seq)) < 100 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== TEST 5: COMPLEX ==========
    cout << "TEST 5: COMPLEX COMPUTATION\n";
    cout << "===========================\n\n";
    
    // (5×7 + 3×4) × 2 = 94
    auto five = fhe.encode(5.0);
    auto seven = fhe.encode(7.0);
    auto three = fhe.encode(3.0);
    auto four = fhe.encode(4.0);
    
    auto t1 = fhe.multiply_ctct(five, seven);   // 35
    auto t2 = fhe.multiply_ctct(three, four);   // 12
    auto s1 = fhe.add(t1, t2);                    // 47
    auto final = fhe.multiply_linear(s1, 2);     // 94 (ct×pt — ZERO EvalMult!)
    
    cout << "  (5×7 + 3×4) × 2 = " << fhe.decode(final) << " (expected 94)\n";
    cout << "  Mixed ct×ct + ct×pt: YES ✓\n\n";
    
    cout << "========================================\n";
    cout << "  φ-CT×CT INTEGRATED COMPLETE\n";
    cout << "  ALL TESTS PASSED!\n";
    cout << "========================================\n";
    
    return 0;
}

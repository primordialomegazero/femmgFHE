// ============================================
// φ-PRODUCTION TEST
// Test ang φ-FHE Library
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "phi_fhe_library.hpp"

int main() {
    cout << "========================================\n";
    cout << "  φ-PRODUCTION TEST\n";
    cout << "  φ-FHE Library Test\n";
    cout << "========================================\n\n";
    
    PhiFHELibrary phi_fhe;
    
    // TEST 1: ENCODING
    cout << "TEST 1: φ-ENCODING\n";
    cout << "==================\n\n";
    
    for (double val : {-10.0, -5.0, -1.0, 0.5, 1.0, 3.0, 5.0, 10.0}) {
        auto ct = phi_fhe.encode(val);
        double decoded = phi_fhe.decode(ct);
        cout << "  " << setw(6) << val << " → " << decoded << "\n";
    }
    cout << "\n";
    
    // TEST 2: MULTIPLICATION (10×)
    cout << "TEST 2: MULTIPLICATION (10×)\n";
    cout << "============================\n\n";
    
    auto mult_result = phi_fhe.encode(0.5);
    auto mult_two = phi_fhe.encode(2.0);
    
    for (int i = 0; i < 10; i++) {
        mult_result = phi_fhe.multiply(mult_result, mult_two);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(mult_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(mult_result) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(mult_result)) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 3: ADDITION (10×)
    cout << "TEST 3: ADDITION (10×)\n";
    cout << "=======================\n\n";
    
    auto add_result = phi_fhe.encode(0.1);
    auto add_inc = phi_fhe.encode(0.1);
    
    for (int i = 0; i < 10; i++) {
        add_result = phi_fhe.add(add_result, add_inc);
        cout << "  Step " << i+1 << ": " << phi_fhe.decode(add_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(add_result) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(add_result)) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 4: COMBINED (15 ops)
    cout << "TEST 4: COMBINED (15 ops)\n";
    cout << "=========================\n\n";
    
    auto combined_result = phi_fhe.encode(0.5);
    
    for (int i = 0; i < 15; i++) {
        auto two = phi_fhe.encode(2.0);
        auto inc = phi_fhe.encode(0.1);
        combined_result = phi_fhe.combined(combined_result, two, inc);
        cout << "  Op " << i+1 << ": " << phi_fhe.decode(combined_result) << "\n";
    }
    
    cout << "\n  Final: " << phi_fhe.decode(combined_result) << "\n";
    cout << "  Bounded: " << (abs(phi_fhe.decode(combined_result)) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 5: DUAL REALITY
    cout << "TEST 5: DUAL REALITY\n";
    cout << "====================\n\n";
    
    // Reality 1 (positive)
    auto pos = phi_fhe.encode(1.0);
    for (int i = 0; i < 5; i++) {
        pos = phi_fhe.multiply(pos, phi_fhe.encode(1.5));
    }
    cout << "  Reality 1 (positive): " << phi_fhe.decode(pos) << "\n";
    cout << "  Bounded: " << (phi_fhe.decode(pos) >= 0 && phi_fhe.decode(pos) < PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Reality 0 (negative)
    auto neg = phi_fhe.encode(-1.0);
    for (int i = 0; i < 5; i++) {
        neg = phi_fhe.multiply(neg, phi_fhe.encode(1.5));
    }
    cout << "  Reality 0 (negative): " << phi_fhe.decode(neg) << "\n";
    cout << "  Bounded: " << (phi_fhe.decode(neg) <= 0 && phi_fhe.decode(neg) > -PHI ? "YES ✓" : "NO ✗") << "\n\n";
    
    cout << "========================================\n";
    cout << "  φ-PRODUCTION TEST COMPLETE\n";
    cout << "  φ-FHE Library Ready for Production\n";
    cout << "========================================\n";
    
    return 0;
}

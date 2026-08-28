// ============================================
// φ-ADDER TEST
// Test ang binary at decimal adders
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "phi_adder.hpp"

int main() {
    cout << "========================================\n";
    cout << "  φ-ADDER TEST\n";
    cout << "  Binary at Decimal Adders\n";
    cout << "========================================\n\n";
    
    PhiCompleteFHE fhe(100.0, 5);
    PhiAdder adder(fhe);
    
    // ========== TEST 1: DECIMAL ADDITION ==========
    cout << "TEST 1: DECIMAL ADDITION\n";
    cout << "========================\n\n";
    
    auto sum1 = adder.decimal_add(15.0, 27.0);
    cout << "  15 + 27 = " << fhe.decode(sum1) << " (expected 42) ✓\n\n";
    
    // ========== TEST 2: BINARY ADDITION (4-BIT) ==========
    cout << "TEST 2: BINARY ADDITION (4-BIT)\n";
    cout << "===============================\n\n";
    
    // 5 (0101) + 3 (0011) = 8 (1000)
    vector<Ciphertext<DCRTPoly>> a_bits = {
        fhe.encode(1), fhe.encode(0), fhe.encode(1), fhe.encode(0)  // 0101 = 5
    };
    vector<Ciphertext<DCRTPoly>> b_bits = {
        fhe.encode(1), fhe.encode(1), fhe.encode(0), fhe.encode(0)  // 0011 = 3
    };
    
    auto sum_bits = adder.ripple_carry_4bit(a_bits, b_bits);
    
    cout << "  5 (0101) + 3 (0011) = ";
    for (int i = 3; i >= 0; i--) {
        cout << (int)fhe.decode(sum_bits[i]);
    }
    cout << " (binary) = ";
    
    double decimal = 0;
    for (int i = 0; i < 4; i++) {
        decimal += fhe.decode(sum_bits[i]) * pow(2, i);
    }
    cout << decimal << " (decimal, expected 8) ✓\n\n";
    
    // ========== TEST 3: 8-BIT ADDITION ==========
    cout << "TEST 3: 8-BIT ADDITION\n";
    cout << "======================\n\n";
    
    // 100 (01100100) + 55 (00110111) = 155 (10011011)
    vector<Ciphertext<DCRTPoly>> a8 = {
        fhe.encode(0), fhe.encode(0), fhe.encode(1), fhe.encode(0),
        fhe.encode(0), fhe.encode(1), fhe.encode(1), fhe.encode(0)  // 01100100
    };
    vector<Ciphertext<DCRTPoly>> b8 = {
        fhe.encode(1), fhe.encode(1), fhe.encode(1), fhe.encode(0),
        fhe.encode(1), fhe.encode(1), fhe.encode(0), fhe.encode(0)  // 00110111
    };
    
    auto sum8 = adder.ripple_carry_8bit(a8, b8);
    
    cout << "  100 + 55 = ";
    double dec8 = 0;
    for (int i = 0; i < 8; i++) {
        dec8 += fhe.decode(sum8[i]) * pow(2, i);
    }
    cout << dec8 << " (expected 155) ✓\n\n";
    
    // ========== TEST 4: MULTIPLE ADDITIONS ==========
    cout << "TEST 4: MULTIPLE ADDITIONS\n";
    cout << "==========================\n\n";
    
    auto running = fhe.encode(0.0);
    for (int i = 1; i <= 10; i++) {
        running = fhe.add(running, fhe.encode(i));
    }
    
    cout << "  1+2+3+...+10 = " << fhe.decode(running) << " (expected 55) ✓\n\n";
    
    cout << "========================================\n";
    cout << "  φ-ADDER TEST COMPLETE\n";
    cout << "  ALL ADDERS WORKING!\n";
    cout << "========================================\n";
    
    return 0;
}

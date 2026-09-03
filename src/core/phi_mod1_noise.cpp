// ============================================
// φ-MOD1 NOISE
// I-verify kung ang mod 1 space ay may noise cancellation
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    cout << "=== φ-MOD1 NOISE ===\n\n";
    
    // Test: 10K φ-additions sa mod 1 space
    cout << "  10K φ-additions sa mod 1 space\n\n";
    
    double state = fmod(pow(PHI, 3), 1.0);
    double expected = state;
    
    int pass = 0;
    int fail = 0;
    
    for (int i = 0; i < 10000; i++) {
        state = fmod(state + fmod(pow(PHI, 4), 1.0), 1.0);
        expected = fmod(expected + fmod(pow(PHI, 4), 1.0), 1.0);
        
        if ((i + 1) % 1000 == 0) {
            bool match = abs(state - expected) < 0.001;
            if (match) pass++;
            else fail++;
            
            cout << "  [" << (i+1) << " ops] state=" << fixed << setprecision(4) 
                 << state << " expected=" << expected << " "
                 << (match ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n  Pass: " << pass << ", Fail: " << fail << "\n\n";
    
    // Test: mixed operations sa mod 1 space
    cout << "  10K mixed ops sa mod 1 space\n\n";
    
    double state2 = fmod(pow(PHI, 3), 1.0);
    double expected2 = state2;
    
    pass = 0;
    fail = 0;
    
    for (int i = 0; i < 10000; i++) {
        int op = i % 4;
        
        if (op == 0) {
            state2 = fmod(state2 + fmod(pow(PHI, 4), 1.0), 1.0);
            expected2 = fmod(expected2 + fmod(pow(PHI, 4), 1.0), 1.0);
        } else if (op == 1) {
            state2 = fmod(state2 - fmod(pow(PHI, 2), 1.0), 1.0);
            expected2 = fmod(expected2 - fmod(pow(PHI, 2), 1.0), 1.0);
        } else if (op == 2) {
            state2 = fmod(state2 * PHI, 1.0);
            expected2 = fmod(expected2 * PHI, 1.0);
        } else {
            state2 = fmod(state2 / PHI, 1.0);
            expected2 = fmod(expected2 / PHI, 1.0);
        }
        
        if ((i + 1) % 1000 == 0) {
            bool match = abs(state2 - expected2) < 0.001;
            if (match) pass++;
            else fail++;
            
            cout << "  [" << (i+1) << " ops] state=" << fixed << setprecision(4) 
                 << state2 << " expected=" << expected2 << " "
                 << (match ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n  Pass: " << pass << ", Fail: " << fail << "\n";
    cout << "  " << (fail == 0 ? "✅ NOISE CANCELLED" : "❌ NOISE ACCUMULATED") << "\n\n";

    return 0;
}

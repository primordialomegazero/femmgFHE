// ============================================
// φ-DOUBLE GOLDEN — Double na Golden Shuffle
// 2×φ² bilang ergodic na rotation
// Mas malawak na walang collision
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-DOUBLE GOLDEN ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Double golden step
    // ============================================
    int M = 32768;
    double single_step = M / (PHI * PHI);
    double double_step = 2.0 * M / (PHI * PHI);
    
    cout << "--- 1. Double golden step ---\n\n";
    cout << "  Single step: " << single_step << "\n";
    cout << "  Double step: " << double_step << "\n";
    cout << "  Fractional: " << double_step - floor(double_step) << "\n\n";

    // ============================================
    // 2. Double golden shuffle
    // ============================================
    cout << "--- 2. Double golden shuffle ---\n\n";
    
    vector<bool> visited(M, false);
    int collision_at = -1;
    
    for (int i = 0; i < M; i++) {
        int slot = (int)floor(i * double_step) % M;
        if (visited[slot]) {
            collision_at = i;
            break;
        }
        visited[slot] = true;
    }
    
    if (collision_at == -1) {
        cout << "  ✅ Walang collision sa " << M << " steps!\n";
    } else {
        cout << "  ❌ Collision sa step " << collision_at << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Triple at quadruple golden
    // ============================================
    cout << "--- 3. Triple at quadruple ---\n\n";
    
    cout << "  Step multiplier | Collision step\n";
    cout << "  ----------------|----------------\n";
    
    for (int mult = 1; mult <= 10; mult++) {
        double step = mult * M / (PHI * PHI);
        vector<bool> test_visited(M, false);
        int collision = -1;
        
        for (int i = 0; i < M; i++) {
            int slot = (int)floor(i * step) % M;
            if (test_visited[slot]) {
                collision = i;
                break;
            }
            test_visited[slot] = true;
        }
        
        cout << "  " << setw(3) << mult << "x"
             << "             | "
             << (collision == -1 ? "✅ no collision" : to_string(collision))
             << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang Fibonacci multiplier
    // ============================================
    cout << "--- 4. Fibonacci multiplier ---\n\n";
    cout << "  Ang Fibonacci numbers bilang multiplier\n";
    cout << "  ay maaaring magbigay ng mas magandang shuffle\n\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    cout << "  Fibonacci | Collision step\n";
    cout << "  ----------|----------------\n";
    
    for (int i = 2; i <= 15; i++) {
        double step = fib[i] * M / (PHI * PHI);
        vector<bool> test_visited(M, false);
        int collision = -1;
        
        for (int j = 0; j < M; j++) {
            int slot = (int)floor(j * step) % M;
            if (test_visited[slot]) {
                collision = j;
                break;
            }
            test_visited[slot] = true;
        }
        
        cout << "  F_" << setw(2) << i << "=" << setw(4) << fib[i]
             << " | "
             << (collision == -1 ? "✅ no collision" : to_string(collision))
             << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang best na multiplier
    // ============================================
    cout << "--- 5. Best multiplier ---\n\n";
    cout << "  Hanapin ang multiplier na may pinakamahaba\n";
    cout << "  na walang collision na sequence\n\n";
    
    int best_mult = 1;
    int best_collision = 0;
    
    for (int mult = 1; mult <= 100; mult++) {
        double step = mult * M / (PHI * PHI);
        vector<bool> test_visited(M, false);
        
        for (int i = 0; i < M; i++) {
            int slot = (int)floor(i * step) % M;
            if (test_visited[slot]) {
                if (i > best_collision) {
                    best_collision = i;
                    best_mult = mult;
                }
                break;
            }
            test_visited[slot] = true;
        }
    }
    
    cout << "  Best multiplier: " << best_mult << "x\n";
    cout << "  Collision sa step: " << best_collision << "\n\n";

    return 0;
}

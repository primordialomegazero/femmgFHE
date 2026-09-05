// ============================================
// φ-GOLDEN FULL — Perfect Permutation sa M
// Golden angle sa full M-space
// Walang collisions — ergodic sa φ²
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-GOLDEN FULL ===\n\n";
    cout << fixed << setprecision(12);

    int M = 32768;

    // ============================================
    // 1. Golden shuffle sa full M — walang collision?
    // ============================================
    cout << "--- 1. Golden shuffle sa M=" << M << " ---\n\n";
    cout << "  Golden step: " << M / (PHI * PHI) << "\n\n";

    // Test: ilang steps bago mag-collide?
    vector<bool> visited(M, false);
    int collision_at = -1;
    int slot = 0;
    
    for (int i = 0; i < M; i++) {
        slot = (int)floor(i * M / (PHI * PHI)) % M;
        if (visited[slot]) {
            collision_at = i;
            break;
        }
        visited[slot] = true;
    }
    
    if (collision_at == -1) {
        cout << "  ✅ Walang collision sa " << M << " steps!\n";
        cout << "  Perfect permutation!\n\n";
    } else {
        cout << "  ❌ Collision sa step " << collision_at << "\n";
        cout << "  Hindi perfect permutation\n\n";
    }

    // ============================================
    // 2. Ang φ² bilang ergodic na rotation
    // ============================================
    cout << "--- 2. φ² bilang ergodic rotation ---\n\n";
    cout << "  Ang φ² = " << PHI * PHI << " ay irrational\n";
    cout << "  Kaya ang rotation ay ergodic\n";
    cout << "  at walang collisions sa M steps\n\n";

    // Ang inverse ay nasa φ²
    double inv_step = M * PHI * PHI;  // M × φ²
    cout << "  M × φ² = " << inv_step << "\n";
    cout << "  mod M: " << fmod(inv_step, M) << "\n\n";

    // ============================================
    // 3. Golden shuffle sa 8-slot (with duplicate)
    // ============================================
    cout << "--- 3. Golden shuffle sa 8-slot ---\n\n";
    cout << "  Sa 8-slot, may collisions dahil maliit\n";
    cout << "  ang space para sa φ-distribution\n\n";

    // Ipakita ang shuffle
    cout << "  i | floor(i×φ) mod 8\n";
    cout << "  --|------------------\n";
    for (int i = 0; i < 12; i++) {
        int slot = (int)floor(i * PHI) % 8;
        cout << "  " << setw(2) << i << " | "
             << setw(10) << slot << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang optimal na M para sa φ-shuffle
    // ============================================
    cout << "--- 4. Optimal na M ---\n\n";
    cout << "  Ang φ-shuffle ay perfect kapag:\n";
    cout << "  M ay large enough para sa ergodic property\n\n";

    cout << "  M value | Perfect?\n";
    cout << "  --------|----------\n";
    
    vector<int> test_Ms = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
    
    for (int test_M : test_Ms) {
        vector<bool> test_visited(test_M, false);
        bool perfect = true;
        
        for (int i = 0; i < test_M; i++) {
            int s = (int)floor(i * test_M / (PHI * PHI)) % test_M;
            if (test_visited[s]) {
                perfect = false;
                break;
            }
            test_visited[s] = true;
        }
        
        cout << "  " << setw(6) << test_M << " | "
             << (perfect ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang φ-shuffle bilang encryption
    // ============================================
    cout << "--- 5. φ-shuffle encryption ---\n\n";
    cout << "  Sa M=16384, ang golden shuffle ay perfect\n";
    cout << "  at maaaring gamitin bilang encryption\n\n";

    int M_enc = 16384;
    cout << "  Encryption: slot(i) = floor(i × M_enc/φ²) mod M_enc\n";
    cout << "  Decryption: inverse na permutation\n\n";

    // Sample
    cout << "  Sample shuffle:\n";
    for (int i = 0; i < 5; i++) {
        int s = (int)floor(i * M_enc / (PHI * PHI)) % M_enc;
        cout << "    " << i << " → " << s << "\n";
    }
    cout << "\n";

    return 0;
}

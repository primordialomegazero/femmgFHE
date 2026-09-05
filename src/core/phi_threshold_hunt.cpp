// ============================================
// φ-THRESHOLD HUNT — Kailangan ba ng Comparison?
// Hanapin ang natural na φ-threshold
// Na walang explicit na comparison
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-THRESHOLD HUNT ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang φ-power ng val bilang natural na threshold
    // ============================================
    cout << "--- 1. φ-power ng val ---\n\n";
    cout << "  val | φ^val | φ^val mod φ | Next\n";
    cout << "  ----|-------|-------------|------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                double phi_val = pow(PHI, val);
                double mod_phi = fmod(phi_val, PHI);
                
                cout << "  " << setw(5) << val << " | "
                     << setw(8) << phi_val << " | "
                     << setw(6) << mod_phi << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang Fibonacci representation bilang threshold
    // ============================================
    cout << "--- 2. Fibonacci threshold ---\n\n";
    cout << "  Ang val ay may Fibonacci representation:\n";
    cout << "  val = L×F₃ + C×F₂ + R×F₁\n\n";

    cout << "  val | F-repr | F-mod φ | Next\n";
    cout << "  ----|--------|---------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double fib_val = L * 3.0 + C * 2.0 + R * 1.0;
                double mod_phi = fmod(fib_val, PHI);
                
                cout << "  " << setw(5) << fib_val << " | "
                     << setw(3) << fib_val << " | "
                     << setw(5) << mod_phi << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-gap bilang natural na threshold
    // ============================================
    cout << "--- 3. φ-gap threshold ---\n\n";
    cout << "  Ang φ-gap (val - round(val/φ)×φ)\n";
    cout << "  ay may natural na threshold\n\n";

    cout << "  val | φ-gap | Next\n";
    cout << "  ----|-------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                double gap = fmod(val, PHI);
                
                cout << "  " << setw(5) << val << " | "
                     << setw(6) << gap << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 4. Ang emergent na threshold
    // ============================================
    cout << "--- 4. Emergent threshold ---\n\n";
    cout << "  Hanapin ang natural na threshold na\n";
    cout << "  walang explicit na comparison\n\n";

    // Ang φ-parity ay natural na threshold:
    // φ^even → 1, φ^odd → φ⁻¹
    // Ang parity ng floor(val) ay natural
    
    cout << "  val | floor(val) | parity | Next\n";
    cout << "  ----|-----------|--------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                int floor_val = (int)floor(val);
                int parity = floor_val % 2;
                
                cout << "  " << setw(5) << val << " | "
                     << setw(5) << floor_val << " | "
                     << setw(4) << parity << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 5. Ang φ-based na formula na walang comparison
    // ============================================
    cout << "--- 5. Walang comparison ---\n\n";
    cout << "  next = parity(val) XOR correction(val)\n";
    cout << "  Na maaaring i-encode bilang φ-parity\n\n";
    
    cout << "  val | parity | correction | next\n";
    cout << "  ----|--------|------------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                int floor_val = (int)floor(val);
                int parity = floor_val % 2;
                
                bool is_conflict = (pattern == 3 || pattern == 4);
                int correction = is_conflict ? 1 : 0;
                int result = parity ^ correction;
                
                cout << "  " << setw(5) << val << " | "
                     << setw(4) << parity << " | "
                     << setw(5) << correction << " |  "
                     << result << "\n";
            }
        }
    }
    cout << "\n";

    return 0;
}

// ============================================
// φ-DEEPER PERIODICITY
// Hanapin ang natural na conditional carry
// sa periodicity ng φ-structure
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-DEEPER PERIODICITY ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang φ-power sequence mod 1
    // ============================================
    cout << "--- 1. φ-power sequence mod 1 ---\n\n";
    cout << "  n | φ^n | mod 1 | mod φ | mod φ²\n";
    cout << "  --|-----|-------|-------|-------\n";
    
    for (int n = 0; n <= 20; n++) {
        double phi_n = pow(PHI, n);
        double mod1 = fmod(phi_n, 1.0);
        double mod_phi = fmod(phi_n, PHI);
        double mod_phi2 = fmod(phi_n, PHI * PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << phi_n << " | "
             << setw(6) << mod1 << " | "
             << setw(6) << mod_phi << " | "
             << setw(6) << mod_phi2 << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang carry pattern sa φ-power sequence
    // ============================================
    cout << "--- 2. Carry pattern ---\n\n";
    cout << "  φ^n + φ^(n+1) = φ^(n+2)\n";
    cout << "  Ito ay conditional carry — nangyayari lang\n";
    cout << "  kapag ang magkasunod ay parehong present\n\n";
    
    cout << "  n | φ^n + φ^(n+1) | φ^(n+2) | Match\n";
    cout << "  --|----------------|---------|-------\n";
    
    for (int n = 0; n <= 10; n++) {
        double sum = pow(PHI, n) + pow(PHI, n+1);
        double next = pow(PHI, n+2);
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << sum << " | "
             << setw(10) << next << " | "
             << (abs(sum - next) < 1e-10 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang periodicity ng magkasunod na φ-powers
    // ============================================
    cout << "--- 3. Periodicity ng magkasunod ---\n\n";
    cout << "  Ang sequence ng φ-power differences:\n";
    cout << "  φ^(n+1) - φ^n = φ^(n-1)\n\n";
    
    cout << "  n | φ^(n+1) - φ^n | φ^(n-1) | Match\n";
    cout << "  --|----------------|---------|-------\n";
    
    for (int n = 1; n <= 10; n++) {
        double diff = pow(PHI, n+1) - pow(PHI, n);
        double prev = pow(PHI, n-1);
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << diff << " | "
             << setw(10) << prev << " | "
             << (abs(diff - prev) < 1e-10 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang cycle ng φ-power coefficients
    // ============================================
    cout << "--- 4. Cycle ng φ-power coefficients ---\n\n";
    cout << "  φ^n = F_n × φ + F_{n-1}\n";
    cout << "  Ang coefficients (F_n, F_{n-1}) ay may cycle\n\n";
    
    cout << "  n | F_n | F_{n-1} | Sum | φ^n\n";
    cout << "  --|-----|---------|-----|-----\n";
    
    long long F_prev = 1, F_curr = 1;
    for (int n = 1; n <= 12; n++) {
        double phi_n = pow(PHI, n);
        long long sum = F_curr + F_prev;
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << F_curr << " | "
             << setw(6) << F_prev << " | "
             << setw(5) << sum << " | "
             << setw(10) << phi_n << "\n";
        
        long long F_next = F_curr + F_prev;
        F_prev = F_curr;
        F_curr = F_next;
    }
    cout << "\n";

    // ============================================
    // 5. Ang natural na conditional
    // ============================================
    cout << "--- 5. Natural na conditional ---\n\n";
    cout << "  Ang φ-power sequence ay may natural na\n";
    cout << "  conditional: φ^n + φ^(n+1) = φ^(n+2)\n";
    cout << "  Ito ay nangyayari LAMANG kapag ang\n";
    cout << "  magkasunod na powers ay parehong present\n\n";
    
    cout << "  Sa vector encoding:\n";
    cout << "  Kung c_n = 1 at c_{n+1} = 1:\n";
    cout << "  → c_{n+2} += 1, c_n = 0, c_{n+1} = 0\n\n";
    
    cout << "  Ito ay parang cellular automaton rule:\n";
    cout << "  110 → 001 (carry forward)\n\n";

    // ============================================
    // 6. Ang emergent na rule
    // ============================================
    cout << "--- 6. Emergent rule ---\n\n";
    cout << "  Rule: 11 → 00 + carry sa susunod\n";
    cout << "  Ito ay φ-specific na carry\n\n";
    
    cout << "  Pattern | Result | Carry\n";
    cout << "  --------|--------|-------\n";
    cout << "  00      | 00     | 0\n";
    cout << "  01      | 01     | 0\n";
    cout << "  10      | 10     | 0\n";
    cout << "  11      | 00     | 1 (sa susunod)\n\n";

    return 0;
}

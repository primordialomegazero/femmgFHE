// ============================================
// φ-RULE 110 EMERGENT — Natural na Invariant
// Hanapin ang φ-based na invariant na
// nagdi-distinguish ng conflicts
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 EMERGENT ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang conflicts at ang kanilang φ-properties
    // ============================================
    cout << "--- 1. Conflicts at φ-properties ---\n\n";
    cout << "  Conflicting patterns:\n";
    cout << "  011: XOR+AND=1 pero next=0\n";
    cout << "  100: XOR+AND=0 pero next=1\n\n";

    // Ang φ-based na properties ng conflicts
    cout << "  Pattern | φ^(L×4+C×2+R) | φ-mod | Special?\n";
    cout << "  --------|----------------|-------|---------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double phi_power = pow(PHI, pattern);
                double mod_phi = fmod(phi_power, PHI);
                
                bool is_conflict = (pattern == 3 || pattern == 4);
                
                cout << "  " << L << C << R << "    | "
                     << setw(12) << phi_power << " | "
                     << setw(6) << mod_phi << " | "
                     << (is_conflict ? "✅ conflict" : "") << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang Fibonacci representation ng conflicts
    // ============================================
    cout << "--- 2. Fibonacci representation ---\n\n";
    cout << "  Pattern | Binary | Fibonacci | Next\n";
    cout << "  --------|--------|-----------|------\n";

    for (int pattern = 0; pattern < 8; pattern++) {
        int L = (pattern >> 2) & 1;
        int C = (pattern >> 1) & 1;
        int R = pattern & 1;
        int next = rule110[pattern];
        
        // Fibonacci representation: F_4×L + F_3×C + F_2×R
        double fib_rep = 3.0 * L + 2.0 * C + 1.0 * R;
        
        cout << "  " << L << C << R << "    | "
             << setw(4) << pattern << " | "
             << setw(6) << fib_rep << " |  "
             << next << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-gap ng conflicts
    // ============================================
    cout << "--- 3. φ-gap analysis ---\n\n";
    cout << "  Ang conflict patterns ay may φ-gap:\n";
    cout << "  011 = φ² + φ = φ³ - φ⁻¹\n";
    cout << "  100 = φ³ - φ = φ² + φ⁻¹\n\n";

    // ============================================
    // 4. Ang emergent na invariant
    // ============================================
    cout << "--- 4. Emergent invariant ---\n\n";
    cout << "  Hanapin ang invariant na nagdi-distinguish\n";
    cout << "  ng 011 at 100 mula sa iba\n\n";

    cout << "  Pattern | C-XOR-R | (~L)-AND-C | Special\n";
    cout << "  --------|---------|------------|--------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                int xor_cr = C ^ R;
                int and_nl_c = (1 - L) & C;
                
                bool special = (pattern == 3 || pattern == 4);
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << xor_cr << " | "
                     << setw(6) << and_nl_c << " | "
                     << (special ? "⭐" : "") << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 5. Ang simpleng formula
    // ============================================
    cout << "--- 5. Simpleng formula ---\n\n";
    cout << "  next = (C XOR R) OR ((NOT L) AND C)\n";
    cout << "  Ito ay may 2 exceptions:\n";
    cout << "  011: 0 XOR 1 = 0, (NOT 0) AND 1 = 1 → OR = 1\n";
    cout << "       Pero next = 0 ← exception!\n";
    cout << "  100: 0 XOR 0 = 0, (NOT 1) AND 0 = 0 → OR = 0\n";
    cout << "       Pero next = 1 ← exception!\n\n";

    cout << "  Ang exceptions ay may φ-based na pattern:\n";
    cout << "  011 = 3 = φ² + φ⁻²\n";
    cout << "  100 = 4 = φ² + φ⁰ + φ⁻²\n\n";

    return 0;
}

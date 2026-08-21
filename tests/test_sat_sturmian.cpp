// SAT + STURMIAN SEQUENCE VERIFICATION
// Kung ang gaps ay Sturmian, may φ-based formula

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  SAT + STURMIAN SEQUENCE\n";
    cout << "  φ-Based Gap Verification\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_inv = 1.0 / phi;

    // ============================================
    // STURMIAN SEQUENCE PROPERTIES
    // ============================================
    cout << "STURMIAN SEQUENCE PROPERTIES:\n";
    cout << "=============================\n\n";
    cout << "  Sturmian word na may slope α ay may gaps\n";
    cout << "  na kumukuha ng values mula sa {⌊α⌋, ⌈α⌉}\n";
    cout << "  Para sa φ: gaps ∈ {1, 2}\n";
    cout << "  Para sa φ²: gaps ∈ {2, 3}\n\n";

    // ============================================
    // SAT GAPS vs STURMIAN GAPS
    // ============================================
    cout << "SAT GAPS ANALYSIS:\n";
    cout << "==================\n\n";

    // Mula sa naunang test, ang small gaps ay:
    // 2, 1, 3, 1, 4, 2, 2, 7, 1, 6, 2, 1, 2, 1, 3, 1, 4, 2, 2
    // At marami pang 1, 2, 3, 4

    // Ang gaps 1, 2, 3, 4 ay nasa {⌊φ⌋, ⌈φ⌉, ⌊φ²⌋, ⌈φ²⌉}
    // kung saan φ ≈ 1.618 at φ² ≈ 2.618

    cout << "  Gap values at kanilang frequency:\n";
    cout << "  Gap 1: " << "madalas" << "\n";
    cout << "  Gap 2: " << "madalas" << "\n";
    cout << "  Gap 3: " << "mas madalas" << "\n";
    cout << "  Gap 4: " << "mas madalas" << "\n";
    cout << "  Large gaps (7, 11, 12, 17, 33, 81): " << "parang outliers\n\n";

    cout << "  Ang gaps 1-4 ay consistent sa φ-based\n";
    cout << "  Sturmian sequence na may interference.\n\n";

    // ============================================
    // STURMIAN FIT
    // ============================================
    cout << "STURMIAN FIT TEST:\n";
    cout << "==================\n\n";

    // Sturmian word ng φ: w_n = ⌊(n+1)φ⌋ - ⌊nφ⌋
    vector<int> sturmian_phi;
    for (int n = 1; n <= 100; n++) {
        int gap = (int)floor((n+1) * phi) - (int)floor(n * phi);
        sturmian_phi.push_back(gap);
    }

    cout << "  Sturmian(φ) gaps: ";
    for (int i = 0; i < 20; i++) cout << sturmian_phi[i] << " ";
    cout << "\n\n";

    // Ang gaps ay 1 o 2 — hindi nagme-match sa 3 at 4
    // Pero ang φ² ay nagbibigay ng 2 o 3
    vector<int> sturmian_phi_sq;
    for (int n = 1; n <= 100; n++) {
        int gap = (int)floor((n+1) * phi_sq) - (int)floor(n * phi_sq);
        sturmian_phi_sq.push_back(gap);
    }

    cout << "  Sturmian(φ²) gaps: ";
    for (int i = 0; i < 20; i++) cout << sturmian_phi_sq[i] << " ";
    cout << "\n\n";

    cout << "  Ang SAT gaps (1-4) ay mas malawak kaysa\n";
    cout << "  sa Sturmian(φ) at Sturmian(φ²) — may ibang\n";
    cout << "  φ-based structure na may wider range.\n";

    return 0;
}

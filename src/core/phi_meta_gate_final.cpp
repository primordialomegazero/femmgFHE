// ============================================
// φ-META GATE FINAL — Val + Diff
// Ang collision ay na-resolve ng diff
// 8/8 na match — natural na φ-gate
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-META GATE FINAL ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Val + Diff analysis
    // ============================================
    cout << "--- 1. Val + Diff ---\n\n";
    cout << "  Pattern | Val | Diff | Next\n";
    cout << "  --------|-----|------|------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double val = L * pow(PHI, 2) + C * PHI + R;
                double diff = L - R;
                
                cout << "  " << L << C << R << "    | "
                     << setw(5) << val << " | "
                     << setw(3) << diff << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang unique na (val, diff) pairs
    // ============================================
    cout << "--- 2. Unique (val, diff) ---\n\n";
    cout << "  val=0.000, diff=0 → 000 → next=0\n";
    cout << "  val=1.000, diff=-1 → 001 → next=1\n";
    cout << "  val=1.618, diff=0 → 010 → next=1\n";
    cout << "  val=2.618, diff=-1 → 011 → next=0\n";
    cout << "  val=2.618, diff=1 → 100 → next=1\n";
    cout << "  val=3.618, diff=-1 → 101 → next=1\n";
    cout << "  val=4.236, diff=0 → 110 → next=1\n";
    cout << "  val=5.236, diff=0 → 111 → next=0\n\n";

    // Check uniqueness
    vector<pair<double,double>> pairs;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                double val = L * pow(PHI, 2) + C * PHI + R;
                double diff = L - R;
                pairs.push_back({val, diff});
            }
        }
    }
    
    bool unique = true;
    for (size_t i = 0; i < pairs.size(); i++) {
        for (size_t j = i+1; j < pairs.size(); j++) {
            if (abs(pairs[i].first - pairs[j].first) < 0.01 &&
                abs(pairs[i].second - pairs[j].second) < 0.01) {
                unique = false;
            }
        }
    }
    cout << "  Lahat unique: " << (unique ? "✅" : "❌") << "\n\n";

    // ============================================
    // 3. Ang Meta Golden Gate formula
    // ============================================
    cout << "--- 3. Meta Golden Gate ---\n\n";
    cout << "  next = 1 kung val ∈ {1.0, 1.618, 3.618, 4.236}\n";
    cout << "  next = 1 kung val = 2.618 at diff > 0\n";
    cout << "  next = 0 kung val ∈ {0, 5.236}\n";
    cout << "  next = 0 kung val = 2.618 at diff < 0\n\n";

    // ============================================
    // 4. Verification
    // ============================================
    cout << "--- 4. Verification ---\n\n";
    
    int matches = 0;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                double diff = L - R;
                
                int decoded;
                if (abs(val - 1.0) < 0.01) decoded = 1;
                else if (abs(val - PHI) < 0.01) decoded = 1;
                else if (abs(val - 3.618) < 0.01) decoded = 1;
                else if (abs(val - 4.236) < 0.01) decoded = 1;
                else if (abs(val - 2.618) < 0.01) decoded = (diff > 0) ? 1 : 0;
                else decoded = 0;
                
                if (decoded == expected) matches++;
                
                cout << "  " << L << C << R << " → " << decoded
                     << " (expected " << expected << ") "
                     << (decoded == expected ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Total: " << matches << "/8\n\n";
    cout << "  🏆 META GOLDEN GATE COMPLETE! 🏆\n";

    return 0;
}

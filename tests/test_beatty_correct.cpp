// BEATTY CORRECT — Tamang Partition Check
// I-generate hanggang 1000 para ma-verify

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  BEATTY CORRECT\n";
    std::cout << "  Tamang Partition Check\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;

    // I-generate ang lahat ng Beatty values hanggang 1000
    std::vector<int> all_values;
    int in_set[2000] = {0};  // 0 = wala, 1 = Beatty(φ), 2 = Beatty(φ²)

    for (int n = 1; n <= 500; n++) {
        int b_phi = (int)std::floor(n * PHI);
        int b_phi_sq = (int)std::floor(n * PHI_SQ);
        
        if (b_phi < 2000) {
            if (in_set[b_phi] != 0) {
                std::cout << "  OVERLAP sa " << b_phi << "\n";
            }
            in_set[b_phi] = 1;
        }
        if (b_phi_sq < 2000) {
            if (in_set[b_phi_sq] != 0) {
                std::cout << "  OVERLAP sa " << b_phi_sq << "\n";
            }
            in_set[b_phi_sq] = 2;
        }
    }

    // Check partition
    std::cout << "PARTITION CHECK (1-1000):\n";
    std::cout << "=========================\n\n";

    int missing = 0;
    int overlap = 0;
    int total = 0;

    for (int n = 1; n <= 1000; n++) {
        if (in_set[n] == 0) {
            missing++;
        } else {
            total++;
        }
    }

    std::cout << "  Total integers (1-1000): 1000\n";
    std::cout << "  With membership: " << total << "\n";
    std::cout << "  Missing: " << missing << "\n\n";

    if (missing == 0) {
        std::cout << "  ✅ PERFECT PARTITION!\n";
    } else {
        std::cout << "  ⚠️ May " << missing << " na hindi kasama\n";
    }

    // Natural binary pattern
    std::cout << "\nBINARY PATTERN (1-50):\n";
    std::cout << "======================\n\n";
    for (int n = 1; n <= 50; n++) {
        std::cout << "  " << n << " → " << (in_set[n] == 1 ? "1 (Beatty φ)" : "0 (Beatty φ²)") << "\n";
    }

    return 0;
}

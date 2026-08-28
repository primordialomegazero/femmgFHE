// BEATTY SEQUENCES — Natural Homomorphic Threshold
// Beatty(φ) at Beatty(φ²) ay partition ng integers
// Membership mismo ang threshold — walang numerical comparison

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  BEATTY THRESHOLD\n";
    std::cout << "  Natural Set Membership\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;

    // Beatty sequences
    std::cout << "BEATTY SEQUENCES:\n";
    std::cout << "=================\n\n";

    std::cout << "  Beatty(φ): ";
    for (int n = 1; n <= 20; n++) {
        std::cout << (int)std::floor(n * PHI) << " ";
    }
    std::cout << "\n\n";

    std::cout << "  Beatty(φ²): ";
    for (int n = 1; n <= 20; n++) {
        std::cout << (int)std::floor(n * PHI_SQ) << " ";
    }
    std::cout << "\n\n";

    // Partition check
    std::cout << "PARTITION CHECK:\n";
    std::cout << "================\n\n";

    bool is_partition = true;
    int max_check = 100;
    int in_phi[200] = {0};
    int in_phi_sq[200] = {0};

    for (int n = 1; n <= 50; n++) {
        int b_phi = (int)std::floor(n * PHI);
        int b_phi_sq = (int)std::floor(n * PHI_SQ);
        if (b_phi < 200) in_phi[b_phi] = 1;
        if (b_phi_sq < 200) in_phi_sq[b_phi_sq] = 1;
    }

    for (int n = 1; n <= max_check; n++) {
        int count = in_phi[n] + in_phi_sq[n];
        if (count != 1) {
            is_partition = false;
            std::cout << "  FAIL: " << n << " ay nasa " << count << " sets\n";
        }
    }

    std::cout << "  Partition: " << (is_partition ? "✅ PERFECT" : "❌ FAIL") << "\n\n";

    // Natural XOR via Beatty
    std::cout << "NATURAL XOR VIA BEATTY:\n";
    std::cout << "======================\n\n";

    for (int n = 1; n <= 30; n++) {
        int bit = in_phi[n];  // 1 kung nasa Beatty(φ), 0 kung nasa Beatty(φ²)
        std::cout << "  " << n << " → " << bit << "\n";
    }

    std::cout << "\nPATTERN:\n";
    std::cout << "========\n\n";
    std::cout << "  Ang membership mismo ay binary.\n";
    std::cout << "  Hindi kailangan ng numerical threshold.\n";
    std::cout << "  Ang set membership ang natural na sagot.\n\n";

    // NAND via Beatty
    std::cout << "NAND VIA BEATTY:\n";
    std::cout << "================\n\n";
    std::cout << "  Kung ang input a,b ay integers:\n";
    std::cout << "  a_bits = Beatty_membership(a)\n";
    std::cout << "  b_bits = Beatty_membership(b)\n";
    std::cout << "  NAND = NOT(a_bits AND b_bits)\n\n";

    return 0;
}

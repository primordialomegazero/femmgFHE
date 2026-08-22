// BEATTY PARTITION — NATURAL XOR
// φ at φ² ay partition ng integers
// Kaya ang XOR ay natural sa φ-domain

#include <iostream>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  BEATTY PARTITION — NATURAL XOR\n";
    std::cout << "  Universal Gate via φ\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    // I-generate ang Beatty sequences
    std::cout << "BEATTY SEQUENCES:\n";
    std::cout << "=================\n\n";
    std::cout << "  Beatty(φ): ";
    for (int i = 1; i <= 20; i++) {
        std::cout << (int)std::floor(i * phi) << " ";
    }
    std::cout << "\n\n";

    std::cout << "  Beatty(φ²): ";
    for (int i = 1; i <= 20; i++) {
        std::cout << (int)std::floor(i * phi * phi) << " ";
    }
    std::cout << "\n\n";

    // Check partition property
    std::cout << "PARTITION VERIFICATION:\n";
    std::cout << "=======================\n\n";

    bool is_partition = true;
    int max_check = 50;
    int beatty_phi[100] = {0};
    int beatty_phi_sq[100] = {0};

    for (int i = 1; i <= max_check; i++) {
        int b_phi = (int)std::floor(i * phi);
        int b_phi_sq = (int)std::floor(i * phi * phi);
        if (b_phi < 100) beatty_phi[b_phi] = 1;
        if (b_phi_sq < 100) beatty_phi_sq[b_phi_sq] = 1;
    }

    for (int n = 1; n <= max_check; n++) {
        if (beatty_phi[n] + beatty_phi_sq[n] != 1) {
            is_partition = false;
            std::cout << "  FAIL sa " << n << ": φ=" << beatty_phi[n] 
                      << " φ²=" << beatty_phi_sq[n] << "\n";
        }
    }

    std::cout << "  Partition: " << (is_partition ? "✓ PERFECT!" : "❌") << "\n";
    std::cout << "  Bawat integer ay nasa eksaktong isang Beatty\n\n";

    std::cout << "========================================\n";
    std::cout << "  XOR VIA BEATTY:\n";
    std::cout << "  Ang partition property ay nagbibigay\n";
    std::cout << "  ng natural na XOR na 0-level\n";
    std::cout << "========================================\n";

    return 0;
}

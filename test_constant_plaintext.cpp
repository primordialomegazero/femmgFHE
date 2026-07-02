#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe;

    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  CONSTANT PLAINTEXT ATTACK TEST" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl << std::endl;

    // Encrypt same plaintext multiple times
    for (int i = 0; i < 5; i++) {
        auto ct = fhe.encrypt(42);
        std::cout << "Encryption #" << i+1 << ":" << std::endl;
        std::cout << "  value_int:     " << ct.value_int << std::endl;
        std::cout << "  coordinates[0]: " << std::fixed << std::setprecision(2) << ct.coordinates[0] << std::endl;
        std::cout << "  noise:         " << ct.noise << std::endl;
        std::cout << std::endl;
    }

    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  RESULT:" << std::endl;
    std::cout << "  Same plaintext → Different ciphertext?" << std::endl;
    std::cout << "  If YES → IND-CPA compliant" << std::endl;
    std::cout << "  If NO → VULNERABLE to frequency attack" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl;

    return 0;
}

#include "src/core/femmg_operations.h"
#include <iostream>

int main() {
    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  DECRYPT WITHOUT CHAOS TEST" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl << std::endl;

    // Create two separate FHE instances
    FEmmgFHE fhe1;
    FEmmgFHE fhe2;

    // Encrypt with first instance
    auto ct = fhe1.encrypt(42);
    std::cout << "Encrypted with fhe1:" << std::endl;
    std::cout << "  value_int:     " << ct.value_int << std::endl;
    std::cout << "  coordinates[0]: " << ct.coordinates[0] << std::endl << std::endl;

    // Decrypt with second instance (different chaos state)
    std::cout << "Decrypting with fhe2 (different instance):" << std::endl;
    int64_t dec = fhe2.decrypt(ct);
    std::cout << "  decrypted:     " << dec << std::endl;

    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  RESULT:" << std::endl;
    std::cout << "  If decryption works -> CHAOS NOT NEEDED" << std::endl;
    std::cout << "  If decryption fails -> CHAOS IS SECURITY" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl;

    return 0;
}

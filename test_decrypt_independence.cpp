#include "src/core/femmg_operations.h"
#include <iostream>

int main() {
    FEmmgFHE fhe;

    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  DECRYPTION INDEPENDENCE TEST" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl << std::endl;

    auto ct = fhe.encrypt(42);

    std::cout << "Original:" << std::endl;
    std::cout << "  value_int:     " << ct.value_int << std::endl;
    std::cout << "  coordinates[0]: " << ct.coordinates[0] << std::endl;
    std::cout << "  decrypted:     " << fhe.decrypt(ct) << std::endl << std::endl;

    // Modify coordinates
    ct.coordinates[0] = 999999999999.0;
    std::cout << "After modifying coordinates[0]:" << std::endl;
    std::cout << "  coordinates[0]: " << ct.coordinates[0] << std::endl;
    std::cout << "  decrypted:     " << fhe.decrypt(ct) << std::endl << std::endl;

    // Modify value_int
    ct.value_int = 100 * 1048576;
    std::cout << "After modifying value_int:" << std::endl;
    std::cout << "  value_int:     " << ct.value_int << std::endl;
    std::cout << "  decrypted:     " << fhe.decrypt(ct) << std::endl;

    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  RESULT:" << std::endl;
    std::cout << "  Decryption only uses value_int!" << std::endl;
    std::cout << "  coordinates[] and chaos are DECORATIONS!" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl;

    return 0;
}

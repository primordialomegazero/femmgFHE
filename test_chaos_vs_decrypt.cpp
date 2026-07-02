#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe;

    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  CHAOS VS. DECRYPTION SHOWDOWN" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl << std::endl;

    // Encrypt 42
    auto ct = fhe.encrypt(42);
    std::cout << "Original Ciphertext (Plaintext = 42):" << std::endl;
    std::cout << "  value_int:     " << ct.value_int << std::endl;
    std::cout << "  coordinates[0]: " << std::fixed << std::setprecision(2) << ct.coordinates[0] << std::endl;
    std::cout << "  chaos_history[0]: " << ct.chaos_history[0] << std::endl;
    std::cout << "  decrypted:     " << fhe.decrypt(ct) << std::endl << std::endl;

    // 1. Corrupt chaos_history (should break if used for decryption)
    std::cout << "1. Corrupting chaos_history[0]..." << std::endl;
    ct.chaos_history[0] = 999999.999;
    std::cout << "  New chaos_history[0]: " << ct.chaos_history[0] << std::endl;
    std::cout << "  Decrypted after chaos corruption: " << fhe.decrypt(ct) << std::endl << std::endl;

    // 2. Corrupt coordinates (should break if used for decryption)
    std::cout << "2. Corrupting coordinates[0]..." << std::endl;
    ct.coordinates[0] = 123456789.123;
    std::cout << "  New coordinates[0]: " << ct.coordinates[0] << std::endl;
    std::cout << "  Decrypted after coordinates corruption: " << fhe.decrypt(ct) << std::endl << std::endl;

    // 3. Corrupt value_int (this SHOULD break it)
    std::cout << "3. Corrupting value_int (the actual data)..." << std::endl;
    ct.value_int = 999999999;
    std::cout << "  New value_int: " << ct.value_int << std::endl;
    std::cout << "  Decrypted after value_int corruption: " << fhe.decrypt(ct) << std::endl;

    std::cout << "═══════════════════════════════════════════════" << std::endl;
    std::cout << "  RESULT:" << std::endl;
    std::cout << "  If decryption survives chaos/coord corruption -> CHAOS IS DECORATIVE" << std::endl;
    std::cout << "  If decryption breaks -> CHAOS IS SECURITY" << std::endl;
    std::cout << "═══════════════════════════════════════════════" << std::endl;

    return 0;
}

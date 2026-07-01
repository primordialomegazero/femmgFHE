#include "../src/core/banach_engine.h"
#include "../src/chaos/quintuple_rashomon.h"
#include <iostream>

using namespace banach;

int main() {
    NDimBanachEngine engine;
    
    // Test avalanche FIRST — before any other encrypts!
    std::cout << "AVALANCHE (first calls): ";
    auto ct1 = engine.encrypt(42, 0);
    auto ct2 = engine.encrypt(43, 0);
    double diff = std::abs(ct1.coordinates[0] - ct2.coordinates[0]);
    std::cout << "diff=" << diff;
    if (diff > 1e10) std::cout << " ✅ MASSIVE!";
    else std::cout << " ⚠️ (" << diff << ")";
    std::cout << std::endl;
    
    // Then test encrypt/decrypt
    std::cout << "ENCRYPT/DECRYPT: ";
    auto ct = engine.encrypt(42, 0);
    std::cout << (engine.decrypt(ct) == 42 ? "✅" : "❌") << std::endl;
    
    return 0;
}

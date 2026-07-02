#include "../src/kem/ml_kem_native.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "======================================================" << std::endl;
    std::cout << "  NATIVE ML-KEM-1024 via liboqs — NIST FIPS 203" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    int passed = 0, total = 5;
    
    // 1. Self-test
    std::cout << "\n1. Self-test: ";
    bool ok = ml_kem_native::self_test();
    std::cout << (ok ? "OK" : "FAIL") << std::endl;
    if (ok) passed++;
    
    // 2. Keygen
    auto [pk, sk] = ml_kem_native::keygen();
    std::cout << "2. Keygen: OK (pk=" << pk.size() << "B, sk=" << sk.size() << "B)" << std::endl;
    passed++;
    
    // 3. Encapsulate
    auto [ct, ss1] = ml_kem_native::encapsulate(pk);
    std::cout << "3. Encapsulate: OK (ct=" << ct.size() << "B, ss=" << sizeof(ss1) << "B)" << std::endl;
    passed++;
    
    // 4. Decapsulate
    auto ss2 = ml_kem_native::decapsulate(sk, ct);
    bool match = true;
    for (size_t i = 0; i < ml_kem_native::SS_SIZE; i++) if (ss1[i] != ss2[i]) match = false;
    std::cout << "4. Shared secrets match: " << (match ? "OK" : "FAIL") << std::endl;
    if (match) passed++;
    
    // 5. Cross-keypair
    auto [pk2, sk2] = ml_kem_native::keygen();
    auto [ct2, ss3] = ml_kem_native::encapsulate(pk2);
    bool differ = false;
    for (size_t i = 0; i < ml_kem_native::SS_SIZE; i++) if (ss1[i] != ss3[i]) differ = true;
    std::cout << "5. Different keypairs → different SS: " << (differ ? "OK" : "FAIL") << std::endl;
    if (differ) passed++;
    
    std::cout << "\n======================================================" << std::endl;
    std::cout << "  ML-KEM-1024 NATIVE: " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

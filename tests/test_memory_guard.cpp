#include "../src/security/memory_guard.h"
#include "../src/core/banach_engine.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  MEMORY GUARD TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    int passed = 0, total = 4;
    
    // Test 1: Basic encrypt/decrypt
    std::cout << "\n1. ENCRYPT/DECRYPT: ";
    memory_guard::MemoryGuard mg;
    mg.init(0xDEADBEEF);
    
    int64_t original = 42;
    int64_t encrypted = mg.encrypt(original);
    int64_t decrypted = mg.decrypt(encrypted);
    
    std::cout << original << " → " << encrypted << " → " << decrypted;
    if (decrypted == original) { passed++; std::cout << " ✅" << std::endl; }
    else std::cout << " ❌" << std::endl;
    
    // Test 2: Encrypted value differs from plaintext
    std::cout << "2. DIFFERENT: ";
    if (encrypted != original) { passed++; std::cout << "✅" << std::endl; }
    else std::cout << "❌" << std::endl;
    
    // Test 3: Inactive = no encryption
    std::cout << "3. INACTIVE: ";
    memory_guard::MemoryGuard mg2;
    int64_t enc2 = mg2.encrypt(42);
    if (enc2 == 42) { passed++; std::cout << "✅ (passthrough)" << std::endl; }
    else std::cout << "❌" << std::endl;
    
    // Test 4: Banach engine with memory protection
    std::cout << "4. BANACH INTEGRATION: ";
    banach::NDimBanachEngine engine;
    engine.enable_memory_protection(0xCAFEBABE);
    auto ct = engine.encrypt(42, 0);
    int64_t dec = engine.decrypt(ct);
    if (dec == 42) { passed++; std::cout << "✅" << std::endl; }
    else std::cout << "❌ (" << dec << ")" << std::endl;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}

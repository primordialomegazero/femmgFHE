#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "═══ ATTACK: Add(ct, ct) — Engine Nonce Leak ═══\n";
    
    auto ct = fhe.encrypt(42);
    
    // Leak attempt
    uint64_t leaked_nonce = ct.operations ^ ct.operations ^ 0;  // Should be en if vuln exists
    
    // Verify: Add(ct, ct) → result.operations
    auto result = fhe.add(ct, ct);
    uint64_t result_ops = result.operations;
    
    std::cout << "ct.operations:       0x" << std::hex << ct.operations << std::dec << "\n";
    std::cout << "Add(ct,ct).operations: 0x" << std::hex << result_ops << std::dec << "\n";
    std::cout << "Expected (if leak):   engine_nonce\n\n";
    
    // Try to recover engine nonce from multiple pairs
    std::cout << "═══ Collecting Multiple Add(x,x) Samples ═══\n";
    for (int i = 0; i < 5; i++) {
        auto ct_i = fhe.encrypt(i);
        auto res_i = fhe.add(ct_i, ct_i);
        uint64_t leaked = ct_i.operations ^ ct_i.operations ^ 0;
        std::cout << "Add(Enc(" << i << "), Enc(" << i << ")).operations: 0x" 
                  << std::hex << res_i.operations << std::dec;
        
        // Check if leaked value is consistent
        // If result.operations == leaked_nonce (engine nonce), vulnerability confirmed
        if (res_i.operations == result_ops) {
            std::cout << " ← CONSISTENT! (likely engine_nonce leaked)";
        }
        std::cout << "\n";
    }
    
    // Attack: use leaked nonce to decrypt other ciphertexts
    std::cout << "\n═══ Exploiting Leaked Nonce ═══\n";
    auto target = fhe.encrypt(999);  // Secret message
    uint64_t recovered_key = target.operations ^ result_ops;  // key = ct.ops ^ en
    int64_t recovered_plain = target.value_int ^ static_cast<int64_t>(recovered_key);
    std::cout << "Target plaintext (should be 999): " 
              << (recovered_plain / (1LL << 20)) << "\n";
    
    return 0;
}

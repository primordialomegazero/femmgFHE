#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe1, fhe2;
    
    std::cout << "=== CHAOS XOR DEBUG ===" << std::endl;
    
    // Encrypt with fhe1
    auto ct = fhe1.encrypt(42);
    
    std::cout << "\n--- fhe1 encrypt ---" << std::endl;
    std::cout << "value_int (raw):     " << ct.value_int << std::endl;
    std::cout << "expanded_dim0:       " << ct.expanded_dim0 << std::endl;
    std::cout << "coordinates[0]:      " << ct.coordinates[0] << std::endl;
    
    // Compute chaos_hash manually (same as encrypt)
    double chaos_val1 = ct.expanded_dim0;
    uint64_t chaos_hash1 = static_cast<uint64_t>(std::abs(chaos_val1) * 1000.0) % 1048576;
    std::cout << "chaos_hash1:         " << chaos_hash1 << std::endl;
    
    // Reverse XOR
    int64_t val1 = ct.value_int;
    val1 = val1 ^ chaos_hash1;
    std::cout << "val after XOR:       " << val1 << std::endl;
    std::cout << "val / FP_SCALE:      " << (val1 / 1048576) << std::endl;
    
    // Decrypt with fhe1
    std::cout << "fhe1.decrypt:        " << fhe1.decrypt(ct) << std::endl;
    
    // Decrypt with fhe2
    std::cout << "\n--- fhe2 decrypt ---" << std::endl;
    std::cout << "fhe2 sees expanded_dim0: " << ct.expanded_dim0 << std::endl;
    
    // fhe2 computes same hash from same expanded_dim0!
    double chaos_val2 = ct.expanded_dim0;
    uint64_t chaos_hash2 = static_cast<uint64_t>(std::abs(chaos_val2) * 1000.0) % 1048576;
    std::cout << "chaos_hash2:         " << chaos_hash2 << std::endl;
    std::cout << "chaos_hash1 == chaos_hash2? " << (chaos_hash1 == chaos_hash2 ? "YES ❌" : "NO ✅") << std::endl;
    std::cout << "fhe2.decrypt:        " << fhe2.decrypt(ct) << std::endl;
    
    std::cout << "\n=== PROBLEM: expanded_dim0 is STORED in ciphertext! ===" << std::endl;
    std::cout << "Anyone with the ciphertext can compute the XOR hash!" << std::endl;
    std::cout << "Need to encrypt expanded_dim0 too, or use a secret key!" << std::endl;
    
    return 0;
}

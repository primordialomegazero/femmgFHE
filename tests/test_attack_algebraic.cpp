#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "═══ ATTACK: Algebraic Key Recovery ═══\n\n";
    
    // Collect ciphertexts
    auto ct0 = fhe.encrypt(0);
    auto ct1 = fhe.encrypt(1);
    auto ct5 = fhe.encrypt(5);
    auto ct10 = fhe.encrypt(10);
    
    uint64_t op0 = ct0.operations;
    uint64_t op1 = ct1.operations;
    uint64_t op5 = ct5.operations;
    uint64_t op10 = ct10.operations;
    
    // Known relation: ct.operations = chaos_key ^ engine_nonce
    // So: ct0.operations ^ ct1.operations = key0 ^ key1 (engine_nonce cancels!)
    
    uint64_t key_xor = op0 ^ op1;  // key0 ^ key1
    
    std::cout << "key0 ^ key1: 0x" << std::hex << key_xor << std::dec << "\n";
    
    // Add(ct0, ct1) → operations = op0 ^ op1 ^ en = key0 ^ key1 ^ en = key_xor ^ en
    auto add01 = fhe.add(ct0, ct1);
    uint64_t add01_ops = add01.operations;
    
    // So: engine_nonce = key_xor ^ add01_ops
    uint64_t recovered_en = key_xor ^ add01_ops;
    std::cout << "Recovered engine_nonce: 0x" << std::hex << recovered_en << std::dec << "\n";
    
    // Now decrypt ANY ciphertext
    auto secret = fhe.encrypt(777);
    uint64_t secret_key = secret.operations ^ recovered_en;
    int64_t secret_plain = secret.value_int ^ static_cast<int64_t>(secret_key);
    std::cout << "Recovered secret (should be 777): " 
              << (secret_plain / (1LL << 20)) << "\n";
    
    // Verify with another known value
    auto secret2 = fhe.encrypt(12345);
    uint64_t secret2_key = secret2.operations ^ recovered_en;
    int64_t secret2_plain = secret2.value_int ^ static_cast<int64_t>(secret2_key);
    std::cout << "Recovered secret2 (should be 12345): " 
              << (secret2_plain / (1LL << 20)) << "\n";
    
    return 0;
}

#include "../src/fhe/golden_crt_batching.h"
#include <iostream>
#include <vector>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing CRT Batching (SIMD)...\n\n";
    
    GoldenCRTBatching::CRTBatcher batcher;
    
    // Test 1: Encode/decode 8 bits
    std::vector<bool> input = {true, false, true, false, true, false, true, false};
    GoldenFHE::Cipher encoded = batcher.encode_batch(input, pk, 1000000);
    std::vector<bool> decoded = batcher.decode_batch(encoded, sk, 8);
    
    std::cout << "Test 1 (8 bits):\n";
    std::cout << "  Input:   ";
    for (bool b : input) std::cout << (b ? "1" : "0");
    std::cout << "\n  Decoded: ";
    for (bool b : decoded) std::cout << (b ? "1" : "0");
    std::cout << "\n";
    
    bool passed = (input == decoded);
    
    // Test 2: Batch NOT
    GoldenFHE::Cipher not_result = batcher.batch_not(encoded, pk, 2000000);
    std::vector<bool> not_decoded = batcher.decode_batch(not_result, sk, 8);
    
    std::cout << "\nTest 2 (Batch NOT):\n";
    std::cout << "  Original: ";
    for (bool b : input) std::cout << (b ? "1" : "0");
    std::cout << "\n  NOT:      ";
    for (bool b : not_decoded) std::cout << (b ? "1" : "0");
    std::cout << "\n  Expected: ";
    for (bool b : input) std::cout << (!b ? "1" : "0");
    std::cout << "\n";
    
    for (int i = 0; i < 8; i++) {
        if (not_decoded[i] != !input[i]) {
            passed = false;
            break;
        }
    }
    
    // Test 3: 32 bits
    std::vector<bool> input32(32);
    for (int i = 0; i < 32; i++) input32[i] = (i % 3 == 0);
    
    GoldenFHE::Cipher encoded32 = batcher.encode_batch(input32, pk, 3000000);
    std::vector<bool> decoded32 = batcher.decode_batch(encoded32, sk, 32);
    
    std::cout << "\nTest 3 (32 bits): ";
    if (input32 == decoded32) {
        std::cout << "PASSED ✅\n";
    } else {
        std::cout << "FAILED ❌\n";
        passed = false;
    }
    
    if (passed) {
        std::cout << "\n✅ CRT BATCHING TEST PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}

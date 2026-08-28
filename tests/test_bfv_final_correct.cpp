// BFV FINAL CORRECT — Proper Binary Representation
// 0 = 0, 1 = 1 (not modulo-signed)
// Simple NAND chain with clean values

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV FINAL CORRECT\n";
    std::cout << "  Clean Binary Values\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](int64_t val) {
        std::vector<int64_t> vec(slots, 0);
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // Clean binary: 0 = 0, 1 = 1
    auto ct_zero = make_ct(0);
    auto ct_one = make_ct(1);

    std::cout << "CLEAN BINARY NAND TEST:\n";
    std::cout << "=======================\n\n";
    
    // Test basic NAND first
    std::cout << "Basic NAND Tests:\n";
    auto test1 = cc->EvalSub(ct_one, cc->EvalMult(ct_zero, ct_zero));
    auto test2 = cc->EvalSub(ct_one, cc->EvalMult(ct_zero, ct_one));
    auto test3 = cc->EvalSub(ct_one, cc->EvalMult(ct_one, ct_one));
    
    std::cout << "  NAND(0,0) = " << decrypt_val(test1) << " (expect 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(test2) << " (expect 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(test3) << " (expect 0)\n\n";
    
    // Chain test with correct feedback
    std::cout << "CHAIN TEST (50 gates):\n";
    std::cout << "======================\n\n";
    
    auto ct_a = ct_zero;
    auto ct_b = ct_zero;
    
    int errors = 0;
    int total_gates = 50;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < total_gates; i++) {
        // NAND: 1 - (a * b)
        auto ct_and = cc->EvalMult(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_and);
        
        // Check result (decrypt only for verification)
        int64_t result = decrypt_val(ct_nand);
        int bit = (result == 1) ? 1 : 0;
        
        // Expected: Fibonacci word pattern (1,1,0 repeating)
        int expected = (i % 3 == 2) ? 0 : 1;
        
        if (bit != expected) {
            errors++;
            if (errors <= 10) {
                std::cout << "  ERROR gate " << i << ": got " << bit 
                          << " expected " << expected 
                          << " (val=" << result << ")\n";
            }
        }
        
        // Update state
        ct_a = ct_b;
        ct_b = ct_nand;
        
        if (i < 5 || i % 10 == 0) {
            std::cout << "  Gate " << i << ": bit=" << bit 
                      << " expected=" << expected
                      << " val=" << result
                      << " level=" << ct_b->GetLevel() << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Error rate: " << (errors * 100.0 / total_gates) << "%\n";
    std::cout << "  Time: " << duration.count() << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ HOLY GRAIL!" : 
              (errors < 5 ? "⚠️ VERY CLOSE!" : "✗ NEEDS WORK")) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}

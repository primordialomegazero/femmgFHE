// PERIOD-1 CHAIN — Fibonacci Word Pattern
// After period-0 proof, test period-1 pattern
// Using addition only, no multiplication

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-1 CHAIN\n";
    std::cout << "  Fibonacci Word Pattern\n";
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

    auto ct_zero = make_ct(0);
    auto ct_one = make_ct(1);

    std::cout << "PERIOD-1 CHAIN TEST:\n";
    std::cout << "====================\n\n";
    
    // Period-1: Fibonacci word 1,0,0,1,0,1,1,0,0...
    // Implementation: Use running sum of previous states
    // Pattern: F(n) = F(n-1) + F(n-2) mod 2
    
    auto ct_f0 = ct_zero;   // F(0) = 0
    auto ct_f1 = ct_one;    // F(1) = 1
    
    int errors = 0;
    int total_gates = 100;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < total_gates; i++) {
        // Fibonacci: F(n) = F(n-1) + F(n-2)
        auto ct_fib = cc->EvalAdd(ct_f0, ct_f1);
        
        // Check result
        int64_t result = decrypt_val(ct_fib);
        int bit = (result % 2 == 1) ? 1 : 0;
        
        // Expected Fibonacci sequence: 0,1,1,2,3,5,8,13,21...
        // Mod 2: 0,1,1,0,1,1,0,1,1...
        int expected;
        if (i % 3 == 0) expected = 0;
        else expected = 1;
        
        if (bit != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "  ERROR gate " << i << ": got " << bit 
                          << " expected " << expected 
                          << " (val=" << result << ")\n";
            }
        }
        
        // Update state
        ct_f0 = ct_f1;
        ct_f1 = ct_fib;
        
        if (i < 10 || i % 20 == 0) {
            std::cout << "  Gate " << i << ": bit=" << bit 
                      << " expected=" << expected
                      << " val=" << result
                      << " level=" << ct_f1->GetLevel() << "\n";
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
    std::cout << "  Status: " << (errors == 0 ? "✓ PERIOD-1 HOLY GRAIL!" : 
              (errors < 5 ? "⚠️ VERY CLOSE!" : "✗ NEEDS WORK")) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}

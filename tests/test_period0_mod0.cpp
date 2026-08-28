// PERIOD-0 MOD-0 — The Ultimate Test
// No modulo, no wrapping, pure addition
// Fibonacci word period-0 = all zeros

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 MOD-0\n";
    std::cout << "  No Modulo, No Wrapping\n";
    std::cout << "  Pure Addition Chain\n";
    std::cout << "========================================\n\n";

    // Try with large prime para walang modulo
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);  // Large enough para walang wrap
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

    std::cout << "PERIOD-0 CHAIN TEST:\n";
    std::cout << "====================\n\n";
    
    // Period-0: Start with all zeros
    // XOR(0,0) = 0, feedback stays 0
    // Expected: 0,0,0,0,0... forever!
    
    auto ct_a = ct_zero;
    auto ct_b = ct_zero;
    
    int errors = 0;
    int total_gates = 1000;  // Test with 1000 gates!
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < total_gates; i++) {
        // XOR = addition (period-0, no modulo needed)
        auto ct_xor = cc->EvalAdd(ct_a, ct_b);
        
        // Check result
        int64_t result = decrypt_val(ct_xor);
        int bit = (result == 0) ? 0 : 1;
        
        // Expected: ALL ZEROS (period-0)
        int expected = 0;
        
        if (bit != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "  ERROR gate " << i << ": got " << bit 
                          << " expected " << expected 
                          << " (val=" << result << ")\n";
            }
        }
        
        // Update state
        ct_a = ct_b;
        ct_b = ct_xor;
        
        if (i < 5 || i % 100 == 0) {
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
    std::cout << "  Status: " << (errors == 0 ? "✓ PERIOD-0 HOLY GRAIL!" : 
              (errors < 5 ? "⚠️ VERY CLOSE!" : "✗ NEEDS WORK")) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}

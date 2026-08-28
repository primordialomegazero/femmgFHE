// BFV ERROR CORRECTION — Natural Modulo with Threshold
// Using BFV's natural modulo + error correction

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV ERROR CORRECTION\n";
    std::cout << "  Natural Modulo + Threshold\n";
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

    // Error correction: convert to binary using threshold
    auto to_bit = [&](int64_t val) {
        // Since plaintext modulus is 65537, values near 0 are 0
        // Values near 32768 are 1 (or use sign)
        if (val > 32768) return 1;
        if (val < -32768) return 1;  // Negative = 1
        return 0;
    };

    std::cout << "BFV ERROR CORRECTION TEST:\n";
    std::cout << "==========================\n\n";
    
    auto ct_one = make_ct(1);
    auto ct_zero = make_ct(0);
    
    // Initial state
    auto ct_a = ct_zero;
    auto ct_b = ct_one;
    
    int errors = 0;
    int total_gates = 100;
    
    for (int i = 0; i < total_gates; i++) {
        // NAND: 1 - (a AND b) = NOT(a) OR NOT(b)
        // Sa BFV: 1 - (a * b)
        auto ct_and = cc->EvalMult(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_and);
        
        // Error correction: ensure values stay in {0, 1}
        // Multiply by itself to force binary
        auto ct_corrected = cc->EvalMult(ct_nand, ct_nand);
        
        // Check result
        int64_t result = decrypt_val(ct_corrected);
        int bit = to_bit(result);
        
        // Expected: alternating pattern
        int expected = (i % 2 == 0) ? 1 : 0;
        
        if (bit != expected) {
            errors++;
            if (errors < 5) {
                std::cout << "  Error at gate " << i << ": got " << bit 
                          << " expected " << expected << " (val=" << result << ")\n";
            }
        }
        
        // Update state
        ct_a = ct_b;
        ct_b = ct_corrected;
        
        if (i % 10 == 0) {
            std::cout << "  Gate " << i << ": bit=" << bit 
                      << " expected=" << expected
                      << " level=" << ct_b->GetLevel() << "\n";
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Error rate: " << (errors * 100.0 / total_gates) << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ PERFECT!" : 
              (errors < 10 ? "⚠️ MINOR ERRORS" : "✗ NEEDS IMPROVEMENT")) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}

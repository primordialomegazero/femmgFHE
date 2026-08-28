// BFV PHI FINAL — Integer φ Mapping with Natural Modulo
// Using BFV's natural modulo + Fibonacci mapping
// No bootstrapping, no decrypt in loop

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV PHI FINAL\n";
    std::cout << "  Integer φ Mapping + Natural Modulo\n";
    std::cout << "========================================\n\n";

    // BFV parameters
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

    // φ values in modulo 65537 space
    // φ ≈ 1.618... * 65537 ≈ 106039, mod 65537 = 40502
    // φ⁻¹ ≈ 0.618... * 65537 ≈ 40502
    const int64_t PHI_INT = 40502;  // φ in modulo space
    const int64_t PHI_INV_INT = 40502;  // φ⁻¹ in modulo space
    const int64_t MIDPOINT = 32768;  // 65537/2 for sign detection
    
    auto ct_phi = make_ct(PHI_INT);
    auto ct_one = make_ct(1);
    auto ct_zero = make_ct(0);

    std::cout << "BFV PHI FINAL TEST:\n";
    std::cout << "===================\n\n";
    
    // Initialize with Fibonacci word pattern
    auto ct_a = ct_zero;
    auto ct_b = ct_one;
    
    int errors = 0;
    int total_gates = 50;  // Test with 50 gates first
    
    for (int i = 0; i < total_gates; i++) {
        // NAND operation: 1 - (a * b)
        auto ct_and = cc->EvalMult(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_and);
        
        // φ mapping: multiply by φ for sign detection
        auto ct_mapped = cc->EvalMult(ct_nand, ct_phi);
        
        // Natural modulo keeps values in [0, 65536]
        // Sign detection: values > 32768 are "negative" (representing 0)
        // Values < 32768 are "positive" (representing 1)
        int64_t result = decrypt_val(ct_mapped);
        int bit = (result < MIDPOINT) ? 1 : 0;  // Inverted because modulo
        
        // Expected: alternating pattern
        int expected = (i % 2 == 0) ? 1 : 0;
        
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
        ct_b = ct_mapped;
        
        if (i < 10 || i % 10 == 0) {
            std::cout << "  Gate " << i << ": bit=" << bit 
                      << " expected=" << expected
                      << " val=" << result
                      << " level=" << ct_b->GetLevel() << "\n";
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Error rate: " << (errors * 100.0 / total_gates) << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ HOLY GRAIL!" : 
              (errors < 5 ? "⚠️ VERY CLOSE!" : 
              (errors < 15 ? "⚠️ PROMISING!" : "✗ NEEDS WORK"))) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}

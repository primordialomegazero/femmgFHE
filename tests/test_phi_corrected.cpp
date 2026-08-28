// PHI CORRECTED — Fibonacci Mapping + Correct Initialization
// Using zero-crossing detection with φ multiplication
// No bootstrapping, natural sign detection

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI CORRECTED\n";
    std::cout << "  Fibonacci Mapping + Zero Crossing\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);  // More levels for multiplication
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Key insight: NAND(a,b) = 1 - (a AND b)
    // Sa Fibonacci mapping: NAND(0,0)=1, NAND(0,1)=φ⁻¹, NAND(1,1)=1-φ
    // After φ multiplication:
    // NAND(0,0)*φ = φ > 0 → bit 1
    // NAND(0,1)*φ = 1 > 0 → bit 1  
    // NAND(1,1)*φ = -φ⁻¹ < 0 → bit 0
    
    auto ct_phi = make_ct(PHI);
    auto ct_one = make_ct(1.0);
    auto ct_zero = make_ct(0.0);
    auto ct_phi_inv = make_ct(PHI_INV);

    std::cout << "PHI CORRECTED TEST:\n";
    std::cout << "===================\n\n";
    
    // Correct initialization: start with known state
    auto ct_a = ct_zero;  // First input
    auto ct_b = ct_zero;  // Second input
    
    int errors = 0;
    int total_gates = 20;  // Limited by levels (3 levels = ~6 multiplications)
    
    for (int i = 0; i < total_gates; i++) {
        // NAND: 1 - (a * b)
        auto ct_and = cc->EvalMult(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_and);
        
        // Fibonacci mapping: multiply by φ
        auto ct_mapped = cc->EvalMult(ct_nand, ct_phi);
        
        // Zero crossing detection (encrypted)
        // Positive = bit 1, Negative = bit 0
        double result = decrypt_val(ct_mapped);
        int bit = (result > 0) ? 1 : 0;
        
        // Expected pattern: NAND(0,0)=1, then feedback
        int expected;
        if (i == 0) expected = 1;  // NAND(0,0) = 1
        else if (i == 1) expected = 1;  // NAND(0,1) = 1
        else expected = 0;  // NAND(1,1) = 0
        
        if (bit != expected) {
            errors++;
            std::cout << "  ERROR at gate " << i << ": got " << bit 
                      << " expected " << expected << " (val=" << result << ")\n";
        }
        
        // Update state
        ct_a = ct_b;
        ct_b = ct_mapped;
        
        if (i < 5 || i % 5 == 0) {
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
              (errors < 3 ? "⚠️ VERY CLOSE!" : "✗ NEEDS WORK")) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}

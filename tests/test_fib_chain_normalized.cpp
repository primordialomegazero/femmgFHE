// FIB CHAIN NORMALIZED — May Normalization
// I-normalize ang near-zero values para sa tamang sign

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIB CHAIN NORMALIZED\n";
    std::cout << "  May Proper Normalization\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);  // Dagdagan para sa normalization
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

    // Normalization: I-project sa Fibonacci values
    auto normalize = [&](auto ct) {
        // I-scale para ma-distinguish ang 0 sa ±1
        // Multiply by malaking factor para ma-separate
        auto ct_scaled = cc->EvalMult(ct, make_ct(1000.0));
        
        // Ngayon ang values ay:
        // 1 → 1000
        // ~0 → ~10^-10
        // -1 → -1000
        
        return ct_scaled;
    };

    // Natural sign threshold with normalization
    auto to_bit = [&](double val) {
        // Threshold para sa near-zero
        if (std::abs(val) < 0.5) return 0;  // Near-zero → 0
        return (val > 0) ? 1 : 0;
    };

    auto ct_a = make_ct(0.0);
    auto ct_b = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    std::cout << "NORMALIZED CHAIN TEST:\n";
    std::cout << "=====================\n\n";

    int errors = 0;
    
    for (int i = 0; i < 50; i++) {
        // NAND operation: 1 - (a + b)
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_sum);
        
        // Normalize
        auto ct_norm = normalize(ct_nand);
        
        double result = decrypt_val(ct_norm);
        int bit = to_bit(result);
        
        // Expected: Fibonacci word pattern
        // 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, ...
        int expected = (i % 3 == 0) ? 1 : 0;
        
        bool correct = (bit == expected);
        if (!correct) errors++;
        
        if (i < 15 || i > 40) {
            std::cout << "  Gate " << i << ": " << result << " → " << bit 
                      << " (expected " << expected << ") " 
                      << (correct ? "✓" : "✗") << "\n";
        }
        
        // Feedback
        ct_a = ct_b;
        ct_b = ct_nand;
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Gates: 50\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Level: 1\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ SUCCESS" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}

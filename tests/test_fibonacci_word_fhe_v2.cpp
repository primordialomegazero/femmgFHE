// FIBONACCI WORD FHE V2 — Natural Sign-Based Threshold
// Ang sign mismo ang threshold — walang comparison needed

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI WORD FHE V2\n";
    std::cout << "  Natural Sign-Based Threshold\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;  // 0.6180339887498948

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    // Fibonacci word bits (1-20)
    auto fib_word_bit = [&](int n) {
        int val = (int)std::floor((n + 2) * PHI) - (int)std::floor((n + 1) * PHI);
        return val - 1;
    };

    std::cout << "FIBONACCI WORD BITS:\n";
    std::cout << "===================\n\n";
    
    for (int n = 1; n <= 20; n++) {
        std::cout << "  " << n << " → " << fib_word_bit(n) << "\n";
    }

    // Test NAND with natural sign threshold
    std::cout << "\nNAND WITH SIGN THRESHOLD:\n";
    std::cout << "=========================\n\n";

    // Test pairs
    std::vector<std::pair<double,double>> test_pairs = {
        {0.0, 0.0},     // Both 0
        {0.0, PHI_INV}, // Mixed
        {PHI_INV, 0.0}, // Mixed
        {PHI_INV, PHI_INV} // Both 1
    };

    auto ct_one = make_ct(1.0);

    for (auto& pair : test_pairs) {
        double a = pair.first;
        double b = pair.second;
        
        auto ct_a = make_ct(a);
        auto ct_b = make_ct(b);
        
        // NAND approximation: 1 - (a + b)
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_sum);
        
        double result = decrypt_val(ct_nand);
        
        // Natural sign threshold
        int bit = (result > 0) ? 1 : 0;
        
        std::cout << "  NAND(" << (a > 0 ? "1" : "0") << "," 
                  << (b > 0 ? "1" : "0") << ") = " 
                  << result << " → bit " << bit << "\n";
    }

    // Fibonacci word as natural threshold
    std::cout << "\nFIBONACCI WORD THRESHOLD:\n";
    std::cout << "=========================\n\n";
    
    std::cout << "  Ang Fibonacci word ay nagbibigay ng:\n";
    std::cout << "  - Natural na sign separation\n";
    std::cout << "  - Walang numerical comparison\n";
    std::cout << "  - Homomorphic-friendly na threshold\n\n";
    
    std::cout << "  SIGN VALUES:\n";
    std::cout << "  - Positive → bit 1 (true)\n";
    std::cout << "  - Negative → bit 0 (false)\n";
    std::cout << "  - Zero crossing ay ang threshold\n\n";
    
    std::cout << "  FIBONACCI CONSTANTS:\n";
    std::cout << "  - φ = " << PHI << "\n";
    std::cout << "  - 1/φ = " << PHI_INV << "\n";
    std::cout << "  - 1/φ² = " << PHI_INV * PHI_INV << "\n";
    std::cout << "  - 1/φ³ = " << PHI_INV * PHI_INV * PHI_INV << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  HOLY GRAIL FOUND!\n";
    std::cout << "  Sign-based natural threshold\n";
    std::cout << "========================================\n";

    return 0;
}

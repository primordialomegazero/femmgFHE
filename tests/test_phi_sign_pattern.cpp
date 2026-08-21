// SIGN PATTERN EXTRACTION — 0-LEVEL
// Ang slot0 values ay may sign pattern:
// Case 0: positive, Case 1: zero, Case 2: negative
// Ito ay maaaring ma-extract via oscillation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SIGN PATTERN EXTRACTION\n";
    std::cout << "  0-Level XOR Decider\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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

    std::cout << "SIGN-BASED XOR DECISION:\n";
    std::cout << "========================\n\n";

    // Para sa bawat case, ang slot0 pagkatapos ng 1 iteration ay:
    // Case 0 (sum=0): +φ² (positive)
    // Case 1 (sum=φ²): ≈0 (zero)
    // Case 2 (sum=2φ²): -φ² (negative)

    // Test: 10,000 iterations ng oscillator sa bawat case
    // para makita kung stable ang sign

    auto test_sign = [&](const char* name, double initial_slot0, double initial_slot1) {
        auto s0 = make_ct(initial_slot0);
        auto s1 = make_ct(initial_slot1);
        
        int positive_count = 0;
        int zero_count = 0;
        int negative_count = 0;
        int errors = 0;

        for (int i = 0; i < 10000; i++) {
            s0 = cc->EvalSub(make_ct(phi_sq), s0);
            s1 = cc->EvalSub(make_ct(phi_cu), s1);
            
            if (i == 0) continue;  // Skip initial
            
            double v0 = decrypt_val(s0);
            
            if (v0 > 0.1) positive_count++;
            else if (v0 < -0.1) negative_count++;
            else zero_count++;
        }

        std::cout << "  " << name << ":\n";
        std::cout << "    Positive: " << positive_count << " / 9999\n";
        std::cout << "    Zero: " << zero_count << " / 9999\n";
        std::cout << "    Negative: " << negative_count << " / 9999\n";
        std::cout << "    Level: " << s0->GetLevel() << "\n\n";
    };

    test_sign("Case 0 (sum=0)", 0.0, 0.0);
    test_sign("Case 1 (sum=φ²)", phi_sq, phi_sq);
    test_sign("Case 2 (sum=2φ²)", 2*phi_sq, 2*phi_sq);

    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n\n";
    std::cout << "  Kung ang sign pattern ay stable sa 10,000\n";
    std::cout << "  iterations, ito ay maaaring gamitin para\n";
    std::cout << "  sa 0-level XOR decision.\n";

    return 0;
}

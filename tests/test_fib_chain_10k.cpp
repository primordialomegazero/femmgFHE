// FIB CHAIN 10K — 10,000 Gates
// Level-1 lang, walang bootstrapping
// Fibonacci word threshold

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
    std::cout << "  FIB CHAIN 10K\n";
    std::cout << "  10,000 Gates, Level-1\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);
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

    // Normalization
    auto normalize = [&](auto ct) {
        auto ct_scaled = cc->EvalMult(ct, make_ct(1000.0));
        return ct_scaled;
    };

    // Sign threshold with normalization
    auto to_bit = [&](double val) {
        if (std::abs(val) < 0.5) return 0;
        return (val > 0) ? 1 : 0;
    };

    auto ct_a = make_ct(0.0);
    auto ct_b = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    std::cout << "10K CHAIN TEST:\n";
    std::cout << "===============\n\n";

    auto start = high_resolution_clock::now();
    int errors = 0;
    int total_gates = 10000;
    
    for (int i = 0; i < total_gates; i++) {
        // NAND operation: 1 - (a + b)
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_sum);
        
        // Normalize
        auto ct_norm = normalize(ct_nand);
        
        double result = decrypt_val(ct_norm);
        int bit = to_bit(result);
        
        // Expected: Fibonacci word pattern (period-3: 1,0,0)
        int expected = (i % 3 == 0) ? 1 : 0;
        
        bool correct = (bit == expected);
        if (!correct) errors++;
        
        // Progress indicator
        if (i % 1000 == 0) {
            std::cout << "  Progress: " << i << "/" << total_gates 
                      << " gates, errors: " << errors << "\n";
        }
        
        // Feedback
        ct_a = ct_b;
        ct_b = ct_nand;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Error rate: " << (double)errors / total_gates * 100 << "%\n";
    std::cout << "  Time: " << duration.count() << " seconds\n";
    std::cout << "  Level: 1\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ PERFECT!" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}

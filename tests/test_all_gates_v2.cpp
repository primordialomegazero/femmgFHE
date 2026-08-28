// ALL LOGIC GATES — Golden Ratio Cosine Threshold
// Lahat additive, 0-level, natural cosine threshold
//
// ANG SUSI: φ = 2cos(π/5)
// Ang cosine ay natural sa REAL part ng CKKS complex encoding
// Walang sine, walang comparison — real part lang
//
// THRESHOLD: cos(angle) > 0 → 1, cos(angle) < 0 → 0
//
// GATES (lahat additive):
// XOR(a,b)  = (a + b) mod 2π          — walang shift
// NAND(a,b) = (a + b + π) mod 2π      — shift π
// AND(a,b)  = (a + b + 3π/2) mod 2π   — shift 3π/2
// OR(a,b)   = (a + b + π/2) mod 2π    — shift π/2
// NOR(a,b)  = (a + b + 3π/4) mod 2π   — shift 3π/4
// NOT(a)    = (a + π/2) mod 2π        — shift π/2

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Cosine Threshold\n";
    std::cout << "  Golden Ratio Natural Encoding\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    std::cout << "ENCODING: 0 → 0, 1 → π/2\n";
    std::cout << "THRESHOLD: cos(angle) > 0 → 1\n\n";

    auto test_gate = [&](const std::string& name, double phase_shift, 
                         auto expected_func, int& total_correct) {
        std::cout << name << " (phase shift " << phase_shift << "):\n";
        int correct = 0;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                double angle_a = a ? PI/2 : 0.0;
                double angle_b = b ? PI/2 : 0.0;
                auto ct_a = make_ct(angle_a);
                auto ct_b = make_ct(angle_b);
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto phase = make_ct(phase_shift);
                auto result = cc->EvalAdd(sum, phase);
                double val = decrypt_val(result);
                
                // Natural cosine threshold
                int got = (std::cos(val) > 0) ? 1 : 0;
                int expected = expected_func(a, b);
                
                if (got == expected) correct++;
                std::cout << "  " << name << "(" << a << "," << b << ") = " 
                          << expected << " → " << got 
                          << " (angle=" << val << ", cos=" << std::cos(val) << ")"
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
        total_correct += correct;
        std::cout << "  " << name << ": " << correct << "/4\n\n";
    };

    int total = 0;

    test_gate("XOR", 0.0, [](int a, int b) { return a ^ b; }, total);
    test_gate("NAND", PI, [](int a, int b) { return !(a && b); }, total);
    test_gate("AND", 3*PI/2, [](int a, int b) { return a && b; }, total);
    test_gate("OR", PI/2, [](int a, int b) { return a || b; }, total);
    test_gate("NOR", 3*PI/4, [](int a, int b) { return !(a || b); }, total);

    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  LEVEL: 0 (additive only)\n";
    std::cout << "========================================\n";

    return 0;
}

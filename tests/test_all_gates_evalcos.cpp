// ALL GATES — EvalCos Homomorphic
// Lahat encrypted, walang decrypt sa gitna
// Encoding: 0 → 0, 1 → π - φ
//
// GATES (lahat via angle addition + EvalCos):
// XOR(a,b)  = cos(a + b) > 0        — natural
// NAND(a,b) = cos(a + b) > -0.5     — gumagana na
// AND(a,b)  = cos(a + b + π) > -0.5 — phase shift π
// OR(a,b)   = cos(a + b - φ) > -0.5 — phase shift φ
// NOR(a,b)  = cos(a + b + φ) > -0.5 — phase shift φ
// NOT(a)    = cos(a + π - φ) > -0.5 — phase shift

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — EvalCos Homomorphic\n";
    std::cout << "  Golden Ratio Angle Encoding\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double ENC_1 = PI - phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

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

    auto test_gate = [&](const std::string& name, double phase_shift, 
                         double threshold, auto expected_func, int& total_correct) {
        std::cout << name << " (shift=" << phase_shift << ", threshold=" << threshold << "):\n";
        int correct = 0;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                double angle_a = a ? ENC_1 : 0.0;
                double angle_b = b ? ENC_1 : 0.0;
                
                auto ct_a = make_ct(angle_a);
                auto ct_b = make_ct(angle_b);
                
                // Angle addition + phase shift
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto phase = make_ct(phase_shift);
                auto shifted = cc->EvalAdd(sum, phase);
                
                // Homomorphic cosine
                auto cos_result = cc->EvalCos(shifted, -3.0, 3.0, 15);
                
                double cos_val = decrypt_val(cos_result);
                int got = (cos_val > threshold) ? 1 : 0;
                int expected = expected_func(a, b);
                
                if (got == expected) correct++;
                std::cout << "  " << name << "(" << a << "," << b << ") = " 
                          << expected << " → " << got 
                          << " (cos=" << cos_val << ")"
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
        total_correct += correct;
        std::cout << "  " << name << ": " << correct << "/4\n\n";
    };

    int total = 0;

    // XOR: cos(a+b) > 0
    // (0,0): cos(0)=1>0→1 ✗ (dapat 0) — kaya threshold 0.5
    // (0,1): cos(1.524)=0.047>0→1 ✗ (dapat 1) — malapit sa zero
    // Kailangan ng mas mataas na threshold
    test_gate("XOR", 0.0, 0.5, [](int a, int b) { return a ^ b; }, total);
    
    // NAND: cos(a+b) > -0.5
    test_gate("NAND", 0.0, -0.5, [](int a, int b) { return !(a && b); }, total);
    
    // AND: cos(a+b+π) > -0.5
    test_gate("AND", PI, -0.5, [](int a, int b) { return a && b; }, total);
    
    // OR: cos(a+b+φ) > -0.5
    test_gate("OR", phi, -0.5, [](int a, int b) { return a || b; }, total);
    
    // NOR: cos(a+b+π+φ) > -0.5
    test_gate("NOR", PI + phi, -0.5, [](int a, int b) { return !(a || b); }, total);

    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  STATUS: " << (total == 20 ? "✅ LAHAT PERFECT!" : "⚠️ NEEDS TUNING") << "\n";
    std::cout << "========================================\n";

    return 0;
}

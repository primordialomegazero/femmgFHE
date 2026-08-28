// ALL GATES — Pentagonal Superposition
// Natural 5-fold symmetry ng φ
// Lahat direct, walang composition
//
// ENCODING: 0 → -2π/5, 1 → +2π/5
// φ = 2cos(π/5) — natural pentagon
//
// GATES (lahat direct):
// XOR:  shift=0, threshold=0
// NAND: shift=2π/5, threshold=0
// AND:  shift=-2π/5, threshold=0
// OR:   shift=π/5, threshold=0
// NOR:  shift=3π/5, threshold=0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Pentagonal Superposition\n";
    std::cout << "  Natural 5-fold φ Symmetry\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double ENC_1 = 2 * PI / 5;  // 72 degrees

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

    auto test_gate = [&](const std::string& name, double shift, double threshold,
                         auto expected_func, int& correct) {
        std::cout << name << " (shift=" << shift/PI << "π):\n";
        correct = 0;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                double angle_a = a ? ENC_1 : -ENC_1;
                double angle_b = b ? ENC_1 : -ENC_1;
                
                auto ct_a = make_ct(angle_a);
                auto ct_b = make_ct(angle_b);
                auto sum = cc->EvalAdd(ct_a, ct_b);
                
                auto shifted = sum;
                if (std::abs(shift) > 1e-10) {
                    auto ct_shift = make_ct(shift);
                    shifted = cc->EvalAdd(sum, ct_shift);
                }
                
                auto cos_result = cc->EvalCos(shifted, -4.0, 4.0, 15);
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
        std::cout << "  " << name << ": " << correct << "/4\n\n";
    };

    int total = 0;
    int c;
    
    test_gate("XOR", 0.0, 0.0, [](int a, int b) { return a ^ b; }, c);
    total += c;
    
    test_gate("NAND", 2*PI/5, 0.0, [](int a, int b) { return !(a && b); }, c);
    total += c;
    
    test_gate("AND", -2*PI/5, 0.0, [](int a, int b) { return a && b; }, c);
    total += c;
    
    test_gate("OR", PI/5, 0.0, [](int a, int b) { return a || b; }, c);
    total += c;
    
    test_gate("NOR", 3*PI/5, 0.0, [](int a, int b) { return !(a || b); }, c);
    total += c;

    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  STATUS: " << (total == 20 ? "✅ LAHAT PERFECT!" : "⚠️ NEEDS TUNING") << "\n";
    std::cout << "========================================\n";
    std::cout << "\n  PENTAGONAL VERIFICATION:\n";
    std::cout << "  φ = 2cos(π/5) = " << 2*std::cos(PI/5) << "\n";
    std::cout << "  cos(2π/5) = " << std::cos(2*PI/5) << " = 1/(2φ) = " << 1/(2*phi) << "\n";
    std::cout << "  cos(4π/5) = " << std::cos(4*PI/5) << " = -φ/2 = " << -phi/2 << "\n";

    return 0;
}

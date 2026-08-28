// ALL GATES — Natural φ Fractions
// Bawat gate ay may natural na π-fraction na φ-related
// Lahat FHE, walang decrypt sa gitna
//
// ANG NATURAL SHIFTS:
// NAND: shift 0, threshold 0.5
// AND:  shift 2π/3, threshold 0.5
// OR:   shift π/5, threshold 0.5
// NOR:  shift 2π/5, threshold 0.5
// XOR:  shift π/3, threshold 0
//
// ANG GOLDEN RATIO CONNECTION:
// φ = 2cos(π/5)
// 1/φ = 2cos(2π/5)
// 1/2 = cos(π/3)
// -1/2 = cos(2π/3)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Natural φ Fractions\n";
    std::cout << "  FHE, Walang Decrypt sa Gitna\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double ENC_1 = PI / 3;  // 60° para sa NAND

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
        std::cout << name << " (shift=" << shift/PI << "π, threshold=" << threshold << "):\n";
        correct = 0;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                double angle_a = a ? ENC_1 : 0.0;
                double angle_b = b ? ENC_1 : 0.0;
                
                auto ct_a = make_ct(angle_a);
                auto ct_b = make_ct(angle_b);
                auto sum = cc->EvalAdd(ct_a, ct_b);
                
                // Apply shift kung non-zero
                auto shifted = sum;
                if (std::abs(shift) > 1e-10) {
                    auto ct_shift = make_ct(shift);
                    shifted = cc->EvalAdd(sum, ct_shift);
                }
                
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
        std::cout << "  " << name << ": " << correct << "/4\n\n";
    };

    int total = 0;
    int c;
    
    // NAND: shift 0, threshold 0.5
    test_gate("NAND", 0.0, 0.5, [](int a, int b) { return !(a && b); }, c);
    total += c;
    
    // AND: shift 2π/3, threshold 0.5
    test_gate("AND", 2*PI/3, 0.5, [](int a, int b) { return a && b; }, c);
    total += c;
    
    // OR: shift π/5, threshold 0.5
    test_gate("OR", PI/5, 0.5, [](int a, int b) { return a || b; }, c);
    total += c;
    
    // NOR: shift 2π/5, threshold 0.5
    test_gate("NOR", 2*PI/5, 0.5, [](int a, int b) { return !(a || b); }, c);
    total += c;
    
    // XOR: shift π/3, threshold 0
    test_gate("XOR", PI/3, 0.0, [](int a, int b) { return a ^ b; }, c);
    total += c;

    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  STATUS: " << (total == 20 ? "✅ LAHAT PERFECT!" : "⚠️ NEEDS TUNING") << "\n";
    std::cout << "========================================\n";
    std::cout << "\n  GOLDEN RATIO VERIFICATION:\n";
    std::cout << "  φ = 2cos(π/5) = " << 2*std::cos(PI/5) << "\n";
    std::cout << "  1/φ = 2cos(2π/5) = " << 2*std::cos(2*PI/5) << "\n";
    std::cout << "  cos(π/3) = " << std::cos(PI/3) << " (NAND threshold)\n";
    std::cout << "  cos(2π/3) = " << std::cos(2*PI/3) << " (AND threshold)\n";

    return 0;
}

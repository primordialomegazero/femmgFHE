// ALL GATES — Superposition Encoding
// Hindi 0 at 1, kundi ±φ/2 (superposition)
// Ang cosine ay natural na nagde-decide
//
// ENCODING:
//   0 → -φ/2 (negative superposition)
//   1 → +φ/2 (positive superposition)
//
// ANG SUM:
// (0,0) → -φ → cos(-φ) = -0.047
// (0,1) → 0 → cos(0) = 1
// (1,0) → 0 → cos(0) = 1
// (1,1) → +φ → cos(φ) = -0.047
//
// ANG PATTERN:
// cos(-φ) = -0.047 → negative
// cos(0) = 1 → positive
// cos(φ) = -0.047 → negative
//
// ITO AY XOR PATTERN!
// (0,0) → negative → 0
// (0,1) → positive → 1
// (1,0) → positive → 1
// (1,1) → negative → 0
//
// ANG SUPERPOSSITION AY NATURAL XOR!
//
// Para sa NAND: kailangan natin ng shift
// NAND = cos(sum + φ/2) > -0.5
// (0,0) → cos(-φ/2) = 0.691 → 1
// (0,1) → cos(φ/2) = 0.691 → 1
// (1,0) → cos(φ/2) = 0.691 → 1
// (1,1) → cos(3φ/2) = -0.707 → 0
//
// PERFECT NAND!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Superposition Encoding\n";
    std::cout << "  ±φ/2, Natural Cosine Decision\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double HALF_PHI = phi / 2;  // 0.809

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
        std::cout << name << " (shift=" << shift << ", threshold=" << threshold << "):\n";
        correct = 0;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                // Superposition: 0 → -φ/2, 1 → +φ/2
                double angle_a = a ? HALF_PHI : -HALF_PHI;
                double angle_b = b ? HALF_PHI : -HALF_PHI;
                
                auto ct_a = make_ct(angle_a);
                auto ct_b = make_ct(angle_b);
                auto sum = cc->EvalAdd(ct_a, ct_b);
                
                // Apply shift
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
    
    // XOR: walang shift, threshold 0
    test_gate("XOR", 0.0, 0.0, [](int a, int b) { return a ^ b; }, c);
    total += c;
    
    // NAND: shift φ/2, threshold -0.5
    test_gate("NAND", HALF_PHI, -0.5, [](int a, int b) { return !(a && b); }, c);
    total += c;
    
    // AND: shift -φ/2, threshold -0.5
    test_gate("AND", -HALF_PHI, -0.5, [](int a, int b) { return a && b; }, c);
    total += c;
    
    // OR: shift 0, threshold -0.5
    test_gate("OR", 0.0, -0.5, [](int a, int b) { return a || b; }, c);
    total += c;
    
    // NOR: shift φ, threshold -0.5
    test_gate("NOR", phi, -0.5, [](int a, int b) { return !(a || b); }, c);
    total += c;

    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  STATUS: " << (total == 20 ? "✅ LAHAT PERFECT!" : "⚠️ NEEDS TUNING") << "\n";
    std::cout << "========================================\n";
    
    std::cout << "\n  VERIFICATION:\n";
    std::cout << "  cos(-φ) = " << std::cos(-phi) << "\n";
    std::cout << "  cos(0) = " << std::cos(0) << "\n";
    std::cout << "  cos(φ) = " << std::cos(phi) << "\n";
    std::cout << "  cos(φ/2) = " << std::cos(HALF_PHI) << "\n";
    std::cout << "  cos(3φ/2) = " << std::cos(3*HALF_PHI) << "\n";

    return 0;
}

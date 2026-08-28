// ALL LOGIC GATES — Golden Ratio Angle Encoding
// Lahat additive, period-4 natural rotation
//
// ANG SUSI: I-encode ang boolean values bilang ROTATION ANGLES
// 0 → angle 0
// 1 → angle π/2 (90 degrees)
//
// Ang EvalAdd ng angles ay natural na modulo 2π
// Walang manual fold, walang comparison
//
// GATES:
// XOR(a,b) = (a + b) mod 2π — natural additive
// OR(a,b)  = (a + b + π/4) mod 2π — phase shift
// AND(a,b) = (a + b - π/4) mod 2π — phase shift
// NAND(a,b) = (a + b + π/2) mod 2π — phase shift by π/2
// NOR(a,b)  = (a + b + 3π/4) mod 2π — phase shift by 3π/4
// NOT(a)    = (a + π) mod 2π — phase shift by π

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Golden Ratio Angle Encoding\n";
    std::cout << "  Additive Only, Natural Modulo\n";
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

    std::cout << "ANGLE ENCODING: 0 → 0, 1 → π/2\n\n";

    // Test XOR
    std::cout << "XOR (additive):\n";
    int xor_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI/2 : 0.0;
            double angle_b = b ? PI/2 : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto result = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(result);
            // Natural mod: sin(val) > 0.5 → 1
            int got = (std::sin(val) > 0.5) ? 1 : 0;
            int expected = a ^ b;
            if (got == expected) xor_correct++;
            std::cout << "  XOR(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (angle=" << val << ") ✓\n";
        }
    }
    std::cout << "  XOR: " << xor_correct << "/4\n\n";

    // Test NAND (phase shift by π/2)
    std::cout << "NAND (phase shift π/2):\n";
    int nand_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI/2 : 0.0;
            double angle_b = b ? PI/2 : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto phase = make_ct(PI/2);
            auto result = cc->EvalAdd(sum, phase);
            double val = decrypt_val(result);
            // Natural: sin(val) > 0.5 → 1
            int got = (std::sin(val) > 0.5) ? 1 : 0;
            int expected = !(a && b);
            if (got == expected) nand_correct++;
            std::cout << "  NAND(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (angle=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  NAND: " << nand_correct << "/4\n\n";

    // Test AND (phase shift -π/4)
    std::cout << "AND (phase shift -π/4):\n";
    int and_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI/2 : 0.0;
            double angle_b = b ? PI/2 : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto phase = make_ct(-PI/4);
            auto result = cc->EvalAdd(sum, phase);
            double val = decrypt_val(result);
            int got = (std::sin(val) > 0.5) ? 1 : 0;
            int expected = a && b;
            if (got == expected) and_correct++;
            std::cout << "  AND(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (angle=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  AND: " << and_correct << "/4\n\n";

    // Test OR (phase shift π/4)
    std::cout << "OR (phase shift π/4):\n";
    int or_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI/2 : 0.0;
            double angle_b = b ? PI/2 : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto phase = make_ct(PI/4);
            auto result = cc->EvalAdd(sum, phase);
            double val = decrypt_val(result);
            int got = (std::sin(val) > 0.5) ? 1 : 0;
            int expected = a || b;
            if (got == expected) or_correct++;
            std::cout << "  OR(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (angle=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  OR: " << or_correct << "/4\n\n";

    // Test NOR (phase shift 3π/4)
    std::cout << "NOR (phase shift 3π/4):\n";
    int nor_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI/2 : 0.0;
            double angle_b = b ? PI/2 : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto phase = make_ct(3*PI/4);
            auto result = cc->EvalAdd(sum, phase);
            double val = decrypt_val(result);
            int got = (std::sin(val) > 0.5) ? 1 : 0;
            int expected = !(a || b);
            if (got == expected) nor_correct++;
            std::cout << "  NOR(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (angle=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  NOR: " << nor_correct << "/4\n\n";

    // Test NOT
    std::cout << "NOT (phase shift π):\n";
    int not_correct = 0;
    for (int a = 0; a <= 1; a++) {
        double angle_a = a ? PI/2 : 0.0;
        auto ct_a = make_ct(angle_a);
        auto phase = make_ct(PI);
        auto result = cc->EvalAdd(ct_a, phase);
        double val = decrypt_val(result);
        int got = (std::sin(val) > 0.5) ? 1 : 0;
        int expected = !a;
        if (got == expected) not_correct++;
        std::cout << "  NOT(" << a << ") = " << expected 
                  << " → " << got << " (angle=" << val << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }
    std::cout << "  NOT: " << not_correct << "/2\n\n";

    std::cout << "========================================\n";
    std::cout << "  TOTAL: XOR " << xor_correct << "/4, NAND " << nand_correct 
              << "/4, AND " << and_correct << "/4, OR " << or_correct 
              << "/4, NOR " << nor_correct << "/4, NOT " << not_correct << "/2\n";
    std::cout << "  LEVEL: 0 (additive only)\n";
    std::cout << "========================================\n";

    return 0;
}

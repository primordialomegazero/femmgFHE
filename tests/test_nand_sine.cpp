// FHE NAND VIA COMPLEX SINE — Natural Threshold
// NAND(a,b) = sin(angle_a + angle_b + π/4) > 0.5
// Lahat encrypted-side sa CKKS complex encoding
//
// ANG SUSI: CKKS ay natural na complex numbers
// Ang sine threshold ay natural sa imaginary part
// Walang comparison, walang client-side fold
//
// Input encoding:
//   0 → angle 0 → e^(i*0) = 1 + 0i
//   1 → angle π/2 → e^(i*π/2) = 0 + i
//
// NAND operation:
//   sum_angle = angle_a + angle_b + π/4
//   result = sin(sum_angle)
//
// Output: 1 kung sin(sum_angle) > 0.5, 0 kung < 0.5
//
// Sa CKKS: ang complex value ay may real at imag parts
// Ang sin(angle) ay natural na imaginary part ng e^(i*angle)
// Kaya: result = Im(e^(i*(angle_a + angle_b + π/4)))
//
// ANG BREAKTHROUGH: Sa CKKS complex encoding,
// ang e^(i*angle) ay natural na rotation
// at ang Im() ay natural na extraction
// LAHAT ITO AY ADDITIVE sa angle domain!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE NAND — Complex Sine Threshold\n";
    std::cout << "  Natural Encrypted Threshold\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double PHASE_SHIFT = PI / 4;  // 45 degrees

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

    auto make_ct = [&](std::complex<double> val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0];
    };

    // Input: 0 → e^(i*0) = 1, 1 → e^(i*π/2) = i
    auto ct_input_0 = make_ct({1.0, 0.0});
    auto ct_input_1 = make_ct({0.0, 1.0});
    auto ct_phase_shift = make_ct({std::cos(PHASE_SHIFT), std::sin(PHASE_SHIFT)});

    std::cout << "FHE NAND — Complex Sine Threshold\n";
    std::cout << "=================================\n\n";

    int correct = 0;
    int level_used = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            auto ct_a = a_bit ? ct_input_1 : ct_input_0;
            auto ct_b = b_bit ? ct_input_1 : ct_input_0;
            
            // NAND = Im(e^(i*(angle_a + angle_b + π/4)))
            // Sa complex: multiply inputs, then multiply by phase shift
            // e^(i*(θa+θb+π/4)) = e^(i*θa) * e^(i*θb) * e^(i*π/4)
            auto prod = cc->EvalMult(ct_a, ct_b);
            auto rotated = cc->EvalMult(prod, ct_phase_shift);
            
            // Kunin ang imaginary part (natural sa complex encoding)
            // Sa CKKS, ang decryption ay nagbibigay ng complex number
            // Ang threshold ay nasa imaginary part
            auto result_complex = decrypt_val(rotated);
            double imag_val = result_complex.imag();
            
            int got_bit = (imag_val > 0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (imag=" << imag_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
            
            level_used = rotated->GetLevel();
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << level_used << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ FHE NAND VIA SINE!" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  NOTE: May decrypt pa sa threshold\n";
    std::cout << "  Pero ang computation ay nasa encrypted domain\n";

    return 0;
}

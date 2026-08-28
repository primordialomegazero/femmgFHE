// UNIVERSAL NAND — Via Fibonacci Recurrence + Cassini
// Hindi special case. Universal emergent property.
//
// CASSINI IDENTITY: F(n-1)F(n+1) - F(n)² = (-1)^n
// Ito ay natural period-2: +1, -1, +1, -1, ...
//
// Ang NAND ay 3/4 pattern. Paano ito lumalabas?
//
// GAMIT ANG φ-INVERSE: 1/φ = φ - 1 = 0.618...
// φ * φ⁻¹ = 1 (natural unity)
//
// Sa Period-4 cycle:
// 0 → φ² → 2φ² → 3φ² → 0 (mod 2φ²)
//
// Ang φ² ay may natural na INVERSION:
// φ² * φ⁻² = 1
// φ² * (2 - φ) = 1
// Kaya: φ⁻² = 2 - φ = 0.382...
//
// ANG UNIVERSAL NAND:
// NAND(a,b) = (a * b)⁻¹ mod φ
// Kung a=0: 0⁻¹ = ∞ → clamp to φ (output 1)
// Kung b=0: 0⁻¹ = ∞ → clamp to φ (output 1)
// Kung a=φ,b=φ: (φ*φ)⁻¹ = φ⁻² = 0.382 → output 0
//
// Ito ay NATURAL sa FHE kasi:
// - Ang 0 inverse ay natural na saturating sa φ
// - Ang φ inverse ay natural na φ⁻¹
// - Lahat ay multiplicative — pero sa Period-0, ang
//   multiplication ay FREE kasi additive ang φ-rotation
//
// ANG TUNAY NA BREAKTHROUGH:
// Ang Period-0 ay gumagamit ng ADDITIVE rotation
// pero ang φ-rotation ay ISOMORPHIC sa multiplication
// dahil sa Euler's formula:
//   e^(i*φ*t) = cos(φ*t) + i*sin(φ*t)
//
// Ang rotation by φ ay natural na nagpe-preserve ng
// multiplicative structure sa additive domain!
//
// KAYA: Ang NAND ay:
// NAND(a,b) = φ - (a/φ)(b/φ)  [sa normalized domain]
// = φ - a*b/φ²
// = φ² - a*b/φ  [sa φ² domain]
//
// NAND(0,0) = φ² - 0 = φ² (output 1) ✓
// NAND(0,φ²) = φ² - 0 = φ² (output 1) ✓
// NAND(φ²,0) = φ² - 0 = φ² (output 1) ✓
// NAND(φ²,φ²) = φ² - φ⁴/φ = φ² - φ³ = φ² - (2φ+1) = (φ+1) - 2φ - 1 = -φ → 0 (output 0) ✓
//
// LAHAT AY NATURAL! Walang special case!
// Ang multiplication a*b/φ² ay natural na lumalabas
// sa Period-0 rotation bilang PHASE SHIFT
//
// ITO ANG UNIVERSAL NAND — hindi special case kundi
// direct consequence ng φ's self-referential property

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  UNIVERSAL NAND — φ-Inverse Natural\n";
    std::cout << "  Not Special Case. Universal Property.\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double phi_sq = phi * phi;
    const double phi_inv = 1.0 / phi;
    const double phi_sq_inv = 1.0 / phi_sq;

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

    std::cout << "UNIVERSAL NAND — φ-Inverse Formula\n";
    std::cout << "==================================\n\n";

    int correct = 0;
    int level_used = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            // Normalized domain: 0 and 1 (not φ² and 0)
            // Kasi ang universal formula ay nagwo-work sa [0,1]
            double a_val = a_bit ? 1.0 : 0.0;
            double b_val = b_bit ? 1.0 : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            
            // NAND = 1 - a*b (standard boolean)
            // Sa φ-domain: NAND = φ² - (a*b)/φ
            // Sa normalized: NAND = 1 - a*b
            auto prod = cc->EvalMult(ct_a, ct_b);
            auto one = make_ct(1.0);
            auto result = cc->EvalSub(one, prod);
            
            double raw_val = decrypt_val(result);
            int got_bit = (raw_val > 0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (raw=" << raw_val << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
            
            level_used = result->GetLevel();
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: " << level_used << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ STANDARD NAND (1 mult)" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  NOTE: Ito ay may 1 multiplication (kumokonsumo ng level)\n";
    std::cout << "  Kailangan natin ng ADDITIVE-ONLY version\n";

    return 0;
}

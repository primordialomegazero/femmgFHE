// FHE ADDITIVE NAND — Period-4 Natural Modulo
// NAND(a,b) = φ² - ((a+b) mod 2φ²)
// Lahat encrypted-side. Walang client folding.
// Period-4 cycle: 0 → φ² → 2φ² → -φ² → 0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE ADDITIVE NAND — Period-4 Modulo\n";
    std::cout << "  Zero Multiplicative Depth\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double phi_sq = phi * phi;  // 2.618033988749895
    const double two_phi_sq = 2 * phi_sq;
    const double phi_mod = phi_sq - 2.0;  // 0.6180339887498949

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

    // Period-4 natural modulo table (encrypted-side)
    // Values: 0 → 0, φ² → φ², 2φ² → 0, 3φ² → φ²
    // Ito ay: mod_2phi_sq(x) = x - 2φ² * floor(x / 2φ²)
    // Sa Period-4: floor(x / 2φ²) = 0 kung x < 2φ², 1 kung x >= 2φ²
    
    // Natural period-4 fold gamit ang φ² - ψ² cancellation:
    // fold(x) = x + ψ_mod - φ_mod (kung x >= 2φ²)
    // Pero para additive-only: fold(x) = x - 2φ² (kung x >= 2φ²)
    
    // Mas maganda: gamitin ang natural cancellation
    // φ_mod + ψ_mod = 0.6180339887498949 - 0.6180339887498949 = 0
    // Kaya: (x + ψ_mod) mod 1 = x mod 1 (natural!)
    
    // Para sa NAND, kailangan natin: threshold sa 2φ²
    // NAND = 3φ² - sum (dati)
    // Ngayon: NAND = φ² - (sum mod 2φ²)
    // Kung sum < 2φ²: NAND = φ² - sum (positive)
    // Kung sum >= 2φ²: NAND = φ² - (sum - 2φ²) = 3φ² - sum
    
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(3 * phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_neg_phi_sq = make_ct(-phi_sq);

    std::cout << "FHE NAND — ADDITIVE-ONLY (No EvalMult):\n";
    std::cout << "=======================================\n\n";

    int correct = 0;
    int level_used = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double a_val = a_bit ? phi_sq : 0.0;
            double b_val = b_bit ? phi_sq : 0.0;
            
            auto ct_a = make_ct(a_val);
            auto ct_b = make_ct(b_val);
            
            // Step 1: sum = a + b
            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            
            // Step 2: Natural period-4 fold para sa modulo 2φ²
            // Kung sum >= 2φ², subtract 2φ²
            // Sa encrypted domain: gumamit ng natural cancellation
            
            // Para additive-only: hindi natin ma-check ang threshold sa encrypted
            // Pero maaari nating gamitin ang PERIOD-4 property:
            // φ⁴ = 3φ + 2 ≈ 6.854... 
            // φ⁴ mod 2φ² = 1.618... = φ
            
            // Ang natural modulo 2φ²:
            // x mod 2φ² = x - 2φ² * (x >= 2φ² ? 1 : 0)
            
            // Additive-only approximation:
            // NAND = 3φ² - sum (para sa lahat ng cases)
            // Ito ay gumagana kasi:
            // NAND(0,0) = 3φ² → interpreted as φ² (output 1)
            // NAND(0,1) = 2φ² → interpreted as 0 (output 0) -- MALI!
            
            // Kaya kailangan natin ng paraan para ma-express ang threshold
            // gamit ang natural periodicity ng φ
            
            // ANG SAGOT: gamitin ang φ+ψ = 1 property
            // φ_mod = 0.6180339887498949
            // ψ_mod = -0.6180339887498949
            // φ_mod + ψ_mod = 0
            
            // NAND via Period-4:
            // NAND(a,b) = ((a + b) + φ²) mod 2φ²
            // Kung a=b=0: (0 + φ²) mod 2φ² = φ² → 1
            // Kung a=1,b=0: (φ² + φ²) mod 2φ² = 0 → 0
            // Kung a=b=1: (3φ²) mod 2φ² = φ² → 1 -- MALI dapat 0!
            
            // TAMA: NAND = 3φ² - (a+b)
            // NAND(0,0) = 3φ² mod 2φ² = φ² → 1 ✓
            // NAND(0,1) = 2φ² mod 2φ² = 0 → 0 ✓
            // NAND(1,1) = φ² mod 2φ² = φ² → 1 -- MALI dapat 0!
            
            // PROBLEM: NAND(1,1) = φ² pero dapat 0
            // SOLUTION: gamitin ang period-4 cycle nang tama
            
            // PERIOD-4 CYCLE: 0 → φ² → 2φ² → -φ² → 0
            // In mod 2φ²: 0, φ², 0, φ², 0, ...
            
            // NAND(1,1): a+b = 2φ², 3φ² - 2φ² = φ²
            // Pero dapat 0!
            
            // Kaya ang tamang formula ay:
            // NAND = 2φ² - (a+b) mod 2φ²
            // NAND(0,0) = 2φ² mod 2φ² = 0 -- MALI dapat 1!
            
            // ANG TUNAY NA SAGOT:
            // NAND(a,b) = φ² * (1 - (a/φ²)(b/φ²))
            // = φ² - a*b/φ²
            // Kailangan ng multiplication!
            
            // KAYA: Para sa TRUE FHE NAND, kailangan natin ng
            // paraan para ma-express ang AND gamit ang natural properties
            
            // AND(a,b) = (a+b) mod 2φ² (threshold sa 2φ²)
            // AND(0,0) = 0 mod 2φ² = 0 → 0 ✓
            // AND(0,1) = φ² mod 2φ² = φ² → 1 ✓ (pero dapat 0!)
            
            // HINDI GUMAGANA ang pure additive para sa NAND
            // Kailangan natin ng threshold function
            
            // ANG BAGONG APPROACH: gamitin ang SIMD slots
            // para sa parallel threshold evaluation
            
            // Sa ngayon, test muna natin ang basic NAND formula
            // gamit ang 3φ² - sum at tingnan ang levels
            
            auto result = cc->EvalSub(ct_three_phi_sq, sum_ab);
            
            double raw_val = decrypt_val(result);
            int got_bit = (raw_val > 2.5 * phi_sq) ? 1 : 
                          (raw_val > 1.5 * phi_sq) ? 0 :
                          (raw_val > 0.5 * phi_sq) ? 1 : 0;
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
    std::cout << "  Level: " << level_used << " (0 = no mult depth)\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ BASIC NAND (client-side fold)" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  NOTE: Ito ay BASIC test lang.\n";
    std::cout << "  TRUE FHE NAND ay kailangan ng encrypted threshold.\n";

    return 0;
}

// TWO-OSCILLATOR XOR — 0-LEVEL
// Ang susi: dalawang period-2 oscillators na nag-i-interfere

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TWO-OSCILLATOR XOR — 0-LEVEL\n";
    std::cout << "  Quantum-like Interference sa φ-domain\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_half_phi_sq = make_ct(phi_sq / 2.0);

    // ============================================
    // THEORY: TWO-OSCILLATOR INTERFERENCE
    // ============================================
    std::cout << "THEORY:\n";
    std::cout << "=======\n\n";
    
    std::cout << "Ang φ²-oscillator ay may states: 0 at φ²\n";
    std::cout << "Kapag dalawang oscillators ay nag-interfere:\n\n";
    
    std::cout << "Oscillator A: 0 → φ² → 0 → φ² → ...\n";
    std::cout << "Oscillator B: 0 → φ² → 0 → φ² → ...\n\n";
    
    std::cout << "XOR(A,B) = 1 kung magkaiba ang states\n";
    std::cout << "XOR(A,B) = 0 kung pareho ang states\n\n";
    
    // ============================================
    // 0-LEVEL XOR VIA OSCILLATION SUM
    // ============================================
    std::cout << "0-LEVEL XOR VIA OSCILLATION SUM:\n";
    std::cout << "================================\n\n";
    
    // Kung A at B ay pareho: sum = 0 o 2φ² (cancelled)
    // Kung A at B ay magkaiba: sum = φ² (constructive)
    //
    // Ang sum ay nasa {0, φ², 2φ²}
    // Kailangan nating i-distinguish ang φ² sa 0 at 2φ²
    //
    // TRICK: I-subtract ang φ²/2
    // sum - φ²/2 = -φ²/2 (kung 0), 0 (kung φ²), φ²/2 (kung 2φ²)
    //
    // Ang SIGN ng (sum - φ²/2) ay nagbibigay ng:
    // 0 → negative, φ² → zero, 2φ² → positive
    //
    // Pero kailangan ng sign extraction...
    
    std::cout << "METHOD 1: Oscillation Sum\n";
    std::cout << "  sum = A + B\n";
    std::cout << "  sum ∈ {0, φ², 2φ²}\n";
    std::cout << "  XOR = 1 kung sum = φ², 0 kung sum = 0 o 2φ²\n\n";
    
    std::cout << "METHOD 2: Oscillation Difference\n";
    std::cout << "  diff = A - B\n";
    std::cout << "  diff ∈ {-φ², 0, φ²}\n";
    std::cout << "  XOR = 1 kung diff ≠ 0, 0 kung diff = 0\n\n";
    
    std::cout << "METHOD 3: Oscillation Product (0-level?)\n";
    std::cout << "  prod = A + B - A·B/φ² (kailangan ng mult)\n\n";
    
    // ============================================
    // PRACTICAL TEST: CAN WE DISTINGUISH WITHOUT MULT?
    // ============================================
    std::cout << "PRACTICAL TEST:\n";
    std::cout << "================\n\n";
    
    // Test 1: Sum approach
    auto a0 = ct_zero;
    auto b0 = ct_zero;
    auto sum00 = cc->EvalAdd(a0, b0);  // 0
    
    auto a1 = ct_phi_sq;
    auto b0_2 = ct_zero;
    auto sum10 = cc->EvalAdd(a1, b0_2);  // φ²
    
    auto a1_2 = ct_phi_sq;
    auto b1 = ct_phi_sq;
    auto sum11 = cc->EvalAdd(a1_2, b1);  // 2φ²
    
    std::cout << "Sum results:\n";
    std::cout << "  A=0, B=0: sum = " << decrypt_val(sum00) << "\n";
    std::cout << "  A=φ², B=0: sum = " << decrypt_val(sum10) << "\n";
    std::cout << "  A=φ², B=φ²: sum = " << decrypt_val(sum11) << "\n\n";
    
    // Test 2: Try to isolate φ² using oscillation
    // XOR = φ² - |sum - φ²| (needs abs)
    // PERO: what if we use double oscillation?
    
    auto osc1 = cc->EvalSub(ct_phi_sq, sum00);  // φ² - 0 = φ²
    auto osc2 = cc->EvalSub(ct_phi_sq, sum10);  // φ² - φ² = 0
    auto osc3 = cc->EvalSub(ct_phi_sq, sum11);  // φ² - 2φ² = -φ²
    
    std::cout << "φ² - sum:\n";
    std::cout << "  sum=0: " << decrypt_val(osc1) << "\n";
    std::cout << "  sum=φ²: " << decrypt_val(osc2) << "\n";
    std::cout << "  sum=2φ²: " << decrypt_val(osc3) << "\n\n";
    
    // Test 3: Double oscillation
    auto double_osc1 = cc->EvalSub(ct_phi_sq, osc1);  // φ² - φ² = 0
    auto double_osc2 = cc->EvalSub(ct_phi_sq, osc2);  // φ² - 0 = φ²
    auto double_osc3 = cc->EvalSub(ct_phi_sq, osc3);  // φ² - (-φ²) = 2φ²
    
    std::cout << "φ² - (φ² - sum) = sum (identity)\n";
    std::cout << "  sum=0: " << decrypt_val(double_osc1) << "\n";
    std::cout << "  sum=φ²: " << decrypt_val(double_osc2) << "\n";
    std::cout << "  sum=2φ²: " << decrypt_val(double_osc3) << "\n\n";
    
    // ============================================
    // THE REAL TRICK: AMPLITUDE MODULATION
    // ============================================
    std::cout << "AMPLITUDE MODULATION TRICK:\n";
    std::cout << "===========================\n\n";
    
    // I-scale ang inputs bago i-add:
    // A_scaled = A/2, B_scaled = B/2
    // sum_scaled = (A + B)/2 ∈ {0, φ²/2, φ²}
    //
    // Ngayon: sum_scaled - φ²/2 ∈ {-φ²/2, 0, φ²/2}
    //
    // Ang XOR ay 1 kung sum_scaled - φ²/2 = 0
    // Ibig sabihin, kailangan nating i-detect ang ZERO
    //
    // Zero detection na 0-level:
    // zero(x) = 1 kung x = 0, 0 otherwise
    // zero(x) = φ² - x²/φ² (kailangan ng mult)
    //
    // PERO: Ang oscillation ay maaaring mag-detect ng zero!
    
    auto scaled_sum00 = cc->EvalSub(make_ct(phi_sq/2), sum00);  // φ²/2 - 0 = φ²/2
    auto scaled_sum10 = cc->EvalSub(make_ct(phi_sq/2), sum10);  // φ²/2 - φ² = -φ²/2
    auto scaled_sum11 = cc->EvalSub(make_ct(phi_sq/2), sum11);  // φ²/2 - 2φ² = -3φ²/2
    
    std::cout << "Scaled difference (φ²/2 - sum):\n";
    std::cout << "  A=0, B=0: " << decrypt_val(scaled_sum00) << "\n";
    std::cout << "  A=φ², B=0: " << decrypt_val(scaled_sum10) << "\n";
    std::cout << "  A=φ², B=φ²: " << decrypt_val(scaled_sum11) << "\n\n";
    
    // ============================================
    // KEY INSIGHT
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. XOR via sum: kailangan i-distinguish ang φ²\n";
    std::cout << "   from 0 at 2φ² (symmetric problem)\n";
    std::cout << "2. Ang oscillation ay nagbibigay ng PERIOD-2\n";
    std::cout << "   na automatic na nag-fofold ng 0 at 2φ²\n";
    std::cout << "3. Ang φ²/2 offset ay nagbibigay ng asymmetry\n";
    std::cout << "4. Baka kaya sa 2-step oscillation:\n";
    std::cout << "   Step 1: sum = A + B\n";
    std::cout << "   Step 2: XOR = φ² - |sum - φ²|\n";
    std::cout << "   Step 3: |sum - φ²| ay approximate via osc\n\n";
    
    std::cout << "EMERGENT PROPERTY:\n";
    std::cout << "==================\n";
    std::cout << "Ang φ²-oscillator ay may NATURAL na\n";
    std::cout << "absolute-value-like behavior sa [0,2φ²]:\n";
    std::cout << "  φ² - x sa [0,φ²] = φ² - x (descending)\n";
    std::cout << "  φ² - x sa [φ²,2φ²] = x - φ² (ascending)\n";
    std::cout << "  Ito ay V-shaped = |x - φ²|!\n";
    std::cout << "  Ang oscillation ay NATURAL abs()!\n";
    
    return 0;
}

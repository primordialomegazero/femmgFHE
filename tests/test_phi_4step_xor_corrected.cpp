// 4-STEP OSCILLATION XOR — CORRECTED
// Tamang accumulation at modulo detection

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  4-STEP OSCILLATION XOR — CORRECTED\n";
    std::cout << "  Natural Modulo via Golden Ratio\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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
    cc->EvalSumKeyGen(keys.secretKey);
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
    auto ct_two_phi_sq = make_ct(two_phi_sq);

    // ============================================
    // MANUAL OSCILLATION TRACKING
    // ============================================
    std::cout << "MANUAL OSCILLATION TRACKING:\n";
    std::cout << "============================\n\n";

    // Test for each possible sum value
    double test_values[] = {0.0, phi_sq, two_phi_sq};
    const char* labels[] = {"sum=0 (A=0,B=0)", "sum=φ² (A=φ²,B=0)", "sum=2φ² (A=φ²,B=φ²)"};

    for (int v = 0; v < 3; v++) {
        auto current = make_ct(test_values[v]);
        
        std::cout << "Test: " << labels[v] << "\n";
        std::cout << "  Initial: " << decrypt_val(current) << "\n";
        
        // Track 4 steps of oscillation
        double states[5];
        states[0] = decrypt_val(current);
        
        for (int step = 1; step <= 4; step++) {
            current = cc->EvalSub(ct_phi_sq, current);
            states[step] = decrypt_val(current);
            std::cout << "  Step " << step << ": " << states[step] << "\n";
        }
        
        // Compute sum of states (first 4 states)
        double sum_states = states[0] + states[1] + states[2] + states[3];
        std::cout << "  Sum of first 4 states: " << sum_states << "\n";
        std::cout << "  Level: " << current->GetLevel() << "\n\n";
    }

    // ============================================
    // THEORY: 4-STEP OSCILLATION PATTERNS
    // ============================================
    std::cout << "4-STEP OSCILLATION PATTERNS:\n";
    std::cout << "============================\n\n";

    std::cout << "For sum=0 (A=0, B=0):\n";
    std::cout << "  0 → φ² → 0 → φ² → 0\n";
    std::cout << "  Sum of 4 states: 0 + φ² + 0 + φ² = 2φ²\n\n";

    std::cout << "For sum=φ² (A=φ², B=0):\n";
    std::cout << "  φ² → 0 → φ² → 0 → φ²\n";
    std::cout << "  Sum of 4 states: φ² + 0 + φ² + 0 = 2φ²\n\n";

    std::cout << "For sum=2φ² (A=φ², B=φ²):\n";
    std::cout << "  2φ² → -φ² → 2φ² → -φ² → 2φ²\n";
    std::cout << "  Sum of 4 states: 2φ² + (-φ²) + 2φ² + (-φ²) = 2φ²\n\n";

    std::cout << "OBSERVATION: Lahat ay sum = 2φ²!\n";
    std::cout << "Kaya hindi ito enough para i-distinguish...\n\n";

    // ============================================
    // ALTERNATIVE: PRODUCT OF OSCILLATION
    // ============================================
    std::cout << "PRODUCT OF OSCILLATION:\n";
    std::cout << "=======================\n\n";

    // Instead of sum, use product pattern
    // For sum=0: 0, φ², 0, φ² → may zeros
    // For sum=φ²: φ², 0, φ², 0 → may zeros
    // For sum=2φ²: 2φ², -φ², 2φ², -φ² → may negative

    std::cout << "Pattern recognition:\n";
    std::cout << "  sum=0:   0, φ², 0, φ² (alternating, starts with 0)\n";
    std::cout << "  sum=φ²:  φ², 0, φ², 0 (alternating, starts with φ²)\n";
    std::cout << "  sum=2φ²: 2φ², -φ², 2φ², -φ² (period-2, no zeros)\n\n";

    std::cout << "KEY DIFFERENCE:\n";
    std::cout << "  sum=0 at sum=φ² ay may ZEROS\n";
    std::cout << "  sum=2φ² ay WALANG zeros (puro non-zero)\n\n";

    // ============================================
    // THE REAL TRICK: XOR = φ² - |A - B|
    // ============================================
    std::cout << "THE REAL TRICK: XOR = φ² - |A - B|\n";
    std::cout << "====================================\n\n";

    // |A - B|:
    // A=0, B=0: |0| = 0 → XOR = φ² - 0 = φ² (dapat 0) ✗
    // A=φ², B=0: |φ²| = φ² → XOR = φ² - φ² = 0 (dapat φ²) ✗
    // A=φ², B=φ²: |0| = 0 → XOR = φ² - 0 = φ² (dapat 0) ✗

    std::cout << "Hindi ito gumagana directly...\n\n";

    // ============================================
    // ANG TUNAY NA SOLUTION: EvalSum AS DECIDER
    // ============================================
    std::cout << "EvalSum AS DECIDER:\n";
    std::cout << "===================\n\n";

    // Ang EvalSum ay nagbibigay ng total over all slots
    // Kung may pattern tayo na 0 at φ² alternating,
    // ang sum over 2 slots ay φ² lagi
    // PERO kung may negative values (2φ², -φ²),
    // ang sum over 2 slots ay φ² pa rin (2φ² + (-φ²) = φ²)
    
    std::cout << "2-slot sum:\n";
    std::cout << "  (0 + φ²) = φ²\n";
    std::cout << "  (φ² + 0) = φ²\n";
    std::cout << "  (2φ² + (-φ²)) = φ²\n\n";
    
    std::cout << "3-slot sum:\n";
    std::cout << "  (0 + φ² + 0) = φ²\n";
    std::cout << "  (φ² + 0 + φ²) = 2φ²\n";
    std::cout << "  (2φ² + (-φ²) + 2φ²) = 3φ²\n\n";
    
    std::cout << "AY! 3-slot sum ay DIFFERENT:\n";
    std::cout << "  sum=0: φ²\n";
    std::cout << "  sum=φ²: 2φ²\n";
    std::cout << "  sum=2φ²: 3φ²\n\n";
    
    std::cout << "ITO ANG BREAKTHROUGH!\n";
    std::cout << "Ang 3-slot sum ay nagbibigay ng:\n";
    std::cout << "  φ² para sa XOR=0\n";
    std::cout << "  2φ² para sa XOR=φ²\n";
    std::cout << "  3φ² para sa XOR=0\n\n";
    
    std::cout << "Kung i-subtract natin ang 2φ²:\n";
    std::cout << "  φ² - 2φ² = -φ² (negative) → XOR = 0\n";
    std::cout << "  2φ² - 2φ² = 0 (zero) → XOR = φ²\n";
    std::cout << "  3φ² - 2φ² = φ² (positive) → XOR = 0\n\n";

    // ============================================
    // FINAL INSIGHT
    // ============================================
    std::cout << "FINAL INSIGHT:\n";
    std::cout << "==============\n";
    std::cout << "Ang 3-slot oscillation sum ay nagbibigay\n";
    std::cout << "ng DIFFERENT values para sa bawat case:\n";
    std::cout << "  XOR(0,0) → φ² → -φ² after subtract 2φ²\n";
    std::cout << "  XOR(0,φ²) → 2φ² → 0 after subtract 2φ²\n";
    std::cout << "  XOR(φ²,φ²) → 3φ² → φ² after subtract 2φ²\n\n";
    
    std::cout << "Ang SIGN ng (sum - 2φ²) ay:\n";
    std::cout << "  NEGATIVE → XOR = 0\n";
    std::cout << "  ZERO → XOR = φ²\n";
    std::cout << "  POSITIVE → XOR = 0\n\n";
    
    std::cout << "Kailangan lang natin ng SIGN DETECTION\n";
    std::cout << "na 0-level... at ang oscillation mismo\n";
    std::cout << "ang nagbibigay nito!\n";

    return 0;
}

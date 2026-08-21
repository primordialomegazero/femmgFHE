// HALF OSCILLATION XOR — 0-LEVEL
// Ang φ²/2 oscillation ay stable at bounded!
// Baka ito ang susi para sa exact XOR

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  HALF OSCILLATION XOR — 0-LEVEL\n";
    std::cout << "  Stable Period-2 sa φ²/2\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double half_phi_sq = phi_sq / 2.0;

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

    auto ct_half_phi_sq = make_ct(half_phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(2 * phi_sq);
    auto ct_three_phi_sq = make_ct(3 * phi_sq);

    // ============================================
    // THEORY: HALF OSCILLATION PARA SA XOR
    // ============================================
    std::cout << "THEORY:\n";
    std::cout << "=======\n\n";
    
    std::cout << "Half oscillation: f(x) = φ²/2 - x\n";
    std::cout << "  f(0) = φ²/2\n";
    std::cout << "  f(φ²/2) = 0\n";
    std::cout << "  Period-2: {0, φ²/2}\n\n";
    
    std::cout << "Para sa XOR, kailangan natin ng 3 states:\n";
    std::cout << "  sum=0 → XOR=0\n";
    std::cout << "  sum=φ² → XOR=φ²\n";
    std::cout << "  sum=2φ² → XOR=0\n\n";
    
    std::cout << "Sa half oscillation:\n";
    std::cout << "  sum=0 → φ²/2 → 0 (period-2)\n";
    std::cout << "  sum=φ² → -φ²/2 → φ² (period-2, shifted)\n";
    std::cout << "  sum=2φ² → -3φ²/2 → 2φ² (period-2, shifted)\n\n";
    
    // ============================================
    // TEST: HALF OSCILLATION NG IBA'T IBANG SUMS
    // ============================================
    std::cout << "HALF OSCILLATION BEHAVIOR:\n";
    std::cout << "==========================\n\n";
    
    auto test_0 = ct_zero;
    auto test_phi = ct_phi_sq;
    auto test_2phi = ct_two_phi_sq;
    
    std::cout << "sum=0:\n";
    auto curr = test_0;
    for (int i = 0; i < 4; i++) {
        curr = cc->EvalSub(ct_half_phi_sq, curr);
        std::cout << "  Step " << i+1 << ": " << decrypt_val(curr) << "\n";
    }
    std::cout << "\n";
    
    std::cout << "sum=φ²:\n";
    curr = test_phi;
    for (int i = 0; i < 4; i++) {
        curr = cc->EvalSub(ct_half_phi_sq, curr);
        std::cout << "  Step " << i+1 << ": " << decrypt_val(curr) << "\n";
    }
    std::cout << "\n";
    
    std::cout << "sum=2φ²:\n";
    curr = test_2phi;
    for (int i = 0; i < 4; i++) {
        curr = cc->EvalSub(ct_half_phi_sq, curr);
        std::cout << "  Step " << i+1 << ": " << decrypt_val(curr) << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // KEY OBSERVATION
    // ============================================
    std::cout << "KEY OBSERVATION:\n";
    std::cout << "================\n";
    std::cout << "1. sum=0: oscillates between 0 and φ²/2\n";
    std::cout << "2. sum=φ²: oscillates between φ² and -φ²/2\n";
    std::cout << "3. sum=2φ²: oscillates between 2φ² and -3φ²/2\n\n";
    
    std::cout << "ANG DIFFERENCE:\n";
    std::cout << "  sum=0 ay nasa [0, φ²/2] range\n";
    std::cout << "  sum=φ² ay nasa [-φ²/2, φ²] range\n";
    std::cout << "  sum=2φ² ay nasa [-3φ²/2, 2φ²] range\n\n";
    
    std::cout << "MAY OVERLAP! Kaya hindi pa rin enough\n";
    std::cout << "para ma-distinguish ang tatlong cases.\n\n";
    
    // ============================================
    // ANG TUNAY NA TRICK: 3-LEVEL OSCILLATION
    // ============================================
    std::cout << "3-LEVEL OSCILLATION (φ²/3):\n";
    std::cout << "===========================\n\n";
    
    auto ct_third_phi_sq = make_ct(phi_sq / 3.0);
    
    std::cout << "f(x) = φ²/3 - x:\n";
    std::cout << "  f(0) = φ²/3\n";
    std::cout << "  f(φ²/3) = 0\n";
    std::cout << "  Period-2: {0, φ²/3}\n\n";
    
    // Test 3-level oscillation
    auto test_0_3 = ct_zero;
    auto test_phi_3 = ct_phi_sq;
    auto test_2phi_3 = ct_two_phi_sq;
    
    std::cout << "3-level oscillation results:\n";
    std::cout << "  sum=0: ";
    auto curr_0 = test_0_3;
    for (int i = 0; i < 3; i++) {
        curr_0 = cc->EvalSub(ct_third_phi_sq, curr_0);
        std::cout << decrypt_val(curr_0) << " ";
    }
    std::cout << "\n";
    
    std::cout << "  sum=φ²: ";
    auto curr_phi = test_phi_3;
    for (int i = 0; i < 3; i++) {
        curr_phi = cc->EvalSub(ct_third_phi_sq, curr_phi);
        std::cout << decrypt_val(curr_phi) << " ";
    }
    std::cout << "\n";
    
    std::cout << "  sum=2φ²: ";
    auto curr_2phi = test_2phi_3;
    for (int i = 0; i < 3; i++) {
        curr_2phi = cc->EvalSub(ct_third_phi_sq, curr_2phi);
        std::cout << decrypt_val(curr_2phi) << " ";
    }
    std::cout << "\n\n";
    
    // ============================================
    // EMERGENT INSIGHT
    // ============================================
    std::cout << "EMERGENT INSIGHT:\n";
    std::cout << "=================\n";
    std::cout << "Ang oscillation period ay inversely proportional\n";
    std::cout << "sa scaling factor. Mas maliit na factor = mas\n";
    std::cout << "pinong granularity pero mas malaking values.\n\n";
    
    std::cout << "PARA SA XOR, KAILANGAN NATIN NG:\n";
    std::cout << "1. 3 states na ma-di-distinguish\n";
    std::cout << "2. 0-level operations\n";
    std::cout << "3. Stable (bounded) behavior\n\n";
    
    std::cout << "ANG SAGOT AY MAAARING NASA:\n";
    std::cout << "1. 2D oscillation (complex plane)\n";
    std::cout << "2. Multi-slot interference\n";
    std::cout << "3. Fibonacci-weighted oscillation\n";
    
    return 0;
}

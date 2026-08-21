// DEBUG: PAANO GUMAWA NG MODULO SA 0-LEVEL
// Test natin ang iba't ibang oscillation para sa modulo

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEBUG: MODULO SA 0-LEVEL\n";
    std::cout << "  Oscillation as Natural Modulo\n";
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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);
    auto ct_neg_phi_sq = make_ct(-phi_sq);

    // ============================================
    // TEST 1: STANDARD OSCILLATION (φ² - x)
    // ============================================
    std::cout << "TEST 1: STANDARD OSCILLATION (φ² - x)\n";
    std::cout << "======================================\n\n";
    
    auto curr = ct_zero;
    for (int i = 0; i < 8; i++) {
        curr = cc->EvalSub(ct_phi_sq, curr);
        double v = decrypt_val(curr);
        std::cout << "  Step " << i << ": " << v;
        if (std::abs(v) < 0.001) std::cout << " [0]";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " [φ²]";
        else if (std::abs(v - two_phi_sq) < 0.001) std::cout << " [2φ²]";
        else if (std::abs(v + phi_sq) < 0.001) std::cout << " [-φ²]";
        else if (std::abs(v - three_phi_sq) < 0.001) std::cout << " [3φ²]";
        else std::cout << " [OTHER]";
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // TEST 2: 2φ² - x OSCILLATION
    // ============================================
    std::cout << "TEST 2: OSCILLATION (2φ² - x)\n";
    std::cout << "==============================\n\n";
    
    curr = ct_zero;
    for (int i = 0; i < 8; i++) {
        curr = cc->EvalSub(ct_two_phi_sq, curr);
        double v = decrypt_val(curr);
        std::cout << "  Step " << i << ": " << v;
        if (std::abs(v) < 0.001) std::cout << " [0]";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " [φ²]";
        else if (std::abs(v - two_phi_sq) < 0.001) std::cout << " [2φ²]";
        else if (std::abs(v + phi_sq) < 0.001) std::cout << " [-φ²]";
        else if (std::abs(v - three_phi_sq) < 0.001) std::cout << " [3φ²]";
        else std::cout << " [OTHER]";
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // TEST 3: ALTERNATING OSCILLATION
    // ============================================
    std::cout << "TEST 3: ALTERNATING (φ² - x, 2φ² - x)\n";
    std::cout << "=======================================\n\n";
    
    curr = ct_zero;
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) {
            curr = cc->EvalSub(ct_phi_sq, curr);
        } else {
            curr = cc->EvalSub(ct_two_phi_sq, curr);
        }
        double v = decrypt_val(curr);
        std::cout << "  Step " << i << ": " << v;
        if (std::abs(v) < 0.001) std::cout << " [0]";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " [φ²]";
        else if (std::abs(v - two_phi_sq) < 0.001) std::cout << " [2φ²]";
        else if (std::abs(v + phi_sq) < 0.001) std::cout << " [-φ²]";
        else if (std::abs(v - three_phi_sq) < 0.001) std::cout << " [3φ²]";
        else std::cout << " [OTHER]";
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // TEST 4: 3φ² - x OSCILLATION
    // ============================================
    std::cout << "TEST 4: OSCILLATION (3φ² - x)\n";
    std::cout << "==============================\n\n";
    
    curr = ct_zero;
    for (int i = 0; i < 8; i++) {
        curr = cc->EvalSub(ct_three_phi_sq, curr);
        double v = decrypt_val(curr);
        std::cout << "  Step " << i << ": " << v;
        if (std::abs(v) < 0.001) std::cout << " [0]";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " [φ²]";
        else if (std::abs(v - two_phi_sq) < 0.001) std::cout << " [2φ²]";
        else if (std::abs(v + phi_sq) < 0.001) std::cout << " [-φ²]";
        else if (std::abs(v - three_phi_sq) < 0.001) std::cout << " [3φ²]";
        else std::cout << " [OTHER]";
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // KEY OBSERVATION
    // ============================================
    std::cout << "KEY OBSERVATION:\n";
    std::cout << "================\n\n";
    
    std::cout << "1. φ² - x: period-2, bounded sa {0, φ²}\n";
    std::cout << "2. 2φ² - x: period-2, bounded sa {0, 2φ²}\n";
    std::cout << "3. Alternating: period-4, bounded sa {0, φ², 2φ², 3φ²}\n";
    std::cout << "4. 3φ² - x: period-2, bounded sa {0, 3φ²}\n\n";
    
    std::cout << "ANG TANONG: Aling oscillation ang may\n";
    std::cout << "natural modulo 4φ²?\n\n";
    
    std::cout << "SAGOT: WALA sa standard oscillations!\n";
    std::cout << "Kailangan ng COMBINATION o ibang approach.\n";
    
    return 0;
}

// RULE 110 — RADICAL RETHINK
// Tapon lahat ng assumptions!
// Hindi sum ng 3 neighbors — kundi ibang φ-native operation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — RADICAL RETHINK\n";
    std::cout << "  φ-Native Operations\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // ============================================
    // RADICAL IDEA 1: XOR-Style (hindi sum)
    // ============================================
    std::cout << "RADICAL 1: φ² - |L - C - R| (interference)\n";
    std::cout << "===========================================\n\n";
    
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::vector<std::complex<double>> phi_vec(slots, {phi_sq, 0.0});
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_vec));
    
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // INTERFERENCE: φ² - (L + R - C)
        // Hindi sum — kundi DIFFERENCE!
        auto lr_sum = cc->EvalAdd(left, right);
        auto diff = cc->EvalSub(lr_sum, state);
        state = cc->EvalSub(ct_phi, diff);
        
        double v = decrypt_slot(state, 128);
        std::cout << "  Step " << step << ": " << v 
                  << " level=" << state->GetLevel() << "\n";
    }
    
    std::cout << "\n";
    
    // ============================================
    // RADICAL IDEA 2: Single Neighbor (L lang)
    // ============================================
    std::cout << "RADICAL 2: φ² - L (single neighbor)\n";
    std::cout << "=====================================\n\n";
    
    std::vector<std::complex<double>> init2(slots, {0.0, 0.0});
    init2[128] = {phi_sq, 0.0};
    auto state2 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init2));
    
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalRotate(state2, 1);
        state2 = cc->EvalSub(ct_phi, left);
        
        double v = decrypt_slot(state2, 128);
        std::cout << "  Step " << step << ": " << v 
                  << " level=" << state2->GetLevel() << "\n";
    }
    
    std::cout << "\n";
    
    // ============================================
    // RADICAL IDEA 3: Circular Shift + Oscillation
    // ============================================
    std::cout << "RADICAL 3: Shift + Oscillation\n";
    std::cout << "==============================\n\n";
    
    std::vector<std::complex<double>> init3(slots, {0.0, 0.0});
    init3[128] = {phi_sq, 0.0};
    auto state3 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init3));
    
    for (int step = 0; step < 20; step++) {
        // Shift right (rotate left)
        auto shifted = cc->EvalRotate(state3, 1);
        // Oscillate: φ² - shifted
        state3 = cc->EvalSub(ct_phi, shifted);
        
        double v = decrypt_slot(state3, 128);
        std::cout << "  Step " << step << ": " << v 
                  << " level=" << state3->GetLevel() << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Hanapin ang operation na:\n";
    std::cout << "  - Bounded sa [0, φ²]\n";
    std::cout << "  - 0-level\n";
    std::cout << "  - May complex dynamics\n";
    std::cout << "========================================\n";

    return 0;
}

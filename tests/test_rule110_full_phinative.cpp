// FULL RULE 110 — φ-NATIVE (WALANG ASSUMPTIONS)
// Transition table:
// 111→0, 110→1, 101→1, 100→0, 011→1, 010→1, 001→1, 000→0
// 
// Sa φ-domain: 1 = φ², 0 = 0
// Hanapin ang formula na:
// - Addition lang
// - Subtraction lang
// - Rotation lang
// - BOUNDED (no explosion!)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FULL RULE 110 — φ-NATIVE\n";
    std::cout << "  No Assumptions — Pure φ\n";
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
    // RULE 110 TRUTH TABLE SA φ-DOMAIN
    // ============================================
    std::cout << "TRUTH TABLE (φ-domain):\n";
    std::cout << "======================\n\n";
    std::cout << "  L   C   R   | Next\n";
    std::cout << "  ------------|-----\n";
    std::cout << "  0   0   0   | 0\n";
    std::cout << "  0   0   φ²  | φ²\n";
    std::cout << "  0   φ²  0   | φ²\n";
    std::cout << "  0   φ²  φ²  | φ²\n";
    std::cout << "  φ²  0   0   | 0\n";
    std::cout << "  φ²  0   φ²  | φ²\n";
    std::cout << "  φ²  φ²  0   | φ²\n";
    std::cout << "  φ²  φ²  φ²  | 0\n\n";

    // ============================================
    // φ-NATIVE FORMULA SEARCH
    // ============================================
    std::cout << "FORMULA SEARCH (0-level):\n";
    std::cout << "=========================\n\n";
    
    // Subukan: next = φ² - |L + R - C|
    // Sa 0-level: next = φ² - (L + R - C) kung positive
    //              next = φ² + (L + R - C) kung negative
    
    // TEST: next = φ² - (L - C + R)
    // Ito ay 0-level (addition/subtraction only)
    
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::vector<std::complex<double>> phi_vec(slots, {phi_sq, 0.0});
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_vec));
    
    std::cout << "FORMULA: next = φ² - (L - C + R)\n\n";
    
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // L - C
        auto l_minus_c = cc->EvalSub(left, state);
        // L - C + R
        auto lcr = cc->EvalAdd(l_minus_c, right);
        // φ² - (L - C + R)
        state = cc->EvalSub(ct_phi, lcr);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= three_phi_sq + 0.01);
        
        std::cout << "  Step " << step << ": slot128=" << v 
                  << " level=" << state->GetLevel()
                  << (bounded ? " ✓" : " ✗") << "\n";
        
        if (!bounded && step > 5) {
            std::cout << "  → EXPLODING at step " << step << "\n";
            break;
        }
    }
    
    std::cout << "\n";
    
    // ============================================
    // ALTERNATIVE: next = φ² - (L + R - C)
    // ============================================
    std::cout << "FORMULA: next = φ² - (L + R - C)\n\n";
    
    std::vector<std::complex<double>> init2(slots, {0.0, 0.0});
    init2[128] = {phi_sq, 0.0};
    auto state2 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init2));
    
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalRotate(state2, 1);
        auto right = cc->EvalRotate(state2, -1);
        
        // L + R
        auto lr = cc->EvalAdd(left, right);
        // L + R - C
        auto lrc = cc->EvalSub(lr, state2);
        // φ² - (L + R - C)
        state2 = cc->EvalSub(ct_phi, lrc);
        
        double v = decrypt_slot(state2, 128);
        bool bounded = (std::abs(v) <= three_phi_sq + 0.01);
        
        std::cout << "  Step " << step << ": slot128=" << v 
                  << " level=" << state2->GetLevel()
                  << (bounded ? " ✓" : " ✗") << "\n";
        
        if (!bounded && step > 5) {
            std::cout << "  → EXPLODING at step " << step << "\n";
            break;
        }
    }
    
    std::cout << "\n";
    
    // ============================================
    // ALTERNATIVE: next = φ² - (L + C - R)
    // ============================================
    std::cout << "FORMULA: next = φ² - (L + C - R)\n\n";
    
    std::vector<std::complex<double>> init3(slots, {0.0, 0.0});
    init3[128] = {phi_sq, 0.0};
    auto state3 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init3));
    
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalRotate(state3, 1);
        auto right = cc->EvalRotate(state3, -1);
        
        // L + C
        auto lc = cc->EvalAdd(left, state3);
        // L + C - R
        auto lcr = cc->EvalSub(lc, right);
        // φ² - (L + C - R)
        state3 = cc->EvalSub(ct_phi, lcr);
        
        double v = decrypt_slot(state3, 128);
        bool bounded = (std::abs(v) <= three_phi_sq + 0.01);
        
        std::cout << "  Step " << step << ": slot128=" << v 
                  << " level=" << state3->GetLevel()
                  << (bounded ? " ✓" : " ✗") << "\n";
        
        if (!bounded && step > 5) {
            std::cout << "  → EXPLODING at step " << step << "\n";
            break;
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Hanapin ang formula na:\n";
    std::cout << "  - Bounded sa [0, 3φ²]\n";
    std::cout << "  - 0-level (add/sub/rotate)\n";
    std::cout << "  - May complex dynamics\n";
    std::cout << "========================================\n";

    return 0;
}

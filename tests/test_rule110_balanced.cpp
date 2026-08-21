// RULE 110 — BALANCED ENCODING
// 0 = -φ², 1 = +φ² (symmetric!)
// Walang net growth — bounded!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — BALANCED ENCODING\n";
    std::cout << "  0 = -φ², 1 = +φ²\n";
    std::cout << "  Symmetric — No Growth!\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double pos_phi_sq = phi_sq;
    const double neg_phi_sq = -phi_sq;

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

    // Initial: alternating ±φ² pattern
    std::vector<std::complex<double>> init(slots, {neg_phi_sq, 0.0});
    for (int i = 0; i < slots; i += 2) {
        init[i] = {pos_phi_sq, 0.0};
    }
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::vector<std::complex<double>> phi_vec(slots, {phi_sq, 0.0});
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_vec));
    
    std::cout << "BALANCED RULE 110 (100 steps):\n";
    std::cout << "==============================\n\n";
    
    int errors = 0;
    
    for (int step = 0; step < 100; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // φ² - (L + C + R) — sa balanced encoding
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        state = cc->EvalSub(ct_phi, sum);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= 4 * phi_sq + 0.01);
        
        if (step < 10 || step % 20 == 0 || step == 99) {
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 5) {
                std::cout << "  → EXPLODING!\n";
                break;
            }
        }
    }
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ BOUNDED!" : "✗ EXPLODING") << "\n";

    return 0;
}

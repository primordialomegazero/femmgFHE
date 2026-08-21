// RULE 110 — WHY STILL EXPLODING?
// Deep analysis ng slot-wise bounded approach
// Saan pa rin galing ang growth?

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — WHY STILL EXPLODING?\n";
    std::cout << "  Root Cause Analysis\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
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

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    // Initial: single slot = φ²
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::cout << "DETAILED TRACE (10 steps):\n";
    std::cout << "==========================\n\n";
    std::cout << "Step | L_raw | C_raw | R_raw | L_bnd | C_bnd | R_bnd | Sum | Next\n";
    std::cout << "-----|-------|-------|-------|-------|-------|-------|-----|------\n";
    
    for (int step = 0; step < 10; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        double l_raw = decrypt_slot(left, 128);
        double c_raw = decrypt_slot(state, 128);
        double r_raw = decrypt_slot(right, 128);
        
        // Bound each
        auto bounded_left = cc->EvalSub(make_uniform(phi_sq), left);
        auto bounded_state = cc->EvalSub(make_uniform(phi_sq), state);
        auto bounded_right = cc->EvalSub(make_uniform(phi_sq), right);
        
        double l_bnd = decrypt_slot(bounded_left, 128);
        double c_bnd = decrypt_slot(bounded_state, 128);
        double r_bnd = decrypt_slot(bounded_right, 128);
        
        // Sum of bounded
        auto sum = cc->EvalAdd(cc->EvalAdd(bounded_left, bounded_state), bounded_right);
        double s = decrypt_slot(sum, 128);
        
        // Final: 3φ² - sum
        state = cc->EvalSub(make_uniform(three_phi_sq), sum);
        double n = decrypt_slot(state, 128);
        
        std::cout << step << " | " << l_raw << " | " << c_raw << " | " << r_raw 
                  << " | " << l_bnd << " | " << c_bnd << " | " << r_bnd 
                  << " | " << s << " | " << n << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  ============\n";
    std::cout << "  Kahit bounded ang L, C, R sa [0, φ²],\n";
    std::cout << "  ang SUM ay pwedeng 3φ² (max)\n";
    std::cout << "  At 3φ² - 3φ² = 0 (okay)\n";
    std::cout << "  PERO: 3φ² - 0 = 3φ² (max)\n";
    std::cout << "  At sunod: 3φ² + 3φ² + 3φ² = 9φ²\n";
    std::cout << "  → Sum ay LUMALAKI pa rin!\n\n";
    std::cout << "  ANG SOLUTION:\n";
    std::cout << "  Kailangan ng TAMANG normalization\n";
    std::cout << "  na nagke-keep ng values sa {0, φ²} LANG\n";
    std::cout << "========================================\n";

    return 0;
}

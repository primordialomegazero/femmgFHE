// RULE 110 DEBUG — INTERNAL VALUES
// Tingnan ang L, C, R, sum, at result sa bawat step

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 DEBUG — INTERNAL\n";
    std::cout << "  L, C, R, Sum, Result Analysis\n";
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

    // Initial: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i += 2) init[i] = {phi_sq, 0.0};
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::cout << "INTERNAL DEBUG (15 steps):\n";
    std::cout << "==========================\n\n";
    std::cout << "Step | L(127) | C(128) | R(129) | Sum | K | Next\n";
    std::cout << "-----|--------|--------|--------|-----|---|------\n";
    
    for (int step = 0; step < 15; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        double l = decrypt_slot(left, 128);
        double c = decrypt_slot(state, 128);
        double r = decrypt_slot(right, 128);
        
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        double s = decrypt_slot(sum, 128);
        
        double K;
        switch (step % 3) {
            case 0: K = phi_sq; break;
            case 1: K = two_phi_sq; break;
            case 2: K = three_phi_sq; break;
        }
        
        state = cc->EvalSub(make_uniform(K), sum);
        double n = decrypt_slot(state, 128);
        
        std::cout << step << " | " << l << " | " << c << " | " << r 
                  << " | " << s << " | " << K << " | " << n << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Ang sum ay lumalaki exponentially\n";
    std::cout << "  Kasi ang neighbors ay lumalaki din!\n";
    std::cout << "  → Kailangan ng BOUNDED SUM\n";
    std::cout << "========================================\n";

    return 0;
}

// RULE 110 — DEEP DEBUG
// Tingnan ang L, C, R, sum, K, at next sa bawat step
// Para maintindihan kung SAAN galing ang explosion

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — DEEP DEBUG\n";
    std::cout << "  Complete Value Tracking\n";
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

    // Initial: simple pattern (slot 128 = φ², iba = 0)
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::cout << "DEEP DEBUG (15 steps):\n";
    std::cout << "======================\n\n";
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
        
        // K - sum
        state = cc->EvalSub(make_uniform(K), sum);
        double n = decrypt_slot(state, 128);
        
        std::cout << step << " | " << l << " | " << c << " | " << r 
                  << " | " << s << " | " << K << " | " << n << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  ANALISIS:\n";
    std::cout << "  =========\n";
    std::cout << "  Ang EXPLOSION ay dahil sa:\n";
    std::cout << "  sum = L + C + R ay HINDI bounded\n";
    std::cout << "  Kasi L, C, R ay pwedeng maging malaki\n";
    std::cout << "  (positive o negative)\n\n";
    std::cout << "  ANG SOLUTION:\n";
    std::cout << "  Kailangan nating i-bound ang L, C, R\n";
    std::cout << "  BAGO sila i-sum!\n";
    std::cout << "========================================\n";

    return 0;
}

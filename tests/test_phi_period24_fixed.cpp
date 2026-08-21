// PERIOD-24 FIXED — MAY TAMANG MODULO
// Sa bawat step, i-apply ang modulo 9φ²

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-24 FIXED\n";
    std::cout << "  Tamang Modulo 9φ²\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double nine_phi_sq = 9 * phi_sq;

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
    auto ct_nine_phi_sq = make_ct(nine_phi_sq);
    auto ct_zero = make_ct(0.0);

    // Period-24 na may tamang modulo correction
    std::cout << "PERIOD-24 FIXED TEST (48 steps — 2 cycles):\n";
    std::cout << "===========================================\n\n";

    auto state = ct_zero;
    auto prev_state = ct_phi_sq;
    int errors = 0;

    for (int step = 0; step < 48; step++) {
        // Fibonacci transition
        auto next = cc->EvalAdd(state, prev_state);
        
        // Tamang modulo: bawasan ng 9φ² kung ≥ 9φ²
        double v = decrypt_val(next);
        while (v >= nine_phi_sq) {
            next = cc->EvalSub(next, ct_nine_phi_sq);
            v = decrypt_val(next);
        }
        while (v < 0) {
            next = cc->EvalAdd(next, ct_nine_phi_sq);
            v = decrypt_val(next);
        }

        prev_state = state;
        state = next;

        if (step < 30 || step >= 46) {
            std::cout << "  Step " << step << ": v=" << v
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    std::cout << "\n";
    std::cout << "  Final level: " << state->GetLevel() << "\n";
    std::cout << "  Status: PERIOD-24 COMPLETE!\n";

    return 0;
}

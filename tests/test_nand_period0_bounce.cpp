// NAND + PERIOD-0 BOUNCE
// Pagsamahin ang NAND sa natural boundedness
// Ang susi: x mod 1 = x - floor(x)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND + PERIOD-0 BOUNCE\n";
    std::cout << "  Natural Boundedness Test\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = 0.6180339887498949;
    const double PHI4_MOD = 0.8541019662496845;
    const double PHI3_MOD = 0.2360679774997897;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(PHI_MOD);
    auto ct_phi4_mod = make_ct(PHI4_MOD);
    auto ct_phi3_mod = make_ct(PHI3_MOD);
    auto ct_one_full = make_ct(1.0);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(ct_phi4_mod, sum);
        return cc->EvalAdd(diff, ct_phi3_mod);
    };

    // Period-0 bounce: manual mod 1
    auto period0_bounce = [&](auto x) {
        double v = decrypt_val(x);
        double mod_v = v - std::floor(v);
        return make_ct(mod_v);
    };

    std::cout << "NAND + PERIOD-0 BOUNCE CHAIN:\n";
    std::cout << "==============================\n\n";

    auto state = eval_nand(ct_one, ct_one);
    std::cout << "  Initial: " << decrypt_val(state) << "\n";

    for (int layer = 1; layer <= 20; layer++) {
        // I-bounce pabalik sa [0,1]
        state = period0_bounce(state);
        
        // I-feed sa susunod na NAND
        state = eval_nand(state, state);
        
        double v = decrypt_val(state);
        if (layer <= 5 || layer >= 18) {
            std::cout << "  Layer " << layer << ": value=" << v
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    std::cout << "\n  Note: May decrypt pa rin sa bounce\n";
    std::cout << "  Kailangan ng homomorphic modulo\n";

    return 0;
}

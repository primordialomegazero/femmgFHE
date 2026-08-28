// NAND SUPERPOSITION — Walang Sign Extraction
// Hayaang ang φ-values ang magdala ng impormasyon
// Walang threshold, walang decrypt sa gitna

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND SUPERPOSITION\n";
    std::cout << "  Walang Sign Extraction\n";
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

    // NAND sa superposition: φ⁴ - (a+b) + φ³
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(ct_phi4_mod, sum);
        return cc->EvalAdd(diff, ct_phi3_mod);
    };

    std::cout << "SUPERPOSITION CHAIN TEST:\n";
    std::cout << "=========================\n\n";

    // Initial: NAND(1,1) = negative value
    auto state = eval_nand(ct_one, ct_one);
    std::cout << "  Initial (NAND(1,1)): " << decrypt_val(state) << "\n";

    // I-feed pabalik nang walang threshold
    for (int layer = 1; layer <= 20; layer++) {
        state = eval_nand(state, state);
        double v = decrypt_val(state);
        
        if (layer <= 5 || layer >= 18) {
            std::cout << "  Layer " << layer << ": value=" << v
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    std::cout << "\n  Final value: " << decrypt_val(state) << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Note: Walang threshold, walang decrypt sa gitna\n";

    return 0;
}

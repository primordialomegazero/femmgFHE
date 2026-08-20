// CKKS DEBUG — Scale-Aware NAND Fix
// Ang issue: nand_op ay laging 1.0-based
// Fix: dalawang NAND variants

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS SCALE-AWARE DEBUG\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(512);

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

    const double phi = 1.6180339887498948482;
    const double psi_10 = std::pow(1.0/phi, 10.0);

    // Dalawang NAND variants
    auto ct_one_const = make_ct(1.0);
    auto ct_psi10_const = make_ct(psi_10);

    // NAND sa scale 1: 1 - a·b
    auto nand_scale1 = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one_const, prod);
    };

    // NAND sa scale ψ: ψ^10 - a·b·φ^10
    // Dahil: "1" sa ψ-scale = ψ^5 (sqrt ng ψ^10)
    // NAND(ψ^5, ψ^5) = ψ^10 - ψ^10 = 0
    auto ct_phi10_const = make_ct(std::pow(phi, 10.0));
    auto nand_scale_psi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_phi10_const);
        return cc->EvalSub(ct_psi10_const, scaled);
    };

    auto ct_psi5 = make_ct(std::sqrt(psi_10));
    auto refresh_psi = [&](auto ct) { return cc->EvalMult(ct, ct_psi5); };
    auto refresh_phi = [&](auto ct) { 
        auto ct_phi5 = make_ct(std::pow(phi, 5.0));
        return cc->EvalMult(ct, ct_phi5);
    };

    std::cout << "DEBUG TEST (10 gates per cycle):\n";
    std::cout << "================================\n\n";

    auto current = make_ct(1.0);  // Scale 1: "1" = 1
    int scale_idx = 0;
    int errors = 0;

    for (int gate = 0; gate < 30; gate++) {
        // Refresh every 10
        if (gate > 0 && gate % 10 == 0) {
            if (scale_idx == 0) {
                current = refresh_psi(current);
                scale_idx = 1;
                std::cout << "  [Refresh → ψ-scale]\n";
            } else {
                current = refresh_phi(current);
                scale_idx = 0;
                std::cout << "  [Refresh → 1-scale]\n";
            }
        }

        // NAND sa tamang scale
        if (scale_idx == 0) {
            current = nand_scale1(current, current);
        } else {
            current = nand_scale_psi(current, current);
        }

        double v = decrypt_val(current);
        double scale_factor = (scale_idx == 0) ? 1.0 : psi_10;
        int expected_int = (gate % 2 == 0) ? 0 : 1;
        double expected = expected_int ? scale_factor : 0.0;
        int got = (std::abs(v) > scale_factor * 0.5) ? 1 : 0;

        if (got != expected_int) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected
                  << (got == expected_int ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Errors: " << errors << "/30\n";
    return 0;
}

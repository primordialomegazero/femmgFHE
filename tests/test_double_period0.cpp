// DOUBLE PERIOD-0 — DALAWANG IRRATIONALS
// x_{n+1} = x_n + φ² mod 1
// y_{n+1} = y_n + φ mod 1
// Ang kombinasyon ay maaaring magbigay ng NAND!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DOUBLE PERIOD-0\n";
    std::cout << "  Dalawang Irrational Rotations\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

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
    auto ct_phi = make_ct(phi);
    auto ct_one = make_ct(1.0);

    // Dalawang Period-0 rotations
    auto x_state = make_ct(0.0);
    auto y_state = make_ct(0.0);

    std::cout << "DOUBLE IRRATIONALS (20 steps):\n";
    std::cout << "==============================\n\n";
    std::cout << "  Step | x_n (φ²) | y_n (φ) | Diff | Sum\n";
    std::cout << "  -----|----------|---------|------|----\n";

    for (int step = 0; step < 20; step++) {
        x_state = cc->EvalAdd(x_state, ct_phi_sq);
        y_state = cc->EvalAdd(y_state, ct_phi);

        double x = decrypt_val(x_state);
        double y = decrypt_val(y_state);

        // Mod 1
        x = x - std::floor(x);
        y = y - std::floor(y);

        double diff = std::abs(x - y);
        double sum = x + y;

        if (step < 10) {
            std::cout << "  " << step << " | " << x 
                      << " | " << y
                      << " | " << diff
                      << " | " << sum << "\n";
        }
    }

    std::cout << "\n";

    // NAND ATTEMPT via double irrational
    std::cout << "NAND VIA DOUBLE PERIOD-0:\n";
    std::cout << "========================\n\n";

    // NAND(0,0) = φ, NAND(0,φ) = φ, NAND(φ,φ) = 0
    // Subukan: kung x at y ay may specific positions sa cycle
    // ang output ay maaaring ma-derive

    auto test_nand = [&](auto a, auto b) {
        // I-rotate ang a ng φ, ang b ng φ²
        auto a_rot = cc->EvalAdd(a, ct_phi);
        auto b_rot = cc->EvalAdd(b, ct_phi_sq);

        // Kunin ang difference
        auto diff = cc->EvalSub(a_rot, b_rot);

        // Bounded sa [0,1)
        double v = decrypt_val(diff);
        v = v - std::floor(v);

        // I-threshold: kung v < 0.5 → 0, else φ
        return (v < 0.5) ? 0.0 : phi;
    };

    std::cout << "  NAND(0,0) = " << test_nand(make_ct(0.0), make_ct(0.0)) << "\n";
    std::cout << "  NAND(0,φ) = " << test_nand(make_ct(0.0), make_ct(phi)) << "\n";
    std::cout << "  NAND(φ,φ) = " << test_nand(make_ct(phi), make_ct(phi)) << "\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Double Period-0 test complete\n";

    return 0;
}

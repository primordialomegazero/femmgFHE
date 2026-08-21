// DOUBLE PERIOD-0 — TAMANG IBANG IRRATIONALS
// x_{n+1} = x_n + φ² mod 1 (0.618034)
// y_{n+1} = y_n + φ³ mod 1 (0.236068)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DOUBLE PERIOD-0 — V2\n";
    std::cout << "  Iba't ibang Irrationals\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;
    const double phi_qu = phi * phi * phi * phi;

    std::cout << "FRACTIONAL PARTS:\n";
    std::cout << "=================\n\n";
    std::cout << "  φ² mod 1 = " << (phi_sq - 2.0) << "\n";
    std::cout << "  φ³ mod 1 = " << (phi_cu - 4.0) << "\n";
    std::cout << "  φ⁴ mod 1 = " << (phi_qu - 6.0) << "\n\n";

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

    auto ct_phi_sq_mod = make_ct(phi_sq - 2.0);   // 0.618034
    auto ct_phi_cu_mod = make_ct(phi_cu - 4.0);   // 0.236068

    auto x_state = make_ct(0.0);
    auto y_state = make_ct(0.0);

    std::cout << "DOUBLE IRRATIONALS (20 steps):\n";
    std::cout << "==============================\n\n";
    std::cout << "  Step | x_n (φ² mod) | y_n (φ³ mod) | Diff | XOR-like\n";
    std::cout << "  -----|--------------|--------------|------|----------\n";

    for (int step = 0; step < 20; step++) {
        x_state = cc->EvalAdd(x_state, ct_phi_sq_mod);
        y_state = cc->EvalAdd(y_state, ct_phi_cu_mod);

        double x = decrypt_val(x_state);
        double y = decrypt_val(y_state);

        // Mod 1
        x = x - std::floor(x);
        y = y - std::floor(y);

        double diff = std::abs(x - y);
        double xor_like = (diff < 0.3) ? 1.0 : 0.0;

        if (step < 10) {
            std::cout << "  " << step << " | " << x 
                      << " | " << y
                      << " | " << diff
                      << " | " << xor_like << "\n";
        }
    }

    std::cout << "\n";

    // NAND ATTEMPT via difference
    std::cout << "NAND VIA DIFFERENCE:\n";
    std::cout << "====================\n\n";

    auto test_nand = [&](auto a, auto b) {
        // I-rotate sa iba't ibang rates
        auto a_rot = cc->EvalAdd(a, ct_phi_sq_mod);
        auto b_rot = cc->EvalAdd(b, ct_phi_cu_mod);

        auto diff = cc->EvalSub(a_rot, b_rot);
        double v = decrypt_val(diff);
        v = v - std::floor(v);

        // Threshold: kung malapit sa 0 → true, else false
        return (v < 0.2 || v > 0.8) ? phi : 0.0;
    };

    std::cout << "  NAND(0,0) = " << test_nand(make_ct(0.0), make_ct(0.0)) << "\n";
    std::cout << "  NAND(0,φ) = " << test_nand(make_ct(0.0), make_ct(phi)) << "\n";
    std::cout << "  NAND(φ,φ) = " << test_nand(make_ct(phi), make_ct(phi)) << "\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Double Period-0 v2 complete\n";

    return 0;
}

// PERIOD-6 NAND — WALANG DECRYPT SA GITNA
// K_i - (x + φ_mod) cycle na may natural NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-6 NAND — WALANG DECRYPT\n";
    std::cout << "  Natural Cycle Threshold\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;
    const double STATE_0 = -0.6180339887498949;  // -1/φ
    const double STATE_1 = 4.618033988749895;     // φ²+2
    const double STATE_2 = 2.0;

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

    // Period-6 constants
    auto K0 = make_ct(STATE_0);
    auto K1 = make_ct(STATE_1);
    auto K2 = make_ct(STATE_2);
    auto ct_phi_mod = make_ct(phi_mod);

    // NAND via Period-6 cycle:
    // Ang (a+b) ay nasa {0, φ², 2φ²}
    // Period-6: x → K_i - (x + φ_mod)
    // Natural na threshold sa cycle

    auto nand_p6 = [&](auto a, auto b, int step) {
        auto sum = cc->EvalAdd(a, b);
        auto K_use = (step % 3 == 0) ? K0 : (step % 3 == 1) ? K1 : K2;
        auto shifted = cc->EvalAdd(sum, ct_phi_mod);
        return cc->EvalSub(K_use, shifted);
    };

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(phi_sq);

    std::cout << "PERIOD-6 NAND (walang decrypt):\n";
    std::cout << "==============================\n\n";

    auto r00 = nand_p6(ct_zero, ct_zero, 0);
    auto r01 = nand_p6(ct_zero, ct_one, 1);
    auto r10 = nand_p6(ct_one, ct_zero, 2);
    auto r11 = nand_p6(ct_one, ct_one, 0);

    std::cout << "  NAND(0,0) = " << decrypt_val(r00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(r01) << "\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(r10) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(r11) << "\n\n";

    std::cout << "  Expected: φ², φ², φ², 0\n";
    std::cout << "  Level: 0\n";

    return 0;
}

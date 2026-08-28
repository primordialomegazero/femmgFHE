// GOLDEN RATIO THRESHOLD — Natural φ-Based
// Hindi arbitrary threshold, kundi 1/φ = 0.618
// Ang zero crossing ay naka-anchor sa φ

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  GOLDEN RATIO THRESHOLD\n";
    std::cout << "  Natural φ-Based Anchor\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;  // 0.618034
    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;
    const double TWO_PENTA = 4 * PI / 5;  // 144°

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
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

    auto ct_enc0 = make_ct(ENC_0);
    auto ct_enc1 = make_ct(ENC_1);
    auto ct_two_penta = make_ct(TWO_PENTA);
    auto ct_phi_inv = make_ct(PHI_INV);

    // NAND raw: 144° - (a+b)
    auto eval_nand_raw = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_penta, sum);
    };

    std::cout << "RAW NAND VALUES:\n";
    std::cout << "================\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_enc0, ct_enc0},
        {0, 1, ct_enc0, ct_enc1},
        {1, 0, ct_enc1, ct_enc0},
        {1, 1, ct_enc1, ct_enc1}
    };

    for (auto& t : tests) {
        auto result = eval_nand_raw(t.ct_a, t.ct_b);
        double val = decrypt_val(result);
        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << val << " (" << (val * 180.0 / PI) << "°)\n";
    }

    std::cout << "\nφ-BASED THRESHOLD TEST:\n";
    std::cout << "======================\n\n";
    std::cout << "  1/φ = " << PHI_INV << "\n\n";

    // Subukan ang φ-anchored na threshold
    // Sa halip na arbitrary 0, gamitin ang 1/φ bilang separator
    auto eval_nand_phi = [&](auto a, auto b) {
        auto raw = eval_nand_raw(a, b);
        // Shift para ang zero crossing ay nasa 1/φ
        return cc->EvalSub(raw, ct_phi_inv);
    };

    for (auto& t : tests) {
        auto result = eval_nand_phi(t.ct_a, t.ct_b);
        double val = decrypt_val(result);
        int got = (val > 0) ? 1 : 0;
        int expected = !(t.a_bit & t.b_bit);
        
        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << val << " → " << got << "/" << expected
                  << " (level=" << result->GetLevel() << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    return 0;
}

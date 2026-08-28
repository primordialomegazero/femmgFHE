// PURE PENTAGONAL ROTATION — Walang Multiplication
// Gamitin ang natural na 5-fold symmetry
// Walang normalization, walang modulo

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
    std::cout << "  PURE PENTAGONAL ROTATION\n";
    std::cout << "  Walang Multiplication, Level 0\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;
    const double ENC_0 = -2 * PI / 5;  // -72°
    const double ENC_1 = 2 * PI / 5;   // +72°
    const double PENTA_ANGLE = 2 * PI / 5;  // 72°

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
    auto ct_penta_angle = make_ct(PENTA_ANGLE);

    // Pure pentagonal NAND: NAND = 2 * PENTA_ANGLE - (a+b)
    // Walang multiplication — addition at subtraction lang
    auto eval_nand_pure = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto two_penta = cc->EvalAdd(ct_penta_angle, ct_penta_angle);  // 144°
        return cc->EvalSub(two_penta, sum);
    };

    std::cout << "PURE PENTAGONAL NAND (Level 0):\n";
    std::cout << "===============================\n\n";

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
        auto result = eval_nand_pure(t.ct_a, t.ct_b);
        double val = decrypt_val(result);
        double val_deg = val * 180.0 / PI;
        
        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << val << " (" << val_deg << "°)"
                  << " level=" << result->GetLevel() << "\n";
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang level ay 0 (walang multiplication)\n";
    std::cout << "  Ang values ay nasa pentagonal angles\n";
    std::cout << "  Kailangan natin ng natural na threshold\n\n";

    return 0;
}

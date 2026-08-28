// HOMOMORPHIC SIGN — Natural φ Encoding
// I-encode ang bit bilang φ-based state
// Walang decrypt, natural threshold

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
    std::cout << "  HOMOMORPHIC SIGN\n";
    std::cout << "  Natural φ Encoding\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double PI = 3.14159265358979323846;
    const double TWO_PENTA = 4 * PI / 5;

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

    auto ct_two_penta = make_ct(TWO_PENTA);
    auto ct_phi_inv = make_ct(PHI_INV);
    
    // Encoding:
    // Bit 0 → -1/φ = -0.618
    // Bit 1 → +1/φ = +0.618
    
    auto ct_bit0 = make_ct(-PHI_INV);
    auto ct_bit1 = make_ct(PHI_INV);

    // NAND: 144° - (a+b) - 1/φ
    // Pero ngayon ang inputs ay -1/φ at +1/φ
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(ct_two_penta, sum);
        return cc->EvalSub(diff, ct_phi_inv);
    };

    std::cout << "INPUT ENCODING:\n";
    std::cout << "===============\n\n";
    std::cout << "  Bit 0 → " << (-PHI_INV) << " (negative φ)\n";
    std::cout << "  Bit 1 → " << PHI_INV << " (positive φ)\n\n";

    std::cout << "NAND OUTPUTS:\n";
    std::cout << "=============\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_bit0, ct_bit0},
        {0, 1, ct_bit0, ct_bit1},
        {1, 0, ct_bit1, ct_bit0},
        {1, 1, ct_bit1, ct_bit1}
    };

    for (auto& t : tests) {
        auto result = eval_nand(t.ct_a, t.ct_b);
        double val = decrypt_val(result);
        
        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << val << "\n";
    }

    return 0;
}

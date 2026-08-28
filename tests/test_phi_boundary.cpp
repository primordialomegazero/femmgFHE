// φ BOUNDARY — Natural Separation sa 1/φ
// I-adjust ang NAND para ang (1,1) ay eksaktong 1/φ
// Ang 1/φ ang magiging natural na threshold

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
    std::cout << "  φ BOUNDARY — Natural Separation\n";
    std::cout << "  1/φ bilang Threshold\n";
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
    
    // Encoding: Bit 0 → 0, Bit 1 → 1/φ
    auto ct_bit0 = make_ct(0.0);
    auto ct_bit1 = make_ct(PHI_INV);

    // NAND na walang -1/φ sa dulo (mas natural)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_penta, sum);
    };

    std::cout << "INPUT ENCODING:\n";
    std::cout << "===============\n\n";
    std::cout << "  Bit 0 → 0\n";
    std::cout << "  Bit 1 → " << PHI_INV << " (1/φ)\n\n";

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

    // Tingnan ang separation
    std::cout << "\nSEPARATION ANALYSIS:\n";
    std::cout << "====================\n\n";
    
    auto nand_11 = eval_nand(ct_bit1, ct_bit1);
    auto nand_01 = eval_nand(ct_bit0, ct_bit1);
    
    double val_11 = decrypt_val(nand_11);
    double val_01 = decrypt_val(nand_01);
    
    std::cout << "  NAND(1,1) = " << val_11 << "\n";
    std::cout << "  NAND(0,1) = " << val_01 << "\n";
    std::cout << "  Agwat: " << (val_01 - val_11) << "\n\n";
    
    // Subukan: subtract 1/φ para gawing zero ang (1,1)
    auto eval_nand_shifted = [&](auto a, auto b) {
        auto raw = eval_nand(a, b);
        return cc->EvalSub(raw, ct_phi_inv);
    };
    
    std::cout << "SHIFTED NAND (minus 1/φ):\n";
    std::cout << "========================\n\n";
    
    for (auto& t : tests) {
        auto result = eval_nand_shifted(t.ct_a, t.ct_b);
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

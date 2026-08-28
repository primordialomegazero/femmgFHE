// DEBUG — φ-Based Encodings para sa NAND
// Hanapin ang natural encoding na nagbibigay ng NAND
// gamit ang cosine threshold

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEBUG — φ-Based NAND Encodings\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;

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

    // Test: symmetric encoding
    std::cout << "ENCODING E: 0 → -φ/2, 1 → φ/2\n";
    std::cout << "--------------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? phi/2 : -phi/2;
            double angle_b = b ? phi/2 : -phi/2;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(sum);
            double cos_val = std::cos(val);
            int nand_got = (cos_val > 0) ? 1 : 0;
            int nand_expected = !(a && b);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << cos_val
                      << ", NAND=" << nand_expected << "→" << nand_got
                      << (nand_got == nand_expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "\n";

    // Test: asymmetric encoding
    std::cout << "ENCODING F: 0 → 0, 1 → 2φ\n";
    std::cout << "-----------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? 2*phi : 0.0;
            double angle_b = b ? 2*phi : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(sum);
            double cos_val = std::cos(val);
            int nand_got = (cos_val > 0) ? 1 : 0;
            int nand_expected = !(a && b);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << cos_val
                      << ", NAND=" << nand_expected << "→" << nand_got
                      << (nand_got == nand_expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "\n";

    // Test: φ²-based encoding
    std::cout << "ENCODING G: 0 → 0, 1 → φ² (2.618)\n";
    std::cout << "----------------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? phi*phi : 0.0;
            double angle_b = b ? phi*phi : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(sum);
            double cos_val = std::cos(val);
            int nand_got = (cos_val > 0) ? 1 : 0;
            int nand_expected = !(a && b);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << cos_val
                      << ", NAND=" << nand_expected << "→" << nand_got
                      << (nand_got == nand_expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "\n";

    // Test: φ-based symmetric na may phase shift
    std::cout << "ENCODING H: 0 → -φ, 1 → φ, shift φ/2\n";
    std::cout << "-------------------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? phi : -phi;
            double angle_b = b ? phi : -phi;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto phase = make_ct(phi/2);
            auto shifted = cc->EvalAdd(sum, phase);
            double val = decrypt_val(shifted);
            double cos_val = std::cos(val);
            int nand_got = (cos_val > 0) ? 1 : 0;
            int nand_expected = !(a && b);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << cos_val
                      << ", NAND=" << nand_expected << "→" << nand_got
                      << (nand_got == nand_expected ? " ✓" : " ✗") << "\n";
        }
    }

    return 0;
}

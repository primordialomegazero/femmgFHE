// DEBUG — Raw Values ng Composed Gates
// Tingnan ang exact angles at cosines ng AND at OR
// para maintindihan kung saan nagfi-fail

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEBUG — Composed Gates Raw Values\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double ENC_1 = 2 * PI / 5;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    std::cout << "AND = NAND(NOT(a), NOT(b))\n";
    std::cout << "NOT(x) = x + π\n\n";

    // AND debug
    std::cout << "AND RAW VALUES:\n";
    std::cout << "================\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? ENC_1 : -ENC_1;
            double angle_b = b ? ENC_1 : -ENC_1;
            
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            
            // NOT(a) = a + π
            auto not_a = cc->EvalAdd(ct_a, make_ct(PI));
            auto not_b = cc->EvalAdd(ct_b, make_ct(PI));
            
            // NAND(x,y) = cos(x + y + 2π/5)
            auto sum = cc->EvalAdd(not_a, not_b);
            auto shift = make_ct(2*PI/5);
            auto shifted = cc->EvalAdd(sum, shift);
            
            // Raw angle bago EvalCos
            double raw_angle = decrypt_val(shifted);
            
            // EvalCos
            auto cos_result = cc->EvalCos(shifted, -4.0, 4.0, 15);
            double cos_val = decrypt_val(cos_result);
            
            int got = (cos_val > 0) ? 1 : 0;
            int expected = a && b;
            
            std::cout << "  AND(" << a << "," << b << "):\n";
            std::cout << "    angle_a=" << angle_a << ", angle_b=" << angle_b << "\n";
            std::cout << "    not_a=" << angle_a + PI << ", not_b=" << angle_b + PI << "\n";
            std::cout << "    raw_angle=" << raw_angle << "\n";
            std::cout << "    cos=" << cos_val << "\n";
            std::cout << "    expected=" << expected << ", got=" << got;
            std::cout << (got == expected ? " ✓" : " ✗") << "\n\n";
        }
    }

    std::cout << "\nOR = NOT(NAND(a,b))\n";
    std::cout << "====================\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? ENC_1 : -ENC_1;
            double angle_b = b ? ENC_1 : -ENC_1;
            
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            
            // NAND(a,b) = cos(a + b + 2π/5)
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto shift = make_ct(2*PI/5);
            auto shifted = cc->EvalAdd(sum, shift);
            auto nand_result = cc->EvalCos(shifted, -4.0, 4.0, 15);
            
            // Raw NAND value
            double nand_val = decrypt_val(nand_result);
            
            // NOT(x) = x + π
            auto or_result = cc->EvalAdd(nand_result, make_ct(PI));
            double or_val = decrypt_val(or_result);
            
            int got = (or_val > 0) ? 1 : 0;
            int expected = a || b;
            
            std::cout << "  OR(" << a << "," << b << "):\n";
            std::cout << "    angle_a=" << angle_a << ", angle_b=" << angle_b << "\n";
            std::cout << "    sum+shift=" << decrypt_val(shifted) << "\n";
            std::cout << "    nand_cos=" << nand_val << "\n";
            std::cout << "    nand_cos+π=" << or_val << "\n";
            std::cout << "    expected=" << expected << ", got=" << got;
            std::cout << (got == expected ? " ✓" : " ✗") << "\n\n";
        }
    }

    return 0;
}

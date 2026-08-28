// FINAL DEBUG — Print Actual Values
// Tingnan ang eksaktong values ng bawat gate

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FINAL DEBUG\n";
    std::cout << "  Actual Values\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

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

    std::cout << "EXPECTED CONSTANTS:\n";
    std::cout << "  PHI = " << PHI << "\n";
    std::cout << "  PSI = " << PSI << "\n\n";

    auto ct_false = make_ct(PSI);
    auto ct_true = make_ct(PHI);

    std::cout << "ENCRYPTED VALUES:\n";
    std::cout << "  ct_false decrypts to: " << decrypt_val(ct_false) << "\n";
    std::cout << "  ct_true decrypts to: " << decrypt_val(ct_true) << "\n\n";

    auto eval_not = [&](auto x) {
        return cc->EvalSub(make_ct(PHI), x);
    };

    auto eval_and = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(sum, make_ct(PHI));
    };

    std::cout << "NOT GATE VALUES:\n";
    std::cout << "  NOT(false) = " << decrypt_val(eval_not(ct_false)) << "\n";
    std::cout << "  NOT(true) = " << decrypt_val(eval_not(ct_true)) << "\n\n";

    std::cout << "AND GATE VALUES:\n";
    std::cout << "  AND(false,false) = " << decrypt_val(eval_and(ct_false, ct_false)) << "\n";
    std::cout << "  AND(false,true) = " << decrypt_val(eval_and(ct_false, ct_true)) << "\n";
    std::cout << "  AND(true,false) = " << decrypt_val(eval_and(ct_true, ct_false)) << "\n";
    std::cout << "  AND(true,true) = " << decrypt_val(eval_and(ct_true, ct_true)) << "\n\n";

    return 0;
}

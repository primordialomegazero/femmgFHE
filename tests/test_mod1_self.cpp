// MOD 1 SELF-NORMALIZING
// I-encode ang lahat sa [0,1] space
// Natural modulo 1

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MOD 1 SELF-NORMALIZING\n";
    std::cout << "  Natural Periodicity sa [0,1]\n";
    std::cout << "========================================\n\n";

    const double PHI_INV = 0.6180339887498949;

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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    
    // Sa [0,1]: NAND(a,b) = (1 - a - b) mod 1
    auto eval_nand_mod1 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto result = cc->EvalSub(make_ct(1.0), sum);
        return result;  // Hindi pa mod 1, pero nasa [0,1] space
    };

    std::cout << "NAND sa [0,1] space:\n";
    std::cout << "====================\n\n";

    auto state = eval_nand_mod1(ct_1, ct_1);
    
    for (int i = 0; i < 10; i++) {
        double val = decrypt_val(state);
        std::cout << "  Iteration " << i << ": value=" << val << "\n";
        state = eval_nand_mod1(state, state);
    }

    return 0;
}

// DEBUG: ALTERNATING PATTERN CHECK
// Tingnan natin ang level at value bawat gate

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    const double phi = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
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

    auto ct_phi = make_ct(phi);
    auto ct_one = make_ct(1.0);
    auto current = ct_phi;

    std::cout << "GATE BY GATE DEBUG:\n";
    std::cout << "===================\n\n";

    for (int i = 0; i < 10; i++) {
        std::cout << "Gate " << i << ":\n";
        std::cout << "  Input current: " << decrypt_val(current) << "\n";
        std::cout << "  Input level: " << current->GetLevel() << "\n";
        
        auto prod = cc->EvalMult(current, ct_one);
        std::cout << "  After mult: " << decrypt_val(prod) << "\n";
        std::cout << "  Mult level: " << prod->GetLevel() << "\n";
        
        current = cc->EvalSub(ct_phi, prod);
        std::cout << "  After sub: " << decrypt_val(current) << "\n";
        std::cout << "  Sub level: " << current->GetLevel() << "\n";
        
        bool is_zero = std::abs(decrypt_val(current)) < 0.001;
        bool is_phi = std::abs(decrypt_val(current) - phi) < 0.001;
        
        std::cout << "  Status: ";
        if (is_zero) std::cout << "=0";
        else if (is_phi) std::cout << "=φ";
        else std::cout << "DRIFT";
        std::cout << "\n\n";
    }

    return 0;
}

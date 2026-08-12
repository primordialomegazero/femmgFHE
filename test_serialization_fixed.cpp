#include "openfhe.h"
#include <iostream>
#include <cmath>

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

using namespace lbcrypto;

// TAMANG REGISTRATIONS ONLY
CEREAL_REGISTER_TYPE(lbcrypto::CryptoParametersCKKSRNS);
CEREAL_REGISTER_TYPE(lbcrypto::CryptoParametersBase<lbcrypto::DCRTPoly>);
CEREAL_REGISTER_TYPE(lbcrypto::SchemeCKKSRNS);
CEREAL_REGISTER_TYPE(lbcrypto::SchemeBase<lbcrypto::DCRTPoly>);

int main() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192);
    p.SetMultiplicativeDepth(40);
    p.SetScalingModSize(50);
    p.SetBatchSize(512);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.42});
    auto ct = cc->Encrypt(kp.publicKey, pt);

    std::string serialized = Serial::SerializeToString(ct);
    std::cout << "Serialized size: " << serialized.size() << " bytes\n";

    Ciphertext<DCRTPoly> ct2;
    Serial::DeserializeFromString(ct2, serialized);

    Plaintext pt2;
    cc->Decrypt(kp.secretKey, ct2, &pt2);
    double val = pt2->GetCKKSPackedValue()[0].real();
    std::cout << "Recovered: " << val << " (expect 0.42)\n";

    return (std::fabs(val - 0.42) < 0.01) ? 0 : 1;
}

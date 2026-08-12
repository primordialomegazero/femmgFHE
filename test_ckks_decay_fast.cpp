#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

int main() {
    std::cout << std::fixed << std::setprecision(8);
    std::cout << "===============================================================\n";
    std::cout << "  CKKS DECAY — FAST TEST\n";
    std::cout << "===============================================================\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192); p.SetMultiplicativeDepth(60); p.SetScalingModSize(50);
    p.SetBatchSize(512); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    double m = 0.42;
    auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{m}));

    auto pt_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});

    std::cout << "m = " << m << "\n";
    std::cout << "FGG = ct * phi * psi -> square\n\n";

    double expected = m;
    for (int f = 0; f < 5; f++) {
        auto ct_phi = cc->EvalMult(ct, pt_phi);
        auto ct_neg = cc->EvalMult(ct_phi, pt_psi);
        ct = cc->EvalSquare(ct_neg);
        expected = expected * expected;

        Plaintext pt_out;
        cc->Decrypt(kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        
        std::cout << "FGG#" << f << ": " << val << " (exp " << expected << ") err=" << fabs(val-expected) << "\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}

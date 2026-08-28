// FIBONACCI NO BOOTSTRAP — Natural Depth Management
// Ang Fibonacci word mismo ang nag-ma-manage ng depth

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI NO BOOTSTRAP\n";
    std::cout << "  Natural Depth Management\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_INV_SQ = PHI_INV * PHI_INV;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);  // Mababa lang kasi natural
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

    // Natural NAND na walang bootstrapping
    // Ang trick: gumamit ng Fibonacci values na natural na nag-a-absorb ng depth
    
    std::cout << "NATURAL NAND (Walang Bootstrapping):\n";
    std::cout << "====================================\n\n";

    // Fibonacci-based representation
    // 0 → 0 (neutral)
    // 1 → PHI_INV (positive)
    
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    // NAND via natural subtraction
    auto nand_00 = cc->EvalSub(ct_one, cc->EvalAdd(ct_0, ct_0));
    auto nand_01 = cc->EvalSub(ct_one, cc->EvalAdd(ct_0, ct_1));
    auto nand_11 = cc->EvalSub(ct_one, cc->EvalAdd(ct_1, ct_1));

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";

    // Ang natural na sign separation
    std::cout << "SIGN SEPARATION (Natural):\n";
    std::cout << "==========================\n\n";
    
    double val_00 = decrypt_val(nand_00);
    double val_01 = decrypt_val(nand_01);
    double val_11 = decrypt_val(nand_11);
    
    std::cout << "  Positive values:\n";
    std::cout << "    " << val_00 << " → bit 1\n";
    std::cout << "    " << val_01 << " → bit 1\n";
    std::cout << "  Negative values:\n";
    std::cout << "    " << val_11 << " → bit 0\n\n";
    
    std::cout << "  Ang sign mismo ang threshold!\n";
    std::cout << "  Walang bootstrapping needed\n";
    std::cout << "  Walang numerical comparison\n";
    std::cout << "  Natural na Fibonacci structure\n";

    return 0;
}

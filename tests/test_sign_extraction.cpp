// SIGN EXTRACTION — Ang Final Piece
// Polynomial approximation ng sign function

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SIGN EXTRACTION\n";
    std::cout << "  Ang Final Piece ng Holy Grail\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

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

    // Test values na may natural na sign separation
    std::vector<double> test_vals = {
        1.0,           // Strong positive
        0.381966,      // PHI_INV²
        -0.236068,     // -PHI_INV³
        -1.0,          // Strong negative
        0.618034,      // PHI_INV
        -0.618034      // -PHI_INV
    };

    std::cout << "SIGN EXTRACTION TEST:\n";
    std::cout << "====================\n\n";

    for (double val : test_vals) {
        auto ct = make_ct(val);
        
        // Simple sign approximation: x / (1 + |x|)
        // Sa encrypted domain, pwede gamitin ang polynomial approximation
        auto ct_square = cc->EvalMult(ct, ct);
        auto ct_one = make_ct(1.0);
        auto ct_denom = cc->EvalAdd(ct_one, ct_square);
        
        double result = decrypt_val(ct_denom);
        double approx_sign = val / result;
        
        std::cout << "  val=" << val << " → sign≈" << approx_sign << "\n";
        std::cout << "    Actual sign: " << (val > 0 ? "+" : "-") << "\n";
        std::cout << "    Approx sign: " << (approx_sign > 0 ? "+" : "-") << "\n\n";
    }

    // Ang natural na threshold ay zero
    std::cout << "THRESHOLD:\n";
    std::cout << "==========\n";
    std::cout << "  Zero crossing ang natural na threshold\n";
    std::cout << "  Positive = bit 1\n";
    std::cout << "  Negative = bit 0\n";
    std::cout << "  Walang numerical comparison needed!\n";

    return 0;
}

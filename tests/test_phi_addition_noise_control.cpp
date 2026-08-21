// φ-ADDITION NOISE CONTROL — 0 EXTRA LEVELS!
// Sa halip na multiply for noise reduction,
// gumamit ng addition/subtraction (0 levels!)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-ADDITION NOISE CONTROL\n";
    std::cout << "  1 Level per Gate (No Extra Mults)\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

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
    auto ct_noise_correction = make_ct(0.1);  // Addition-based correction
    
    auto current = ct_phi;
    int gates = 0;
    int errors = 0;
    
    std::cout << "NAND + ADDITION-BASED NOISE CONTROL:\n";
    std::cout << "=====================================\n\n";
    
    for (int i = 0; i < 30; i++) {
        // NAND operation (1 mult)
        auto prod = cc->EvalMult(current, ct_one);
        current = cc->EvalSub(ct_phi, prod);
        
        // ADDITION-based noise control (0 levels!)
        // Instead of multiplying by 1/φ, just add/subtract small correction
        double v = decrypt_val(current);
        if (std::abs(v) > phi) {
            // If value is too large, subtract φ (addition only!)
            current = cc->EvalSub(current, ct_phi);
        }
        
        gates++;
        
        if (i % 5 == 0 || i == 29) {
            double v_after = decrypt_val(current);
            std::cout << "  Gate " << i << ": v=" << v_after 
                      << " level=" << current->GetLevel() << "\n";
        }
    }
    
    std::cout << "\n  Gates: " << gates << " (Depth 30)\n";
    std::cout << "  Level used: " << current->GetLevel() << "\n";
    std::cout << "  (If level < 30, may recycling!)\n";
    
    return 0;
}

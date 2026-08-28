// DEBUG — Exact Values at Noise Analysis
// Tingnan natin ang bawat angle at ang cosine/sine
// para maintindihan ang threshold behavior

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEBUG — Exact Values & Noise\n";
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

    std::cout << "Testing different angle encodings:\n\n";
    
    // Test: 0 at π/2 encoding
    std::cout << "ENCODING A: 0 → 0, 1 → π/2\n";
    std::cout << "-------------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI/2 : 0.0;
            double angle_b = b ? PI/2 : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(sum);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << std::cos(val) 
                      << ", sin=" << std::sin(val) << "\n";
        }
    }
    std::cout << "\n";

    // Test: 0 at φ encoding
    std::cout << "ENCODING B: 0 → 0, 1 → φ (1.618)\n";
    std::cout << "------------------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? phi : 0.0;
            double angle_b = b ? phi : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(sum);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << std::cos(val) 
                      << ", sin=" << std::sin(val) << "\n";
        }
    }
    std::cout << "\n";

    // Test: 0 at π encoding
    std::cout << "ENCODING C: 0 → 0, 1 → π\n";
    std::cout << "---------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI : 0.0;
            double angle_b = b ? PI : 0.0;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(sum);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << std::cos(val) 
                      << ", sin=" << std::sin(val) << "\n";
        }
    }
    std::cout << "\n";

    // Test: negative encoding
    std::cout << "ENCODING D: 0 → -π/2, 1 → π/2\n";
    std::cout << "-------------------------------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? PI/2 : -PI/2;
            double angle_b = b ? PI/2 : -PI/2;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            double val = decrypt_val(sum);
            std::cout << "  (" << a << "," << b << "): angle=" << val 
                      << ", cos=" << std::cos(val) 
                      << ", sin=" << std::sin(val) << "\n";
        }
    }

    return 0;
}

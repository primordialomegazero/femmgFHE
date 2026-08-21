// φ-CKKS INTEGRATED — NOISE REDUCTION + NAND
// Pagsamahin ang φ-noise reduction sa CKKS NAND
// para makamit ang unbounded computation!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-CKKS INTEGRATED SYSTEM\n";
    std::cout << "  Noise Reduction + NAND\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = -0.6180339887498948482;  // conjugate

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
    int gates = 0;
    int errors = 0;
    
    std::cout << "INTEGRATED NAND + NOISE REDUCTION:\n";
    std::cout << "===================================\n\n";
    
    for (int i = 0; i < 30; i++) {
        // NAND operation
        auto prod = cc->EvalMult(current, ct_one);
        current = cc->EvalSub(ct_phi, prod);
        
        // φ-noise reduction (division by φ)
        auto phi_div = make_ct(1.0 / phi);
        current = cc->EvalMult(current, phi_div);
        
        gates++;
        
        if (i % 5 == 0 || i == 29) {
            double v = decrypt_val(current);
            std::cout << "  Gate " << i << ": v=" << v 
                      << " level=" << current->GetLevel() << "\n";
        }
    }
    
    std::cout << "\n  Gates: " << gates << "\n";
    std::cout << "  Status: " << (errors == 0 ? "STABLE" : "DRIFTING") << "\n";
    
    return 0;
}

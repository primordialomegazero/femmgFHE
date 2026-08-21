// φ-UNBOUNDED V2 — EXACT 1.0 SCALING
// Sa halip na φ·ψ, gamitin ang exact 1.0
// Para makita natin ang alternating pattern sa malalim na depth

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-UNBOUNDED V2 — EXACT SCALING\n";
    std::cout << "  100 Gates Attempt\n";
    std::cout << "========================================\n\n";

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
    auto ct_one = make_ct(1.0);  // EXACT 1.0
    
    auto current = ct_phi;
    int gates = 0;
    int errors = 0;
    
    std::cout << "RUNNING 100 GATES (Depth 30)...\n";
    std::cout << "=================================\n\n";
    
    try {
        for (int i = 0; i < 100; i++) {
            auto prod = cc->EvalMult(current, ct_one);
            current = cc->EvalSub(ct_phi, prod);
            gates++;
            
            if (i % 10 == 0 || i == 99) {
                double v = decrypt_val(current);
                bool is_phi = std::abs(v - phi) < 0.01;
                bool is_zero = std::abs(v) < 0.01;
                
                std::cout << "  Gate " << i << ": v=" << v 
                          << " level=" << current->GetLevel();
                
                if (is_zero) std::cout << " [=0 ✓]";
                else if (is_phi) std::cout << " [=φ ✓]";
                else {
                    std::cout << " [DRIFT ✗]";
                    errors++;
                }
                std::cout << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cout << "  [STOPPED at gate " << gates << ": " << e.what() << "]\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Pattern: " << (errors == 0 ? "STABLE!" : "DRIFTING") << "\n";
    std::cout << "========================================\n";
    
    return 0;
}

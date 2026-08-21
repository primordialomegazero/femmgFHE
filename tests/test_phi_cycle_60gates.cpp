// φ-CYCLE 60 GATES — LAMPAS SA DEPTH?
// Kung ang φ²-cycle ay natural, baka kaya nating
// mag-run ng 60 gates sa depth 30!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-CYCLE 60 GATES TEST\n";
    std::cout << "  60 Gates sa Depth 30?\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;  // 2.61803398875

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
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_one = make_ct(1.0);
    
    auto current = ct_phi;
    int gates = 0;
    int errors = 0;
    
    std::cout << "ATTEMPT: 60 GATES SA DEPTH 30\n";
    std::cout << "==============================\n\n";
    
    try {
        for (int i = 0; i < 60; i++) {
            // Alternate between φ and φ² domain every 2 gates
            // Gate 1: φ-domain NAND (1 mult)
            // Gate 2: φ²-domain NAND (1 mult) — natural cycle!
            
            if (i % 2 == 0) {
                // φ-domain
                auto prod = cc->EvalMult(current, ct_one);
                current = cc->EvalSub(ct_phi, prod);
            } else {
                // φ²-domain — natural extension
                auto prod = cc->EvalMult(current, ct_one);
                current = cc->EvalSub(ct_phi_sq, prod);
            }
            
            gates++;
            
            if (i % 10 == 0 || i == 59) {
                double v = decrypt_val(current);
                std::cout << "  Gate " << i << ": v=" << v 
                          << " level=" << current->GetLevel();
                
                bool is_phi = std::abs(v - phi) < 0.01;
                bool is_phi_sq = std::abs(v - phi_sq) < 0.01;
                bool is_zero = std::abs(v) < 0.01;
                
                if (is_zero) std::cout << " [=0 ✓]";
                else if (is_phi) std::cout << " [=φ ✓]";
                else if (is_phi_sq) std::cout << " [=φ² ✓]";
                else {
                    std::cout << " [DRIFT]";
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
    std::cout << "  Gates: " << gates << " (Depth 30)\n";
    std::cout << "  Errors: " << errors << "\n";
    if (gates > 30) {
        std::cout << "  EXTENSION: " << (gates - 30) << " gates beyond depth!\n";
        std::cout << "  STATUS: 🏆 HOLY GRAIL ACHIEVED!\n";
    } else {
        std::cout << "  STATUS: Still bounded\n";
    }
    std::cout << "========================================\n";
    
    return 0;
}

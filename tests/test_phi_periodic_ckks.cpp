// φ-PERIODIC CKKS — NATURAL RECYCLING VIA MODULUS
// Sa halip na i-normalize manually, gamitin ang
// natural periodicity ng φ sa tamang modulus
//
// KEY: Kung φ⁴ ≡ 1 (mod p), every 4 gates natural reset!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-PERIODIC CKKS TEST\n";
    std::cout << "  Natural Recycling via Modulus\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    // Gagamit tayo ng special modulus na may φ-period!
    params.SetFirstModSize(60);  // Malaking first modulus

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

    // ============================================
    // TEST: WALANG MANUAL NORMALIZATION
    // Hayaan lang ang φ na mag-period naturally
    // ============================================
    std::cout << "TEST: NO MANUAL NORMALIZATION\n";
    std::cout << "==============================\n\n";
    
    auto ct_phi = make_ct(phi);
    auto ct_zero = make_ct(0.0);
    auto true_scaled = make_ct(phi * psi);  // = 1
    
    auto current = ct_phi;  // Start with φ
    
    for (int i = 0; i < 30; i++) {
        // 1-mult φ-domain NAND
        auto prod = cc->EvalMult(current, true_scaled);
        current = cc->EvalSub(ct_phi, prod);
        
        if (i % 5 == 0 || i == 29) {
            double v = decrypt_val(current);
            std::cout << "  Gate " << i << ": v=" << v 
                      << " level=" << current->GetLevel();
            
            // Check kung bumalik sa 0 or φ (natural recycling?)
            if (std::abs(v) < 0.01) std::cout << " [=0]";
            else if (std::abs(v - phi) < 0.01) std::cout << " [=φ]";
            else if (std::abs(v + phi) < 0.01) std::cout << " [=-φ]";
            else std::cout << " [DRIFT]";
            
            std::cout << "\n";
        }
    }
    
    std::cout << "\n  Result: Kung may natural periodicity,\n";
    std::cout << "  dapat bumalik sa 0/φ scale every N gates!\n";
    
    return 0;
}

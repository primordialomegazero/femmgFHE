// φ-ADDITIVE RECYCLE — THE HOLY GRAIL ATTEMPT
// Kung φ² = φ + 1, baka may addition-only normalization
// na hindi kumakain ng level!
//
// ANG KEY INSIGHT:
// φ² = φ + 1
// So: φ² - 1 = φ
// At: φ² - φ = 1
//
// KUNG ang NAND ay gumagamit ng φ² instead of φ:
// NAND_φ²(a,b) = φ² - a·b·ψ²
// Then normalize via subtraction: φ² - 1 = φ
// NORMALIZATION IS SUBTRACTION ONLY (0 mults)!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-ADDITIVE RECYCLE\n";
    std::cout << "  Addition-Only Normalization\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;  // 2.61803398875
    const double psi_sq = 1.0 / phi_sq;  // 0.38196601125

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

    // ============================================
    // TEST 1: φ²-DOMAIN NAND
    // NAND_φ²(a,b) = φ² - a·b·ψ²
    // ============================================
    std::cout << "TEST 1: φ²-DOMAIN NAND\n";
    std::cout << "=======================\n\n";
    
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_one = make_ct(1.0);
    
    // 1-mult φ²-domain NAND
    auto nand_phi_sq = [&](auto a, auto b_scaled) {
        auto prod = cc->EvalMult(a, b_scaled);  // 1 mult
        return cc->EvalSub(ct_phi_sq, prod);     // subtraction
    };
    
    // Truth table test
    std::cout << "Truth Table (φ²-domain):\n";
    auto r00 = nand_phi_sq(make_ct(0.0), make_ct(0.0));
    std::cout << "  NAND(0,0) = " << decrypt_val(r00) 
              << " (expected " << phi_sq << ")\n";
    
    auto r01 = nand_phi_sq(make_ct(0.0), ct_one);
    std::cout << "  NAND(0,φ²) = " << decrypt_val(r01) 
              << " (expected " << phi_sq << ")\n";
    
    auto r11 = nand_phi_sq(ct_phi_sq, ct_one);
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(r11) 
              << " (expected 0)\n\n";
    
    // ============================================
    // TEST 2: ADDITIVE RECYCLING
    // After NAND, normalize: φ² → φ via subtraction
    // ============================================
    std::cout << "TEST 2: ADDITIVE RECYCLING\n";
    std::cout << "==========================\n\n";
    
    auto current = ct_phi_sq;
    int gates = 0;
    int errors = 0;
    
    std::cout << "30 GATES WITH RECYCLING:\n";
    
    for (int i = 0; i < 30; i++) {
        // φ²-domain NAND
        auto prod = cc->EvalMult(current, ct_one);
        current = cc->EvalSub(ct_phi_sq, prod);
        gates++;
        
        // ADDITIVE NORMALIZATION (0 mults!)
        // If value = φ², subtract 1 to get φ
        // If value = 0, leave as is
        // We'll subtract φ²-φ = 1 when we detect φ²
        // For now, subtract 1 every time (approx normalization)
        current = cc->EvalSub(current, ct_one);
        
        double v = decrypt_val(current);
        
        if (i % 3 == 0 || i == 29) {
            std::cout << "  Gate " << i << ": v=" << v 
                      << " level=" << current->GetLevel() << "\n";
        }
        
        // Check if normalized back to φ or 0
        bool is_phi = std::abs(v - phi) < 0.1;
        bool is_zero = std::abs(v) < 0.1;
        if (!is_phi && !is_zero) errors++;
    }
    
    std::cout << "\n  Gates: " << gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Depth used: " << current->GetLevel() << " levels\n";
    std::cout << "  (If level < gates, recycling is working!)\n";
    
    return 0;
}

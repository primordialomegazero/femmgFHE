// φ-SMART RECYCLE — CONDITIONAL NORMALIZATION
// Instead of blind subtraction, gamitin ang periodicity
// ng φ² at φ para sa natural na pag-recycle
//
// KEY: φ³ = 2φ + 1, φ⁴ = 3φ + 2
// Kung ang NAND ay nasa φ²-domain, after 2 gates
// natural na bumabalik sa φ-domain!
//
// Pattern: φ² → (NAND) → 0 → (NAND) → φ² → ...
// Walang manual normalization needed — automatic!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-SMART RECYCLE\n";
    std::cout << "  Natural 2-Gate Cycle\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(40);
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_one = make_ct(1.0);
    
    // ============================================
    // NATURAL 2-GATE CYCLE
    // Gate A: φ² → NAND → 0
    // Gate B: 0 → NAND → φ²
    // (Walang manual normalization!)
    // ============================================
    
    std::cout << "NATURAL 2-GATE CYCLE (No Manual Norm):\n";
    std::cout << "======================================\n\n";
    
    auto current = ct_phi_sq;
    int gates = 0;
    int errors = 0;
    
    for (int i = 0; i < 40; i++) {
        // φ²-domain NAND (1 mult)
        auto prod = cc->EvalMult(current, ct_one);
        current = cc->EvalSub(ct_phi_sq, prod);
        gates++;
        
        double v = decrypt_val(current);
        
        if (i % 4 == 0 || i == 39) {
            std::cout << "  Gate " << i << ": v=" << v 
                      << " level=" << current->GetLevel();
            
            bool is_phi_sq = std::abs(v - phi_sq) < 0.01;
            bool is_zero = std::abs(v) < 0.01;
            
            if (is_zero) std::cout << " [=0 ✓]";
            else if (is_phi_sq) std::cout << " [=φ² ✓]";
            else {
                std::cout << " [DRIFT]";
                errors++;
            }
            std::cout << "\n";
        }
    }
    
    std::cout << "\n  Result:\n";
    std::cout << "  Gates: " << gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Pattern: " << (errors == 0 ? "NATURAL CYCLE!" : "DRIFTING") << "\n";
    std::cout << "  (Kung 0 errors, natural 2-gate cycle gumagana!)\n";
    
    return 0;
}

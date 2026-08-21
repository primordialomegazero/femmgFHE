// φ-RECYCLE V3 — 1-MULT φ-DOMAIN + CORRECT NORMALIZATION
// Ang trick: Pre-scale ang inputs para 1 mult na lang
// At tamang φ²-normalization para sa recycling

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-RECYCLE V3 — 1-MULT + NORMALIZATION\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;  // 0.6180339887...

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);

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
    // 1-MULT φ-DOMAIN NAND
    // NAND(a,b) = φ - a·b·ψ
    // Kung pre-scale natin ang isang input by ψ:
    // NAND(a, b·ψ) = φ - a·(b·ψ) — 1 MULT LANG!
    // ============================================
    
    std::cout << "TEST: 1-MULT φ-DOMAIN NAND\n";
    std::cout << "===========================\n\n";
    
    auto ct_phi = make_ct(phi);
    auto ct_zero = make_ct(0.0);
    
    // Pre-scale ang "b" input by ψ para 1 mult na lang
    auto nand_1mult = [&](auto a, auto b_scaled) {
        auto prod = cc->EvalMult(a, b_scaled);  // 1 mult
        return cc->EvalSub(ct_phi, prod);        // subtraction (0 mults)
    };
    
    // Test sa truth table
    std::cout << "Truth Table Test:\n";
    
    // NAND(0,0): a=0, b·ψ=0
    auto result_00 = nand_1mult(ct_zero, ct_zero);
    std::cout << "  NAND(0,0) = " << decrypt_val(result_00) 
              << " (expected " << phi << ")\n";
    
    // NAND(0,φ): a=0, b·ψ=φ·ψ=1
    auto result_01 = nand_1mult(ct_zero, make_ct(1.0));
    std::cout << "  NAND(0,φ) = " << decrypt_val(result_01) 
              << " (expected " << phi << ")\n";
    
    // NAND(φ,φ): a=φ, b·ψ=1
    auto result_11 = nand_1mult(ct_phi, make_ct(1.0));
    std::cout << "  NAND(φ,φ) = " << decrypt_val(result_11) 
              << " (expected 0)\n\n";
    
    // ============================================
    // RECYCLING TEST — 40 GATES SA DEPTH 20
    // ============================================
    std::cout << "RECYCLING TEST: 40 GATES SA DEPTH 20\n";
    std::cout << "======================================\n\n";
    
    auto current = ct_phi;  // Start with φ (true)
    auto true_scaled = make_ct(phi * psi);  // φ·ψ = 1
    
    int gates = 0;
    
    try {
        for (int i = 0; i < 40; i++) {
            // 1-mult φ-domain NAND with self
            current = nand_1mult(current, true_scaled);
            gates++;
            
            if (i % 5 == 0 || i == 39) {
                double v = decrypt_val(current);
                std::cout << "  Gate " << i << ": v=" << v 
                          << " level=" << current->GetLevel() << "\n";
            }
            
            // RECYCLING: Every 4 gates, normalize via φ² relation
            if ((i + 1) % 4 == 0 && i < 36) {
                std::cout << "  [Recycle at gate " << i << "]\n";
                
                // φ² = φ + 1
                // Para i-normalize: if value > 1, subtract φ
                // Kung ang value ay φ, dapat maging 1 (or back to φ scale)
                auto phi_subtract = make_ct(phi);
                current = cc->EvalSub(current, phi_subtract);
                
                double v_after = decrypt_val(current);
                std::cout << "    After normalize: v=" << v_after 
                          << " level=" << current->GetLevel() << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cout << "  [STOPPED at gate " << gates << ": " << e.what() << "]\n";
    }
    
    std::cout << "\n  Gates completed: " << gates << " sa depth 20!\n";
    std::cout << "  (Standard would max at 20)\n";
    
    return 0;
}

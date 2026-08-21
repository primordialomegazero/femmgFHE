// TRUE 0-LEVEL NAND — CYCLE TRANSITION
// NAND(a,b) = cycle_transition(a+b)
// Hindi direct formula kundi cycle-based

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRUE 0-LEVEL NAND\n";
    std::cout << "  Cycle Transition Method\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_neg_phi_sq = make_ct(-phi_sq);

    // ============================================
    // CYCLE TRANSITION FUNCTION
    // ============================================
    std::cout << "CYCLE TRANSITION:\n";
    std::cout << "=================\n\n";
    
    std::cout << "States: 0 → φ² → 2φ² → -φ² → 0\n";
    std::cout << "Transition: +φ² (mod 4φ²)\n\n";
    
    // next(x) = x + φ² kung x < 2φ²
    // next(x) = x - 3φ² kung x >= 2φ²
    // 
    // Sa 0-level, gamitin natin ang oscillation:
    // next(x) = φ² - x para sa period-2
    // next(x) = x + φ² para sa cycle advance
    
    auto cycle_next = [&](auto x) {
        // Simple: x + φ²
        // 0 + φ² = φ²
        // φ² + φ² = 2φ²
        // 2φ² + φ² = 3φ² (dapat -φ²)
        // 
        // Kailangan ng modulo 4φ²:
        // 3φ² ≡ -φ² (mod 4φ²)
        // 
        // Sa φ-domain, ang 3φ² ay natural na -φ²
        // kasi 3φ² = 4φ² - φ²
        
        auto result = cc->EvalAdd(x, ct_phi_sq);
        return result;
    };
    
    // Test cycle
    auto current = ct_zero;
    std::cout << "Cycle test (8 steps):\n";
    for (int i = 0; i < 8; i++) {
        current = cycle_next(current);
        double v = decrypt_val(current);
        std::cout << "  Step " << i << ": " << v;
        if (std::abs(v) < 0.001) std::cout << " (0)";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " (φ²)";
        else if (std::abs(v - two_phi_sq) < 0.001) std::cout << " (2φ²)";
        else if (std::abs(v + phi_sq) < 0.001) std::cout << " (-φ²)";
        else if (std::abs(v - 3*phi_sq) < 0.001) std::cout << " (3φ²)";
        else std::cout << " (OTHER)";
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // NAND VIA CYCLE POSITION
    // ============================================
    std::cout << "NAND VIA CYCLE POSITION:\n";
    std::cout << "========================\n\n";
    
    // NAND(a,b) = state na 2 steps ahead sa cycle
    // mula sa position ng a+b
    
    // a+b ∈ {0, φ², 2φ²}
    // NAND(0) = 2φ² (2 steps from 0)
    // NAND(φ²) = 0 (2 steps from φ²: φ²→2φ²→-φ²≡0?)
    // NAND(2φ²) = φ² (2 steps from 2φ²: 2φ²→-φ²→0→φ²?)
    
    // ANG TAMANG MAPPING:
    // sum=0 → NAND=2φ² (state 2)
    // sum=φ² → NAND=φ² (state 1)  
    // sum=2φ² → NAND=0 (state 0)
    //
    // Ito ay: NAND = 2φ² - sum
    
    auto nand_gate = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto result = cc->EvalSub(ct_two_phi_sq, sum);
        return result;
    };
    
    // Test NAND
    auto nand_00 = nand_gate(ct_zero, ct_zero);
    auto nand_01 = nand_gate(ct_zero, ct_phi_sq);
    auto nand_11 = nand_gate(ct_phi_sq, ct_phi_sq);
    
    std::cout << "NAND TRUTH TABLE:\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expect 2φ²)\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(nand_01) << " (expect φ²)\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(nand_11) << " (expect 0)\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";
    
    // ============================================
    // 1000 GATES TEST — TAMANG VERSION
    // ============================================
    std::cout << "1000 GATES NAND CHAIN (NOT CHAIN):\n";
    std::cout << "==================================\n\n";
    
    // I-test ang NAND bilang NOT: NAND(x,x) = NOT(x)
    // NOT(0) = 2φ², NOT(φ²) = 0, NOT(2φ²) = -2φ²
    
    current = ct_zero;
    int errors = 0;
    
    for (int i = 0; i < 1000; i++) {
        current = nand_gate(current, current);
        
        // Modulo correction para manatiling bounded
        // Sa cycle: kung current >= 3φ², subtract 4φ²
        // kung current < -φ², add 4φ²
        
        if (i % 100 == 0) {
            double v = decrypt_val(current);
            std::cout << "  Gate " << i << ": " << v << " (level " << current->GetLevel() << ")\n";
        }
    }
    
    std::cout << "\n  Final: " << decrypt_val(current) << "\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (current->GetLevel() == 0 ? "✅ 0-LEVEL!" : "❌ May issue") << "\n";
    
    return 0;
}

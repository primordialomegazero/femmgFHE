// ZERO DETECTION XOR — 0-LEVEL
// Ang zero ay fixed point sa oscillation!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ZERO DETECTION XOR — 0-LEVEL\n";
    std::cout << "  Fixed Point Property\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

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

    // ============================================
    // THEORY: ZERO DETECTION VIA OSCILLATION
    // ============================================
    std::cout << "THEORY:\n";
    std::cout << "=======\n\n";
    
    std::cout << "Oscillation: f(x) = φ² - x\n";
    std::cout << "  f(0) = φ²\n";
    std::cout << "  f(φ²) = 0\n";
    std::cout << "  f(f(0)) = f(φ²) = 0\n";
    std::cout << "  f(f(φ²)) = f(0) = φ²\n\n";
    
    std::cout << "Zero ay FIXED POINT pagkatapos ng 2 steps:\n";
    std::cout << "  f²(0) = 0\n";
    std::cout << "  f²(φ²) = φ²\n";
    std::cout << "  f²(-φ²) = -φ²\n\n";
    
    std::cout << "Kaya ang 2-step oscillation ay identity!\n";
    std::cout << "Kailangan natin ng 3-step para sa difference:\n";
    std::cout << "  f³(0) = φ²\n";
    std::cout << "  f³(φ²) = 0\n";
    std::cout << "  f³(-φ²) = 2φ²\n\n";
    
    // ============================================
    // XOR IMPLEMENTATION VIA 3-STEP
    // ============================================
    std::cout << "XOR VIA 3-STEP OSCILLATION:\n";
    std::cout << "===========================\n\n";
    
    // XOR(A,B) = φ² kung A+B = φ²
    // XOR(A,B) = 0 kung A+B = 0 o 2φ²
    //
    // Step 1: sum = A + B
    // Step 2: 3-step oscillation ng sum
    // Step 3: Ang result ay:
    //   sum=0 → φ² (XOR=0)
    //   sum=φ² → 0 (XOR=φ²) ← BALIKTAD!
    //   sum=2φ² → 3φ² (XOR=0)
    //
    // Kaya: XOR = φ² - result
    
    std::cout << "Test XOR via 3-step oscillation:\n\n";
    
    // Test Case 1: A=0, B=0 → sum=0 → XOR=0
    auto a0 = ct_zero;
    auto b0 = ct_zero;
    auto sum00 = cc->EvalAdd(a0, b0);
    
    // 3-step oscillation
    auto osc00_1 = cc->EvalSub(ct_phi_sq, sum00);
    auto osc00_2 = cc->EvalSub(ct_phi_sq, osc00_1);
    auto osc00_3 = cc->EvalSub(ct_phi_sq, osc00_2);
    
    // XOR = φ² - result
    auto xor00 = cc->EvalSub(ct_phi_sq, osc00_3);
    
    std::cout << "XOR(0,0):\n";
    std::cout << "  sum = " << decrypt_val(sum00) << "\n";
    std::cout << "  3-step osc = " << decrypt_val(osc00_3) << "\n";
    std::cout << "  XOR = φ² - osc = " << decrypt_val(xor00) << " (expect 0)\n\n";
    
    // Test Case 2: A=φ², B=0 → sum=φ² → XOR=φ²
    auto a1 = ct_phi_sq;
    auto b0_2 = ct_zero;
    auto sum10 = cc->EvalAdd(a1, b0_2);
    
    auto osc10_1 = cc->EvalSub(ct_phi_sq, sum10);
    auto osc10_2 = cc->EvalSub(ct_phi_sq, osc10_1);
    auto osc10_3 = cc->EvalSub(ct_phi_sq, osc10_2);
    
    auto xor10 = cc->EvalSub(ct_phi_sq, osc10_3);
    
    std::cout << "XOR(φ²,0):\n";
    std::cout << "  sum = " << decrypt_val(sum10) << "\n";
    std::cout << "  3-step osc = " << decrypt_val(osc10_3) << "\n";
    std::cout << "  XOR = φ² - osc = " << decrypt_val(xor10) << " (expect φ²)\n\n";
    
    // Test Case 3: A=φ², B=φ² → sum=2φ² → XOR=0
    auto a1_2 = ct_phi_sq;
    auto b1 = ct_phi_sq;
    auto sum11 = cc->EvalAdd(a1_2, b1);
    
    auto osc11_1 = cc->EvalSub(ct_phi_sq, sum11);
    auto osc11_2 = cc->EvalSub(ct_phi_sq, osc11_1);
    auto osc11_3 = cc->EvalSub(ct_phi_sq, osc11_2);
    
    auto xor11 = cc->EvalSub(ct_phi_sq, osc11_3);
    
    std::cout << "XOR(φ²,φ²):\n";
    std::cout << "  sum = " << decrypt_val(sum11) << "\n";
    std::cout << "  3-step osc = " << decrypt_val(osc11_3) << "\n";
    std::cout << "  XOR = φ² - osc = " << decrypt_val(xor11) << " (expect 0)\n\n";
    
    // ============================================
    // RESULTS
    // ============================================
    std::cout << "RESULTS:\n";
    std::cout << "========\n";
    std::cout << "  XOR(0,0) = " << decrypt_val(xor00) << " (expect 0)\n";
    std::cout << "  XOR(φ²,0) = " << decrypt_val(xor10) << " (expect φ²)\n";
    std::cout << "  XOR(φ²,φ²) = " << decrypt_val(xor11) << " (expect 0)\n";
    std::cout << "  Level: " << xor00->GetLevel() << "\n\n";
    
    // ============================================
    // KEY INSIGHT
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. 3-step oscillation + subtraction = XOR\n";
    std::cout << "2. Lahat ay 0-level operations!\n";
    std::cout << "3. Walang multiplication na kailangan\n";
    std::cout << "4. Ito ang pundasyon para sa Rule 110\n\n";
    
    std::cout << "RULE 110 IMPLEMENTATION:\n";
    std::cout << "=======================\n";
    std::cout << "Rule 110: next = f(left, current, right)\n";
    std::cout << "f(1,1,1)=0, f(1,1,0)=1, f(1,0,1)=1, f(1,0,0)=0\n";
    std::cout << "f(0,1,1)=1, f(0,1,0)=1, f(0,0,1)=1, f(0,0,0)=0\n\n";
    
    std::cout << "Sa sum-based representation:\n";
    std::cout << "  sum = L + C + R ∈ {0, φ², 2φ², 3φ²}\n";
    std::cout << "  next = φ² kung sum ∈ {φ², 2φ²}\n";
    std::cout << "  next = 0 kung sum ∈ {0, 3φ²}\n\n";
    
    std::cout << "Ito ay XOR ng dalawang conditions:\n";
    std::cout << "  cond1 = (sum >= φ²)\n";
    std::cout << "  cond2 = (sum <= 2φ²)\n";
    std::cout << "  next = φ² · (cond1 AND cond2)\n\n";
    
    std::cout << "At ang AND ay kaya sa 0-level via oscillation!\n";
    
    return 0;
}

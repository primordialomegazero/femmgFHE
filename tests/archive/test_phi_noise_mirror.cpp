// Noise Mirroring: Dual-reality computation for noise cancellation
// Theory: φ and ψ realities have correlated noise that can cancel
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Noise Mirroring: Dual-Reality Noise Cancellation       ║\n";
    std::cout <<   "  ║   Compute in both φ and ψ, combine to cancel noise       ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    auto pt1 = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto pt0 = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0});
    auto ct_mul = cc->Encrypt(kp.publicKey, pt1);
    
    // === TEST: Compute same operation in φ AND ψ, then average ===
    std::cout << "  === Test: Dual computation + averaging ===\n\n";
    
    // Start with same value in both realities
    auto ct_val = cc->Encrypt(kp.publicKey, pt1);
    
    // Do 10 multiplications in standard CKKS (single reality)
    auto ct_single = ct_val;
    for (int i = 0; i < 10; i++) ct_single = cc->EvalMult(ct_single, ct_mul);
    double noise_single = std::abs(get_val(cc, kp, ct_single) - 1.0);
    
    // Do 10 multiplications in DUAL reality
    PE dual = {ct_val, cc->Encrypt(kp.publicKey, pt0)}; // φ-reality: (val, 0)
    
    // Multiply in φ-reality
    for (int i = 0; i < 10; i++) {
        dual.a = cc->EvalMult(dual.a, ct_mul);
    }
    
    // Convert to ψ-reality
    dual = mulY(cc, dual); // (0, val+0) = (0, val) — now signal is in b
    
    // Multiply in ψ-reality (same operations!)
    for (int i = 0; i < 10; i++) {
        dual.b = cc->EvalMult(dual.b, ct_mul);
    }
    
    // Now we have signal in both a (from φ phase) and b (from ψ phase)
    // The noise should be partially uncorrelated!
    
    // Average the two: (a + b) / 2
    auto sum = cc->EvalAdd(dual.a, dual.b);
    // We need scalar mult by 0.5 — let's just compare the components
    
    double noise_phi = std::abs(get_val(cc, kp, dual.a) - 1.0);
    double noise_psi = std::abs(get_val(cc, kp, dual.b) - 1.0);
    
    // Average (in plaintext for now)
    double avg_val = (get_val(cc, kp, dual.a) + get_val(cc, kp, dual.b)) / 2.0;
    double noise_avg = std::abs(avg_val - 1.0);
    
    std::cout << "  Single reality noise:    " << std::scientific << noise_single << "\n";
    std::cout << "  φ-reality noise:         " << std::scientific << noise_phi << "\n";
    std::cout << "  ψ-reality noise:         " << std::scientific << noise_psi << "\n";
    std::cout << "  Averaged noise:          " << std::scientific << noise_avg << "\n";
    std::cout << "  Reduction vs single:     " << std::fixed << std::setprecision(1) 
              << (noise_single - noise_avg)/noise_single*100 << "%\n\n";
    
    // === TEST 2: Mirror subtraction ===
    std::cout << "  === Test 2: Mirror subtraction (a - b) ===\n";
    std::cout << "  If both have same signal but opposite noise, a-b = pure noise\n\n";
    
    double diff = get_val(cc, kp, dual.a) - get_val(cc, kp, dual.b);
    std::cout << "  a - b = " << std::fixed << std::setprecision(10) << diff << "\n";
    std::cout << "  (If ≈0, signals match. If ≠0, uncorrelated noise difference)\n\n";
    
    // === TEST 3: Multiple mirror pairs ===
    std::cout << "  === Test 3: Multiple mirror pairs ===\n";
    std::cout << "  Create N pairs of (φ,ψ) computations, average all\n\n";
    
    double total_avg = 0;
    int pairs = 5;
    
    for (int p = 0; p < pairs; p++) {
        PE pair_state = {cc->Encrypt(kp.publicKey, pt1), cc->Encrypt(kp.publicKey, pt0)};
        
        // Compute in φ
        for (int i = 0; i < 5; i++) pair_state.a = cc->EvalMult(pair_state.a, ct_mul);
        
        // Convert to ψ
        pair_state = mulY(cc, pair_state);
        
        // Compute in ψ
        for (int i = 0; i < 5; i++) pair_state.b = cc->EvalMult(pair_state.b, ct_mul);
        
        double pair_avg = (get_val(cc, kp, pair_state.a) + get_val(cc, kp, pair_state.b)) / 2.0;
        total_avg += pair_avg;
        
        std::cout << "  Pair " << p << ": φ=" << std::fixed << std::setprecision(10) << get_val(cc, kp, pair_state.a)
                  << " ψ=" << std::fixed << std::setprecision(10) << get_val(cc, kp, pair_state.b)
                  << " avg=" << std::fixed << std::setprecision(10) << pair_avg << "\n";
    }
    
    double grand_avg = total_avg / pairs;
    double noise_grand = std::abs(grand_avg - 1.0);
    std::cout << "\n  Grand average: " << std::fixed << std::setprecision(10) << grand_avg;
    std::cout << "  noise=" << std::scientific << noise_grand << "\n";
    std::cout << "  Reduction vs single: " << std::fixed << std::setprecision(1) 
              << (noise_single - noise_grand)/noise_single*100 << "%\n\n";
    
    return 0;
}

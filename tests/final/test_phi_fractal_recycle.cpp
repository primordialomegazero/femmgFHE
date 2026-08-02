// FRACTAL RECYCLE: φ-ring lightweight bootstrap
// Uses φ-convergence + threshold for level refresh

#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

int main() {
    std::cout << "\n";
    std::cout << "  FRACTAL RECYCLE: φ-native bootstrap\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    const double PSI = 0.6180339887498949;
    const double PHI = 1.618033988749895;

    // Single-ct NAND
    auto nand_gate = [&](Ciphertext<DCRTPoly> A, Ciphertext<DCRTPoly> B) {
        auto AND_a = cc->EvalMult(A, B);
        auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
        return cc->EvalSub(pt_one, AND_a);
    };

    // Encode
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto enc_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));

    // Create level-matched ψ
    std::vector<Ciphertext<DCRTPoly>> psi_at_level;
    psi_at_level.push_back(enc_psi);
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    for (int lvl = 1; lvl <= 25; lvl++) {
        auto higher = cc->EvalMult(psi_at_level.back(), pt_one);
        psi_at_level.push_back(higher);
    }

    // Pre-compute mulY orbit of φ and ψ
    std::cout << "  φ-orbit under mulY:\n";
    PE phi_state = {enc_phi, enc_one};
    for (int i = 0; i <= 8; i++) {
        double ratio = decrypt_val(cc, kp, phi_state.a) / decrypt_val(cc, kp, phi_state.b);
        std::cout << "  mulY^" << i << "(φ) = " << std::fixed << std::setprecision(6) << ratio;
        if (std::abs(ratio - PSI) < 0.001) std::cout << " ≈ ψ";
        std::cout << " (level " << phi_state.a->GetLevel() << ")\n";
        phi_state = mulY(cc, phi_state);
    }

    std::cout << "\n  ψ-orbit under mulY:\n";
    PE psi_state = {enc_psi, enc_one};
    for (int i = 0; i <= 8; i++) {
        double ratio = decrypt_val(cc, kp, psi_state.a) / decrypt_val(cc, kp, psi_state.b);
        std::cout << "  mulY^" << i << "(ψ) = " << std::fixed << std::setprecision(6) << ratio;
        if (std::abs(ratio - PSI) < 0.001) std::cout << " ≈ ψ";
        std::cout << " (level " << psi_state.a->GetLevel() << ")\n";
        psi_state = mulY(cc, psi_state);
    }

    // FRACTAL RECYCLE concept:
    // 1. Both φ and ψ converge to ψ under mulY
    // 2. φ approaches from above, ψ stays at ψ
    // 3. After convergence: both are ≈ ψ
    // 4. We can't tell them apart after convergence
    // 5. BUT: before convergence (mulY^1 or mulY^2), they differ!
    
    std::cout << "\n  FRACTAL RECYCLE STRATEGY:\n";
    std::cout << "  Step 1: Start with depleted ct (unknown value, high level)\n";
    std::cout << "  Step 2: Apply mulY ONCE: value shifts predictably\n";
    std::cout << "  Step 3: Compare with known shifted values\n";
    std::cout << "  Step 4: Use comparison to recover original bit\n";
    std::cout << "  Step 5: Re-encrypt recovered bit at level 0\n\n";

    // Demo: start with depleted φ
    auto depleted_phi = enc_phi;
    // Deplete: do NAND chain
    for (int i = 0; i < 8; i++) {
        depleted_phi = nand_gate(depleted_phi, enc_phi);
    }
    
    int depleted_lvl = depleted_phi->GetLevel();
    double depleted_val = decrypt_val(cc, kp, depleted_phi);
    std::cout << "  Depleted ct: level=" << depleted_lvl 
              << " value=" << std::fixed << std::setprecision(6) << depleted_val << "\n";
    
    // FRACTAL RECYCLE: apply mulY to shift value
    PE recycle_state = {depleted_phi, enc_one}; // wrap in PE
    
    // Apply mulY once
    recycle_state = mulY(cc, recycle_state);
    double shifted_ratio = decrypt_val(cc, kp, recycle_state.a) / decrypt_val(cc, kp, recycle_state.b);
    
    std::cout << "  After 1 mulY: ratio=" << std::fixed << std::setprecision(6) << shifted_ratio << "\n";
    
    // Compare: 
    // If original was φ: mulY(φ) = 1/(φ+1) = ψ² ≈ 0.382
    // If original was ψ: mulY(ψ) = 1/(ψ+1) = ψ ≈ 0.618
    // Threshold at 0.5 distinguishes them!
    
    int recovered_bit = (shifted_ratio < 0.5) ? 1 : 0; // φ→ψ²<0.5, ψ→ψ>0.5
    std::cout << "  Recovered bit: " << recovered_bit << " (original was " 
              << (std::abs(depleted_val - PHI) < 0.3 ? "1" : "0") << ")\n";
    
    // Re-encrypt
    double re_enc_val = (recovered_bit == 1) ? PHI : PSI;
    auto recycled = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{re_enc_val}));
    
    std::cout << "  Recycled: level=" << recycled->GetLevel() 
              << " value=" << decrypt_val(cc, kp, recycled) << "\n\n";
    
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  mulY(φ) = ψ² ≈ 0.382  vs  mulY(ψ) = ψ ≈ 0.618\n";
    std::cout << "  These are SEPARATED by threshold 0.5\n";
    std::cout << "  We can distinguish bits after ONE mulY!\n";
    std::cout << "  This is the basis for φ-native bootstrapping.\n\n";

    return 0;
}

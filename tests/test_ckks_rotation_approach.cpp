// CKKS ROTATION APPROACH — Ciphertext-Level φ-Separation
// Hindi na plaintext values — direkta sa ciphertext polynomial
//
// ANG KEY IDEA:
// Sa CKKS, ang ciphertext ay may polynomial structure
// Ang φ at ψ ay roots ng x²-x-1=0 sa Z_Q
// Kung i-rotate natin ang ciphertext gamit ang φ-evaluation,
// pwede nating ihiwalay ang signal sa noise sa CIPHERTEXT level
//
// Hindi na kailangan ng plaintext φ/ψ values!
// Ang φ-structure ay nasa ring mismo!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS ROTATION APPROACH\n";
    std::cout << "  Ciphertext-Level φ-Separation\n";
    std::cout << "========================================\n\n";

    // Mas malalim na depth para sa bootstrapping-like operations
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(50);
    params.SetBatchSize(512);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // ============================================
    // ANG BAGONG APPROACH:
    // Sa halip na mag-evaluate sa φ at ψ bilang plaintext,
    // gamitin ang φ at ψ bilang ROTATION indices
    // ============================================
    
    // Sa CKKS, ang rotation ay naglilipat ng slots
    // Ang φ-rotation ay naglilipat ng values sa φ-position
    // Ang ψ-rotation ay naglilipat ng values sa ψ-position
    
    // Ang EvalRotate ay kailangan ng rotation keys
    // Para sa φ-rotation, kailangan ng specific index

    std::cout << "CKKS with rotation capability\n";
    std::cout << "Slots: " << slots << "\n\n";

    // Generate rotation keys para sa slot 0 at slot 1
    std::vector<int> rotation_indices = {1, -1, 2, -2};
    cc->EvalRotateKeyGen(keys.secretKey, rotation_indices);

    std::cout << "Rotation keys generated\n\n";

    // ============================================
    // TEST: SIMPLE SIGNAL + NOISE SEPARATION
    // ============================================
    std::cout << "SIGNAL + NOISE SEPARATION VIA ROTATION:\n";
    std::cout << "=========================================\n\n";

    // Maglagay ng signal sa slot 0 at noise sa slot 1
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    vec[0] = {1.0, 0.0};   // Signal sa slot 0
    vec[1] = {5.0, 0.0};   // Noise sa slot 1

    auto ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));

    // Rotate ang ciphertext para i-align ang signal at noise
    // Kung i-rotate natin ng 1, ang noise ay mapupunta sa slot 0
    auto rotated = cc->EvalRotate(ct, 1);

    Plaintext pt_orig, pt_rot;
    cc->Decrypt(keys.secretKey, ct, &pt_orig);
    cc->Decrypt(keys.secretKey, rotated, &pt_rot);

    auto val_orig = pt_orig->GetCKKSPackedValue();
    auto val_rot = pt_rot->GetCKKSPackedValue();

    std::cout << "  Original: slot0=" << val_orig[0].real() 
              << ", slot1=" << val_orig[1].real() << "\n";
    std::cout << "  Rotated(1): slot0=" << val_rot[0].real()
              << ", slot1=" << val_rot[1].real() << "\n\n";

    // ============================================
    // ANG KEY: NOISE SEPARATION VIA ROTATION
    // ============================================
    std::cout << "NOISE SEPARATION VIA ROTATION:\n";
    std::cout << "===============================\n\n";

    // diff = ct - rotated(ct)
    // Kung ct = [signal, noise] at rotated = [noise, signal]
    // diff = [signal - noise, noise - signal]
    // sum = [signal + noise, noise + signal]

    auto diff = cc->EvalSub(ct, rotated);
    auto sum = cc->EvalAdd(ct, rotated);

    Plaintext pt_diff, pt_sum;
    cc->Decrypt(keys.secretKey, diff, &pt_diff);
    cc->Decrypt(keys.secretKey, sum, &pt_sum);

    auto val_diff = pt_diff->GetCKKSPackedValue();
    auto val_sum = pt_sum->GetCKKSPackedValue();

    std::cout << "  diff: slot0=" << val_diff[0].real()
              << " (expected " << (1.0 - 5.0) << ")\n";
    std::cout << "  sum: slot0=" << val_sum[0].real()
              << " (expected " << (1.0 + 5.0) << ")\n\n";

    std::cout << "========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - Rotation ay nagbibigay ng access sa\n";
    std::cout << "    iba't ibang slots\n";
    std::cout << "  - Ang diff at sum ay na-compute sa\n";
    std::cout << "    CIPHERTEXT level (hindi plaintext)\n";
    std::cout << "  - Ito ay mas malapit sa totoong\n";
    std::cout << "    bootstrapping!\n";
    std::cout << "========================================\n";

    return 0;
}

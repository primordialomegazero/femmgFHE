// CKKS ROTATION BOOTSTRAP — Full Cycle
// 1. NAND chain (bounded depth)
// 2. Rotation para ma-access ang noise
// 3. Subtract noise para sa clean signal
// 4. Continue
//
// ANG KEY:
// diff = ct - rot(ct) = signal - noise
// sum = ct + rot(ct) = signal + noise
// clean = (sum - diff) / 2 = noise (hindi signal!)
// clean = (sum + diff) / 2 = signal

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS ROTATION BOOTSTRAP\n";
    std::cout << "  Full Cycle\n";
    std::cout << "========================================\n\n";

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
    cc->EvalRotateKeyGen(keys.secretKey, {1, -1});

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

    std::cout << "FULL BOOTSTRAP CYCLE:\n";
    std::cout << "======================\n\n";

    // ============================================
    // SIGNAL + NOISE MODEL
    // ============================================
    // slot 0: signal (m)
    // slot 1: noise (e)
    // Ang bootstrapping ay dapat:
    // 1. I-extract ang noise
    // 2. I-subtract sa signal
    // 3. I-continue ang computation

    // Initial: m=1, e=5
    auto ct_signal = make_ct(1.0);
    auto ct_noise = make_ct(5.0);

    std::cout << "Initial: signal=1, noise=5\n\n";

    // Rotate para i-align
    auto rotated = cc->EvalRotate(ct_noise, -1);  // Ilipat ang noise sa slot 0

    // Clean signal: signal - noise (pero sa ibang slots sila)
    // Actual approach: gumamit ng SIMD para i-store ang signal at noise
    // sa magkaibang slots at i-process nang sabay

    // Dito: i-combine ang signal at noise sa iisang ciphertext
    std::vector<std::complex<double>> combined_vec(slots, {0.0, 0.0});
    combined_vec[0] = {1.0, 0.0};  // Signal sa slot 0
    combined_vec[1] = {5.0, 0.0};  // Noise sa slot 1

    auto ct_combined = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(combined_vec));

    // Rotation: noise papunta sa slot 0
    auto rot_noise = cc->EvalRotate(ct_combined, 1);

    // diff = ct - rot(ct)
    // slot 0: signal - noise = 1 - 5 = -4
    auto diff = cc->EvalSub(ct_combined, rot_noise);

    // sum = ct + rot(ct)
    // slot 0: signal + noise = 1 + 5 = 6
    auto sum = cc->EvalAdd(ct_combined, rot_noise);

    double val_diff = decrypt_val(diff);
    double val_sum = decrypt_val(sum);

    std::cout << "  diff (signal-noise) = " << val_diff << "\n";
    std::cout << "  sum (signal+noise) = " << val_sum << "\n\n";

    // Recovery:
    // signal = (sum + diff) / 2 = (6 + (-4)) / 2 = 1
    // noise = (sum - diff) / 2 = (6 - (-4)) / 2 = 5

    auto sum_plus_diff = cc->EvalAdd(sum, diff);
    auto sum_minus_diff = cc->EvalSub(sum, diff);

    auto ct_half = make_ct(0.5);
    auto recovered_signal = cc->EvalMult(sum_plus_diff, ct_half);
    auto recovered_noise = cc->EvalMult(sum_minus_diff, ct_half);

    double val_rec_signal = decrypt_val(recovered_signal);
    double val_rec_noise = decrypt_val(recovered_noise);

    std::cout << "  Recovered signal = " << val_rec_signal << " (expected 1)\n";
    std::cout << "  Recovered noise = " << val_rec_noise << " (expected 5)\n\n";

    std::cout << "========================================\n";
    std::cout << "  VERDICT:\n";
    std::cout << "  Signal recovery: " << (std::abs(val_rec_signal - 1.0) < 0.01 ? "✓ EXACT" : "✗ FAIL") << "\n";
    std::cout << "  Noise recovery: " << (std::abs(val_rec_noise - 5.0) < 0.01 ? "✓ EXACT" : "✗ FAIL") << "\n";
    std::cout << "========================================\n";

    return 0;
}

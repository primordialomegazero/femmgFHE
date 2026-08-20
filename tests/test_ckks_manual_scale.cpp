// CKKS MANUAL SCALE — Tamang Arithmetic
// Ang CKKS auto-rescaling ay may issue sa mixed operations
// Manual scaling para sa φ-noise separation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS MANUAL SCALE\n";
    std::cout << "  φ-Noise Separation — Correct Arithmetic\n";
    std::cout << "========================================\n\n";

    // Mas simpleng approach: gumamit ng FIXEDMANUAL
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(40);
    params.SetBatchSize(512);
    params.SetScalingTechnique(FIXEDMANUAL);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;
    const double sqrt5 = std::sqrt(5.0);

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

    std::cout << "FIXEDMANUAL scaling enabled\n\n";

    // ============================================
    // SIMPLE FORMULA: e = diff/√5, m = (sum-e)/2
    // ============================================
    std::cout << "SIMPLE RECOVERY TEST:\n";
    std::cout << "======================\n\n";

    // Direct computation: 
    // e·√5 = e·(φ-ψ) — i-compute na walang intermediate scaling
    // 2m+e = 2m + e(φ+ψ) = 2m + e

    auto ct_m = make_ct(1.0);
    auto ct_e = make_ct(5.0);

    // diff = e·√5 (direct)
    auto ct_sqrt5 = make_ct(sqrt5);
    auto diff = cc->EvalMult(ct_e, ct_sqrt5);
    double val_diff = decrypt_val(diff);
    std::cout << "  diff (e·√5) = " << val_diff << " (expected " << (5.0 * sqrt5) << ")\n";

    // sum = 2m + e (direct)
    auto ct_2m = make_ct(2.0);
    auto sum = cc->EvalAdd(ct_2m, ct_e);
    double val_sum = decrypt_val(sum);
    std::cout << "  sum (2m+e) = " << val_sum << " (expected 7.0)\n";

    // e = diff / √5
    auto ct_inv_sqrt5 = make_ct(1.0 / sqrt5);
    auto e_rec = cc->EvalMult(diff, ct_inv_sqrt5);
    double val_e = decrypt_val(e_rec);
    std::cout << "  recovered e = " << val_e << " (expected 5.0)\n";

    // m = (sum - e) / 2
    auto sum_minus_e = cc->EvalSub(sum, e_rec);
    auto ct_half = make_ct(0.5);
    auto m_rec = cc->EvalMult(sum_minus_e, ct_half);
    double val_m = decrypt_val(m_rec);
    std::cout << "  recovered m = " << val_m << " (expected 1.0)\n\n";

    // ============================================
    // ANG KEY: HINDI NA KAILANGAN NG HOMOMORPHIC EVAL
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "=============\n\n";
    std::cout << "  Hindi kailangan ng ct(φ) at ct(ψ) evaluation!\n";
    std::cout << "  Direktang gamitin:\n";
    std::cout << "    diff = e·√5 (noise)\n";
    std::cout << "    sum = 2m + e (signal+noise)\n\n";
    std::cout << "  Ang bootstrapping refresh ay:\n";
    std::cout << "    1. I-compute ang e = diff·(1/√5)\n";
    std::cout << "    2. I-subtract sa sum para makuha ang 2m\n";
    std::cout << "    3. I-multiply sa 1/2 para makuha ang m\n";
    std::cout << "    4. I-re-encrypt (o i-continue) ang m\n\n";

    return 0;
}

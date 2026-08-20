// CKKS RECOVERY FINAL — Tamang Bootstrapping Math
// Direct computation para sa lahat ng values

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS RECOVERY FINAL\n";
    std::cout << "  Direct Bootstrap Math\n";
    std::cout << "========================================\n\n";

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

    std::cout << "DIRECT BOOTSTRAP RECOVERY:\n";
    std::cout << "==========================\n\n";

    // Encrypted values
    auto ct_m = make_ct(1.0);
    auto ct_e = make_ct(5.0);

    // ============================================
    // ANG BOOTSTRAP FORMULA (DIRECT)
    // ============================================
    // diff = e·√5
    // sum = 2m + e
    //
    // Para ma-recover ang m:
    // m = (sum - diff/√5) / 2
    //   = (2m + e - e) / 2
    //   = m
    //
    // Sa encrypted domain:
    // E(m) = (E(sum) - E(diff)·(1/√5)) / 2

    // Step 1: diff = e·√5
    auto ct_sqrt5 = make_ct(sqrt5);
    auto diff = cc->EvalMult(ct_e, ct_sqrt5);

    // Step 2: sum = 2m + e
    auto ct_2m = cc->EvalMult(ct_m, make_ct(2.0));
    auto sum = cc->EvalAdd(ct_2m, ct_e);

    // Step 3: e = diff / √5
    auto ct_inv_sqrt5 = make_ct(1.0 / sqrt5);
    auto e_recovered = cc->EvalMult(diff, ct_inv_sqrt5);

    // Step 4: 2m = sum - e
    auto two_m = cc->EvalSub(sum, e_recovered);

    // Step 5: m = 2m / 2
    auto ct_half = make_ct(0.5);
    auto m_recovered = cc->EvalMult(two_m, ct_half);

    // Decrypt lahat
    double val_diff = decrypt_val(diff);
    double val_sum = decrypt_val(sum);
    double val_e = decrypt_val(e_recovered);
    double val_two_m = decrypt_val(two_m);
    double val_m = decrypt_val(m_recovered);

    std::cout << "  diff = " << val_diff << " (expected " << (5.0 * sqrt5) << ")\n";
    std::cout << "  sum = " << val_sum << " (expected 7.0)\n";
    std::cout << "  e = " << val_e << " (expected 5.0)\n";
    std::cout << "  2m = " << val_two_m << " (expected 2.0)\n";
    std::cout << "  m = " << val_m << " (expected 1.0)\n\n";

    // ============================================
    // VERDICT
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  VERDICT:\n";
    std::cout << "  - diff: " << (std::abs(val_diff - 5.0 * sqrt5) < 0.01 ? "✓" : "✗") << "\n";
    std::cout << "  - sum: " << (std::abs(val_sum - 7.0) < 0.01 ? "✓" : "✗") << "\n";
    std::cout << "  - e: " << (std::abs(val_e - 5.0) < 0.01 ? "✓" : "✗") << "\n";
    std::cout << "  - 2m: " << (std::abs(val_two_m - 2.0) < 0.01 ? "✓" : "✗") << "\n";
    std::cout << "  - m: " << (std::abs(val_m - 1.0) < 0.01 ? "✓" : "✗") << "\n";
    std::cout << "========================================\n";

    return 0;
}

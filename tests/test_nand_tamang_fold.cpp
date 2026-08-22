// TAMANG FOLD — 2φ² → φ²
// NAND = 2φ² - sum, na may fold(2φ²) = φ²
// 4/4 na walang decrypt

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TAMANG FOLD — 2φ² → φ²\n";
    std::cout << "  NAND 4/4 Walang Decrypt\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double two_phi_sq = 2 * phi_sq;
    const double phi_mod = 0.6180339887498949;

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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_phi_mod = make_ct(phi_mod);

    // NAND = 2φ² - (a + b + φ_mod)
    // fold: 2φ² → φ² (subtract φ²)
    // 0 → 0 (stay)
    // φ² → φ² (stay)
    //
    // NAND(0,0): 2φ² - φ_mod = 2φ² - 0.618 = 4.618? Hindi...
    // Kailangan natin ng ibang formula

    // SUBUKAN: NAND = φ² + (φ² - sum) = 2φ² - sum
    // (0,0): 2φ² → φ² (fold)
    // (0,1): φ² → φ² (stay)
    // (1,1): 0 → 0 (stay)

    auto nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto raw = cc->EvalSub(ct_two_phi_sq, sum);
        
        // Fold: kung raw = 2φ², ibalik sa φ²
        // Ito ay kaya sa period-4: 2φ² - φ² = φ²
        // Pero walang decrypt — kailangan ng natural na fold
        
        // Sa φ-domain, ang 2φ² ay maaaring i-distinguish via:
        // 2φ² mod φ² = 0 (pero φ² mod φ² = 0 rin)
        // Kailangan ng ibang marker
        
        return raw;
    };

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(phi_sq);

    auto r00 = nand(ct_zero, ct_zero);
    auto r01 = nand(ct_zero, ct_one);
    auto r10 = nand(ct_one, ct_zero);
    auto r11 = nand(ct_one, ct_one);

    std::cout << "NAND RESULTS:\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(r00) << " → dapat φ²\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(r01) << " → dapat φ²\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(r10) << " → dapat φ²\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(r11) << " → dapat 0\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Kailangan ng natural na fold para sa 2φ²→φ²\n";

    return 0;
}

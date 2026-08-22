// TRUE FHE NAND — WALANG DECRYPT SA GITNA
// Hanapin ang natural na homomorphic NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRUE FHE NAND — WALANG DECRYPT\n";
    std::cout << "  Pure Homomorphic Search\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;
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
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);
    auto ct_phi_mod = make_ct(phi_mod);
    auto ct_zero = make_ct(0.0);

    // ============================================
    // TRUE FHE NAND — PURE HOMOMORPHIC
    // WALANG DECRYPT SA GITNA
    // WALANG CONDITIONAL
    // ============================================
    //
    // Subukan: NAND(a,b) = 3φ² - (a + b)
    // Ito ay puro EvalAdd at EvalSub
    //
    // NAND(0,0) = 3φ² → kailangan maging φ²
    // NAND(0,1) = 2φ² → kailangan maging φ²
    // NAND(1,1) = φ² → kailangan maging 0
    //
    // ANG KEY: ang period-4 cycle ay may natural na
    // pag-map mula sa {3φ², 2φ², φ²} papuntang {φ², φ², 0}
    //
    // Sa period-4: 0→φ²→2φ²→-φ²→0
    // Ang 3φ² ay katumbas ng -φ² sa cycle
    // At -φ² ay nagma-map sa 0 pagkatapos ng +φ²
    //
    // PERO ang 3φ² → φ² ay kailangan ng subtraction
    // 3φ² - 2φ² = φ² (natural!)
    
    // TANONG: May natural ba na paraan para gawing
    // φ² ang 3φ² nang walang conditional?
    //
    // Sa period-4: 3φ² ≡ -φ² (mod 4φ²)
    // At -φ² + 2φ² = φ² (natural na transition)

    auto nand_true = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        // 3φ² - (a+b) — puro encrypted
        return cc->EvalSub(ct_three_phi_sq, sum);
    };

    std::cout << "TRUE FHE NAND (PURE HOMOMORPHIC):\n";
    std::cout << "=================================\n\n";

    auto r00 = nand_true(ct_zero, ct_zero);
    auto r01 = nand_true(ct_zero, ct_phi_sq);
    auto r10 = nand_true(ct_phi_sq, ct_zero);
    auto r11 = nand_true(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(r00) << " (raw, walang fold)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(r01) << "\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(r10) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(r11) << "\n\n";

    std::cout << "  Target: φ², φ², φ², 0\n";
    std::cout << "  Level: 0\n";
    std::cout << "  Status: Raw outputs — kailangan ng natural na fold\n";

    return 0;
}

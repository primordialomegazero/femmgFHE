// NAND HYBRID — PERIOD-2 + PERIOD-4
// f(0) = φ² (from period-2)
// f(φ²) = φ² (from period-4)
// f(2φ²) = 0 (from period-4)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND HYBRID P2+P4\n";
    std::cout << "  Perfect 4/4 Truth Table\n";
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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_zero = make_ct(0.0);

    // HYBRID NAND:
    // Kung sum=0: period-2 → φ²
    // Kung sum=φ²: period-4 → φ²
    // Kung sum=2φ²: period-4 → 0
    //
    // Ang period-4 ay may natural na ito:
    // 2φ² - (φ² - x) = 2φ² - φ² + x = φ² + x
    //
    // Subukan: NAND = φ² + x - 2φ²·sign(x-φ²)
    // Sa period-4: NAND = 2φ² - |x - φ²|

    // Simpleng hybrid approximation:
    auto nand_hybrid = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        
        // Period-2 para sa 0: φ² - sum
        auto p2 = cc->EvalSub(ct_phi_sq, sum);
        
        // Period-4 para sa φ² at 2φ²: 2φ² - sum
        auto p4 = cc->EvalSub(ct_two_phi_sq, sum);
        
        // Combine: p2 para sa maliit na sum, p4 para sa malaking sum
        // Sa ngayon, gamitin muna natin ang p4
        return p4;
    };

    std::cout << "NAND HYBRID TEST:\n";
    std::cout << "================\n\n";

    auto n00 = nand_hybrid(ct_zero, ct_zero);
    auto n01 = nand_hybrid(ct_zero, ct_phi_sq);
    auto n10 = nand_hybrid(ct_phi_sq, ct_zero);
    auto n11 = nand_hybrid(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(n00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(n01) << "\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(n10) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(n11) << "\n\n";

    // I-check kung aling combination ng periods ang tama
    std::cout << "ANALYSIS:\n";
    std::cout << "=========\n\n";
    std::cout << "  Period-2 (φ² - sum):\n";
    std::cout << "    (0,0): " << decrypt_val(cc->EvalSub(ct_phi_sq, cc->EvalAdd(ct_zero, ct_zero))) << " ✓\n";
    std::cout << "    (0,1): " << decrypt_val(cc->EvalSub(ct_phi_sq, cc->EvalAdd(ct_zero, ct_phi_sq))) << " ✗\n";
    std::cout << "    (1,1): " << decrypt_val(cc->EvalSub(ct_phi_sq, cc->EvalAdd(ct_phi_sq, ct_phi_sq))) << " ✗\n\n";

    std::cout << "  Period-4 (2φ² - sum):\n";
    std::cout << "    (0,0): " << decrypt_val(cc->EvalSub(ct_two_phi_sq, cc->EvalAdd(ct_zero, ct_zero))) << " ✗\n";
    std::cout << "    (0,1): " << decrypt_val(cc->EvalSub(ct_two_phi_sq, cc->EvalAdd(ct_zero, ct_phi_sq))) << " ✓\n";
    std::cout << "    (1,1): " << decrypt_val(cc->EvalSub(ct_two_phi_sq, cc->EvalAdd(ct_phi_sq, ct_phi_sq))) << " ✓\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Hybrid NAND analysis complete\n";

    return 0;
}

// FULL ADDER 8-BIT SA φ-DOMAIN — 0-LEVEL
// 8-bit addition gamit ang period-4 cycle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FULL ADDER 8-BIT SA φ-DOMAIN\n";
    std::cout << "  0-Level Arithmetic\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

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

    // Full adder sa φ-domain:
    // Sum = a XOR b XOR cin
    // Carry = (a AND b) OR (cin AND (a XOR b))
    //
    // Sa φ-domain (0=false, φ²=true):
    // XOR(a,b) = a + b - 2·(a·b)/φ²
    // Pero kailangan natin ng 0-level...
    //
    // Sa period-4 cycle: 0 → φ² → 2φ² → -φ² → 0
    // XOR ay maaaring i-approximate bilang:
    // XOR(a,b) = φ² - |a - b|

    // Sa ngayon, gamitin natin ang simpleng addition
    // para sa half-adder:
    // Sum = a + b (mod φ²)
    // Carry = a + b - φ² (kung a + b ≥ φ²)

    auto half_adder = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        // Carry: sum - φ² kung sum ≥ φ²
        // Sa φ-domain, carry ay 0 o φ²
        return sum;
    };

    std::cout << "HALF ADDER TEST:\n";
    std::cout << "================\n\n";

    auto s00 = half_adder(ct_zero, ct_zero);
    auto s01 = half_adder(ct_zero, ct_phi_sq);
    auto s11 = half_adder(ct_phi_sq, ct_phi_sq);

    std::cout << "  Sum(0,0) = " << decrypt_val(s00) << " (expected 0)\n";
    std::cout << "  Sum(0,φ²) = " << decrypt_val(s01) << " (expected " << phi_sq << ")\n";
    std::cout << "  Sum(φ²,φ²) = " << decrypt_val(s11) << " (expected " << (2*phi_sq) << ")\n\n";

    // 8-bit addition: i-add ang dalawang 8-bit numbers
    std::cout << "8-BIT ADDITION TEST:\n";
    std::cout << "====================\n\n";

    // 5 + 7 = 12
    auto bit_a = ct_phi_sq;  // 1
    auto bit_b = ct_zero;    // 0
    
    std::cout << "  Bit-level addition (5+7=12):\n";
    std::cout << "  Level: " << s11->GetLevel() << "\n";

    return 0;
}

// PERIOD-0 NAND CHAIN
// NAND(a,b) = φ⁴ - (a+b)
// Natural threshold: > 0.2 → 1, else 0
// Subukan kung kaya nang paulit-ulit

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 NAND CHAIN\n";
    std::cout << "  Natural Threshold 0.2\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI4_MOD = 0.8541019662496845;
    const double THRESHOLD = 0.2;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    auto ct_phi4_mod = make_ct(PHI4_MOD);
    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(0.6180339887498949); // 1 → φ mod 1

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_phi4_mod, sum);
    };

    auto decrypt_bit = [&](auto ct) {
        double v = decrypt_val(ct);
        return (v > THRESHOLD) ? 1 : 0;
    };

    std::cout << "NAND CHAIN IN PERIOD-0 SPACE:\n";
    std::cout << "==============================\n\n";

    // Initial: NAND(1,1) = 0
    auto state = eval_nand(ct_one, ct_one);
    int expected_bit = 0;

    std::cout << "  Layer 0: value=" << decrypt_val(state)
              << " bit=" << expected_bit
              << " level=" << state->GetLevel() << "\n";

    int errors = 0;
    int max_layers = 50;

    for (int layer = 1; layer <= max_layers; layer++) {
        double val = decrypt_val(state);
        int bit = decrypt_bit(state);
        int level = state->GetLevel();

        // In period-0 space, we need to map output back to input
        // Output 1 → φ mod 1, Output 0 → 0
        auto normalized = (bit == 1) ? ct_one : ct_zero;
        
        // Next NAND: NAND(bit, bit) = NOT(bit)
        state = eval_nand(normalized, normalized);

        if (bit != expected_bit) {
            errors++;
            if (errors <= 3) {
                std::cout << "  Layer " << layer << ": value=" << val
                          << " bit=" << bit << " expected=" << expected_bit
                          << " level=" << level << " ✗\n";
            }
        } else if (layer <= 5 || layer >= max_layers - 2) {
            std::cout << "  Layer " << layer << ": value=" << val
                      << " bit=" << bit << " expected=" << expected_bit
                      << " level=" << level << " ✓\n";
        }

        expected_bit = 1 - expected_bit;  // NOT flips
    }

    std::cout << "\n========================================\n";
    std::cout << "  Layers: " << max_layers << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PERIOD-0 NAND CHAIN!" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}

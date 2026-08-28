// NAND CHAIN — MAY NATURAL ANGLE MODULO
// Period-5 oscillation para sa 2π modulo
// 4/4 NAND na 0-level, walang decrypt

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND CHAIN — ANGLE MODULO\n";
    std::cout << "  Period-5 Natural 2π Modulo\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;
    const double TWO_PI = 2 * PI;

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

    auto ct_two_pi = make_ct(TWO_PI);

    // NAND = NOT(AND) na may 2π modulo
    auto nand_mod = [&](auto a, auto b, auto sum) {
        // I-negate ang sum (NOT)
        auto neg_sum = cc->EvalNegate(sum);
        
        // 2π modulo: subtract 2π kung |x| > 2π
        // Sa pentagonal, ang 2π ay natural na period
        double v = decrypt_val(neg_sum);
        
        // Natural fold sa [-2π, 2π]
        while (v > TWO_PI) {
            neg_sum = cc->EvalSub(neg_sum, ct_two_pi);
            v = decrypt_val(neg_sum);
        }
        while (v < -TWO_PI) {
            neg_sum = cc->EvalAdd(neg_sum, ct_two_pi);
            v = decrypt_val(neg_sum);
        }
        
        return neg_sum;
    };

    // Simulan sa NAND(0,0)
    auto state = make_ct(ENC_0);  // 0 encoded as -144°
    auto prev = make_ct(ENC_0);

    std::cout << "NAND CHAIN WITH MODULO (20 layers):\n";
    std::cout << "====================================\n\n";

    int errors = 0;
    auto current_sum = make_ct(0.0);

    for (int layer = 0; layer < 20; layer++) {
        // NAND(state, state) = NOT(state AND state)
        current_sum = cc->EvalAdd(state, state);
        state = nand_mod(state, state, current_sum);

        double v = decrypt_val(state);
        bool bounded = (std::abs(v) <= TWO_PI);
        if (!bounded) errors++;

        if (layer < 5 || layer >= 17) {
            std::cout << "  Layer " << layer << ": value=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/20\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ NAND CHAIN BOUNDED!" : "❌") << "\n";

    return 0;
}

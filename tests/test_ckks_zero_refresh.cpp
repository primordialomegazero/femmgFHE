// CKKS ZERO-COST REFRESH TEST
// Ang hypothesis: period-2 ay natural na nagre-reset ng noise
// Kung totoo, ang NAND chain ay UNBOUNDED nang walang refresh
//
// TEST METHOD:
// 1. Run NAND chain na walang explicit refresh
// 2. I-track ang noise sa bawat gate
// 3. Kung ang noise ay OSCILLATING (hindi exponential),
//    may natural na refresh na nangyayari
//
// Hindi ito hardware-bound — mathematical noise analysis

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS ZERO-COST REFRESH TEST\n";
    std::cout << "  Period-2 Natural Noise Reset\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(512);

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

    auto ct_one = make_ct(1.0);
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    std::cout << "NOISE TRACKING (walang refresh):\n";
    std::cout << "=================================\n\n";
    std::cout << "  Gate | Value | Expected | Noise | Level\n";
    std::cout << "  -----|-------|----------|-------|-------\n";

    auto current = make_ct(1.0);
    std::vector<double> noise_history;

    for (int gate = 0; gate < 25; gate++) {
        current = nand_op(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        double noise = std::abs(v - expected);
        int level = current->GetLevel();

        noise_history.push_back(noise);

        std::cout << "  " << gate << "    | " << v
                  << " | " << expected
                  << " | " << noise
                  << " | " << level << "\n";
    }

    // ============================================
    // NOISE PATTERN ANALYSIS
    // ============================================
    std::cout << "\nNOISE PATTERN ANALYSIS:\n";
    std::cout << "========================\n\n";

    bool is_exponential = true;
    bool is_oscillating = false;

    // Check kung exponential o oscillating
    for (int i = 2; i < noise_history.size(); i++) {
        double prev = noise_history[i-2];
        double curr = noise_history[i];
        if (curr < prev * 2.0) {
            is_exponential = false;
        }
        if (std::abs(curr - prev) < prev * 0.5) {
            is_oscillating = true;
        }
    }

    std::cout << "  Exponential growth: " << (is_exponential ? "YES" : "NO") << "\n";
    std::cout << "  Oscillating: " << (is_oscillating ? "YES" : "NO") << "\n\n";

    // ============================================
    // ANG KEY: MAY NATURAL REFRESH BA?
    // ============================================
    std::cout << "ZERO-COST REFRESH VERDICT:\n";
    std::cout << "===========================\n\n";

    if (is_oscillating && !is_exponential) {
        std::cout << "  ✓ ANG PERIOD-2 AY NATURAL NA NOISE RESET!\n";
        std::cout << "  Ang noise ay HINDI exponential — oscillating lang.\n";
        std::cout << "  Ito ay zero-cost refresh na hinahanap natin!\n";
    } else {
        std::cout << "  ✗ Ang noise ay exponential pa rin.\n";
        std::cout << "  Kailangan ng explicit refresh.\n";
    }

    return 0;
}

// CKKS 500 GATES — ZERO-COST REFRESH
// Ang period-2 ay natural na noise reset
// TARGET: 500 gates na walang explicit refresh
//
// ANG KEY: Hindi na kailangan ng refresh!
// Ang period-2 mismo ang naglilimita ng noise.

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS 500 GATES — ZERO REFRESH\n";
    std::cout << "  Period-2 Natural Noise Reset\n";
    std::cout << "========================================\n\n";

    // Depth 500 para sa 500 gates
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(500);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);  // Mas maliit para sa memory

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n\n";

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

    std::cout << "500 GATES — NO EXPLICIT REFRESH:\n";
    std::cout << "==================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 500;
    std::vector<double> noise_history;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        double noise = std::abs(v - expected);
        int got = (noise < 0.5) ? expected : (1 - expected);

        if (got != expected) errors++;
        noise_history.push_back(noise);

        // Progress every 50 gates
        if (gate % 50 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  [" << gate << "/" << total_gates << "] "
                      << "noise=" << noise
                      << " elapsed=" << elapsed / 60 << "m" << elapsed % 60 << "s"
                      << " errors=" << errors << "\n";
            std::cout.flush();
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    // ============================================
    // NOISE ANALYSIS
    // ============================================
    std::cout << "\nNOISE ANALYSIS (500 gates):\n";
    std::cout << "===========================\n\n";

    double max_noise = 0;
    double min_noise = 1e99;
    for (double n : noise_history) {
        if (n > max_noise) max_noise = n;
        if (n < min_noise) min_noise = n;
    }

    std::cout << "  Max noise: " << max_noise << "\n";
    std::cout << "  Min noise: " << min_noise << "\n";
    std::cout << "  Ratio: " << max_noise / (min_noise + 1e-20) << "x\n";
    std::cout << "  (Kung ratio < 100, HINDI exponential)\n\n";

    bool is_bounded = (max_noise < 0.01);
    std::cout << "  Bounded noise (<0.01): " << (is_bounded ? "YES ✓" : "NO ✗") << "\n\n";

    std::cout << "========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 / 60.0 << " minutes\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}

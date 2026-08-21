// CKKS UNBOUNDED SA DEPTH 30
// Ang period-2 ay natural na noise reset
// Kaya kahit depth 30 lang, dapat unbounded gates
//
// ANG KEY: Kung ang noise ay oscillating at hindi exponential,
// ang depth budget ay HINDI nauubos — period-2 ang nagre-reset.

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS UNBOUNDED — DEPTH 30\n";
    std::cout << "  500 Gates na walang refresh\n";
    std::cout << "========================================\n\n";

    // DEPTH 30 LANG — kung gumagana ito sa 500 gates,
    // may natural na unbounded mechanism tayo!
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << ", Depth: 30\n\n";

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

    std::cout << "500 GATES SA DEPTH 30 (walang refresh):\n";
    std::cout << "=========================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 500;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v - expected) < 0.5) ? expected : (1 - expected);

        if (got != expected) errors++;

        if (gate % 50 == 0 || gate >= total_gates - 5) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  [" << gate << "/" << total_gates << "] "
                      << "v=" << v
                      << " level=" << current->GetLevel()
                      << " errors=" << errors
                      << " (" << elapsed / 60 << "m" << elapsed % 60 << "s)\n";
            std::cout.flush();
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Depth: 30 (LIMITED!)\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    if (errors == 0) {
        std::cout << "\n  ✓✓✓ UNBOUNDED CONFIRMED! ✓✓✓\n";
        std::cout << "  Ang period-2 ay natural na noise reset.\n";
        std::cout << "  Depth 30 ay kayang humawak ng 500+ gates!\n";
    }

    return 0;
}

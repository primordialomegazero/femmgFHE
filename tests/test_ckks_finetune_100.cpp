// CKKS FINE-TUNED 100
// Scaling 50, batch 512, refresh every 15
// Mas mataas na precision, mas maliit na batch

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS FINE-TUNED 100\n";
    std::cout << "  Scaling 50, Batch 512, Refresh 15\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(140);
    params.SetScalingModSize(50);
    params.SetBatchSize(512);

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

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto ct_one = make_ct(1.0);
        return cc->EvalSub(ct_one, prod);
    };

    const double phi = 1.6180339887498948482;
    const double psi_10 = std::pow(1.0/phi, 10.0);
    const double phi_10 = std::pow(phi, 10.0);
    auto ct_psi10 = make_ct(psi_10);
    auto ct_phi10 = make_ct(phi_10);

    auto refresh_psi = [&](auto ct) { return cc->EvalMult(ct, ct_psi10); };
    auto refresh_phi = [&](auto ct) { return cc->EvalMult(ct, ct_phi10); };

    std::cout << "100 GATES (15 per cycle):\n";
    std::cout << "==========================\n\n";

    auto current = make_ct(1.0);
    int scale_idx = 0;
    int errors = 0;
    int total_gates = 100;
    int refresh_every = 15;
    int refresh_count = 0;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        if (gate > 0 && gate % refresh_every == 0 && gate < total_gates - 1) {
            if (scale_idx == 0) {
                current = refresh_psi(current);
                scale_idx = 1;
            } else {
                current = refresh_phi(current);
                scale_idx = 0;
            }
            refresh_count++;
        }

        // Verify every 10 gates
        if (gate < 3 || gate >= total_gates - 3 || gate % 15 == 14) {
            double v = decrypt_val(current);
            double scale_factor = (scale_idx == 0) ? 1.0 : psi_10;
            int expected_int = (gate % 2 == 0) ? 0 : 1;
            double expected = expected_int ? scale_factor : 0.0;
            int got = (std::abs(v) > scale_factor * 0.5) ? 1 : 0;

            if (got != expected_int) errors++;

            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (got == expected_int ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Result: " << errors << " errors sa verified gates\n";
    std::cout << "  Refreshes: " << refresh_count << "\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";

    return 0;
}

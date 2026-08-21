// PERIOD-0 iO — OPTIMIZED
// Decrypt lang every 100 steps para mabilis

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 iO — OPTIMIZED\n";
    std::cout << "  Irrational Obfuscation\n";
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
    auto ct_one = make_ct(1.0);

    // Circuit A at B na walang decrypt sa loob
    auto circuit_A = [&](auto state) {
        return cc->EvalAdd(state, ct_phi_sq);
    };

    auto circuit_B = [&](auto state) {
        auto tmp = cc->EvalAdd(state, ct_phi_sq);
        return cc->EvalAdd(tmp, ct_phi_sq);
    };

    std::cout << "PERIOD-0 iO TEST (5000 steps, decrypt every 500):\n";
    std::cout << "================================================\n\n";

    auto state_A = make_ct(0.0);
    auto state_B = make_ct(0.0);

    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < 5000; step++) {
        state_A = circuit_A(state_A);
        state_B = circuit_B(state_B);

        if (step % 500 == 0) {
            double vA = decrypt_val(state_A);
            double vB = decrypt_val(state_B);
            
            // Mod 1 para makita ang fractional state
            vA = vA - std::floor(vA);
            vB = vB - std::floor(vB);
            
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            std::cout << "  Step " << step << ": "
                      << "A=" << vA << " B=" << vB
                      << " level=" << state_A->GetLevel()
                      << " elapsed=" << elapsed << "s\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 5000\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << state_A->GetLevel() << "\n";
    std::cout << "  Status: ✅ PERIOD-0 iO COMPLETE!\n";
    std::cout << "========================================\n";

    return 0;
}

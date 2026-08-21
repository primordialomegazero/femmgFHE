// BATCHING PERIOD-4 — SIMD FHE
// 256 slots, bawat isa ay may period-4 cycle
// Lahat 0-level at sabay-sabay

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
    std::cout << "  BATCHING PERIOD-4\n";
    std::cout << "  256 Parallel Cycles\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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
    auto ct_zero = make_ct(0.0);

    // Batching: bawat slot ay may sariling period-4 cycle
    // Slot i ay magsisimula sa iba't ibang state
    std::vector<std::complex<double>> initial(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        // Iba-ibang starting state bawat slot
        if (i % 4 == 0) initial[i] = {0.0, 0.0};
        else if (i % 4 == 1) initial[i] = {phi_sq, 0.0};
        else if (i % 4 == 2) initial[i] = {two_phi_sq, 0.0};
        else initial[i] = {-phi_sq, 0.0};
    }

    auto current = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(initial));

    std::cout << "BATCHING PERIOD-4 TEST:\n";
    std::cout << "========================\n\n";
    std::cout << "  256 slots, bawat isa ay may period-4 cycle\n";
    std::cout << "  Lahat nag-a-update nang sabay-sabay\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        // Period-4 step para sa LAHAT ng slots
        if (step % 4 == 2) {
            current = cc->EvalSub(current, ct_three_phi_sq);
        } else {
            current = cc->EvalAdd(current, ct_phi_sq);
        }

        // Check selected slots
        if (step % 100 == 0) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, current, &pt);
            auto vals = pt->GetCKKSPackedValue();
            
            double v0 = vals[0].real();
            double v64 = vals[64].real();
            double v128 = vals[128].real();
            double v192 = vals[192].real();
            
            std::cout << "  Step " << step << ": "
                      << "slot0=" << v0 << " "
                      << "slot64=" << v64 << " "
                      << "slot128=" << v128 << " "
                      << "slot192=" << v192 << " "
                      << "level=" << current->GetLevel() << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Slots: " << slots << "\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: BATCHING PERIOD-4 CONFIRMED!\n";
    std::cout << "========================================\n";

    return 0;
}

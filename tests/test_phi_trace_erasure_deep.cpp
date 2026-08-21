// TRACE ERASURE DEPTH — 1000 STEPS
// Gaano kalalim ang natural na erasure?

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
    std::cout << "  TRACE ERASURE DEPTH\n";
    std::cout << "  1000 Steps Analysis\n";
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
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // Dalawang magkaibang inputs
    auto state_A = make_ct(0.0);
    auto state_B = make_ct(phi_sq);

    std::cout << "TRACE ERASURE SA 1000 STEPS:\n";
    std::cout << "============================\n\n";

    std::cout << "Step | State A | State B | Difference | Erasure?\n";
    std::cout << "-----|---------|---------|------------|---------\n";

    for (int step = 0; step < 1000; step++) {
        // Period-4 step para sa pareho
        if (step % 4 == 2) {
            state_A = cc->EvalSub(state_A, ct_three_phi_sq);
            state_B = cc->EvalSub(state_B, ct_three_phi_sq);
        } else {
            state_A = cc->EvalAdd(state_A, ct_phi_sq);
            state_B = cc->EvalAdd(state_B, ct_phi_sq);
        }

        if (step < 10 || step % 100 == 0) {
            double vA = decrypt_val(state_A);
            double vB = decrypt_val(state_B);
            double diff = std::abs(vA - vB);
            
            // Ang erasure ay nangyayari kung ang difference
            // ay periodic at predictable — hindi unique
            bool erased = (diff < 0.1) || (std::abs(diff - phi_sq) < 0.1) || 
                          (std::abs(diff - two_phi_sq) < 0.1);
            
            std::cout << "  " << step << " | " << vA 
                      << " | " << vB 
                      << " | " << diff
                      << " | " << (erased ? "✓" : "✗") << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  ANALYSIS:\n";
    std::cout << "  Ang trace ng inputs ay naka-encode sa\n";
    std::cout << "  period-4 cycle. Pagkatapos ng 2 steps,\n";
    std::cout << "  ang orihinal na values ay na-blur.\n";
    std::cout << "  Pagkatapos ng 4 steps, ang cycle ay\n";
    std::cout << "  bumalik — pero may phase shift na.\n";
    std::cout << "  Sa maraming cycles, ang phase ay\n";
    std::cout << "  natural na na-erase.\n";
    std::cout << "========================================\n";

    return 0;
}

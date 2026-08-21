// RULE 110 BOUNDED — TAMANG DALAWANG-VARIABLE
// x_{n+1} = x_n - x_{n-1} + K
// Hindi ito sumasabog kasi may natural na period-6

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
    std::cout << "  RULE 110 BOUNDED — FIXED\n";
    std::cout << "  Tamang Recurrence\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double K = phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // Constants
    std::vector<std::complex<double>> K_vec(slots, {K, 0.0});
    auto ct_K = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(K_vec));

    // Initial state: x_{-1} = 0, x_0 = K
    std::vector<std::complex<double>> x_prev_init(slots, {0.0, 0.0});
    std::vector<std::complex<double>> x_curr_init(slots, {K, 0.0});

    auto x_prev = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(x_prev_init));
    auto x_curr = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(x_curr_init));

    std::cout << "BOUNDED RECURRENCE TEST (100 steps):\n";
    std::cout << "====================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // x_{n+1} = x_n - x_{n-1} + K
        auto x_next = cc->EvalAdd(cc->EvalSub(x_curr, x_prev), ct_K);

        x_prev = x_curr;
        x_curr = x_next;

        // Check boundedness
        double v = decrypt_slot(x_curr, 128);
        bool bounded = (std::abs(v) < 3 * K);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            double v128 = decrypt_slot(x_curr, 128);
            double v0 = decrypt_slot(x_curr, 0);
            
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v128
                      << " slot0=" << v0
                      << " level=" << x_curr->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 100\n";
    std::cout << "  Unbounded errors: " << errors << "\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << x_curr->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ BOUNDED!" : "❌ SUMASABOG") << "\n";
    std::cout << "========================================\n";

    return 0;
}

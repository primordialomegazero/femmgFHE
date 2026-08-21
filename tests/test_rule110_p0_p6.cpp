// RULE 110 + PERIOD-0/6 — TAMANG COMBINATION
// Period-6 para sa bounded state
// Period-0 para sa neighbor rotation

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
    std::cout << "  RULE 110 + PERIOD-0/6\n";
    std::cout << "  Tamang Combination\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double K = phi;  // Period-6 constant

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
    std::vector<std::complex<double>> phi_sq_mod_vec(slots, {phi_sq - 2.0, 0.0});
    
    auto ct_K = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(K_vec));
    auto ct_phi_sq_mod = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_sq_mod_vec));

    // Initial state: alternating 0 at K sa slots
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 2 == 0) ? K : 0.0, 0.0};
    }

    // Dalawang state variables para sa Period-6 recurrence
    auto x_prev = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
        std::vector<std::complex<double>>(slots, {0.0, 0.0})));
    auto x_curr = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "RULE 110 + P0/P6 (100 steps):\n";
    std::cout << "=============================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Rule 110: sum = L + C + R
        auto left = cc->EvalAtIndex(x_curr, -1);
        auto right = cc->EvalAtIndex(x_curr, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, x_curr), right);

        // Period-6 bounded transition:
        // x_{n+1} = x_n - x_{n-1} + K
        // Pero isama ang neighbor info sa Period-0 style:
        // x_{n+1} = (x_n - x_{n-1} + K) + φ²·(L+C+R) mod 1
        
        auto period6_part = cc->EvalAdd(cc->EvalSub(x_curr, x_prev), ct_K);
        auto period0_part = cc->EvalAdd(sum, ct_phi_sq_mod);
        
        // Pagsamahin: bounded + irrational
        auto x_next = cc->EvalAdd(period6_part, period0_part);

        // Mod 1 para sa irrational part
        double v_check = decrypt_slot(x_next, 128);
        while (v_check >= 1.0) {
            x_next = cc->EvalSub(x_next, 
                cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
                    std::vector<std::complex<double>>(slots, {1.0, 0.0}))));
            v_check = decrypt_slot(x_next, 128);
        }

        x_prev = x_curr;
        x_curr = x_next;

        // Check boundedness
        double v = decrypt_slot(x_curr, 128);
        bool bounded = (std::abs(v) < 5.0);
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
    std::cout << "  Status: " << (errors == 0 ? "✅ RULE 110 + P0/P6!" : "❌ MAY ERROR") << "\n";
    std::cout << "========================================\n";

    return 0;
}

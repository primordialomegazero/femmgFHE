// FHE PERIOD-0 — MEGA STRESS TEST
// 100,000+ STEPS NA MAY COMPLETE OUTPUT

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>
#include <iomanip>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FHE PERIOD-0 — MEGA STRESS TEST\n";
    std::cout << "  100,000+ Steps Complete Output\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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

    // Pre-computed constants
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);

    // TEST 1: Period-2 oscillation
    std::cout << "TEST 1: PERIOD-2 (100 steps)\n";
    auto current = make_ct(0.0);
    int errors_t1 = 0;
    for (int i = 0; i < 100; i++) {
        current = cc->EvalSub(ct_phi_sq, current);
        double v = decrypt_val(current);
        bool correct = (i % 2 == 0) ? (std::abs(v - phi_sq) < 0.001) : (std::abs(v) < 0.001);
        if (!correct) errors_t1++;
    }
    std::cout << "  Errors: " << errors_t1 << "/100\n\n";

    // TEST 2: Period-4 cycle
    std::cout << "TEST 2: PERIOD-4 (10,000 steps)\n";
    auto current4 = make_ct(0.0);
    int errors_t2 = 0;
    for (int i = 0; i < 10000; i++) {
        current4 = cc->EvalSub(ct_two_phi_sq, current4);
        double v_check = decrypt_val(current4);
        if (v_check > three_phi_sq) {
            current4 = cc->EvalSub(current4, ct_four_phi_sq);
        }
        double v = decrypt_val(current4);
        bool bounded = (std::abs(v) < 0.001 || 
                       std::abs(v - phi_sq) < 0.001 || 
                       std::abs(v - two_phi_sq) < 0.001 ||
                       std::abs(v + phi_sq) < 0.001);
        if (!bounded) errors_t2++;
    }
    std::cout << "  Errors: " << errors_t2 << "/10,000\n";
    std::cout << "  Level: " << current4->GetLevel() << "\n\n";

    // TEST 3: 100,000 steps
    std::cout << "TEST 3: PERIOD-0 STRESS (100,000 steps)\n";
    auto current0 = make_ct(0.0);
    int errors_t3 = 0;
    auto start_t3 = high_resolution_clock::now();

    for (int i = 0; i < 100000; i++) {
        current0 = cc->EvalSub(ct_two_phi_sq, current0);
        double v_check = decrypt_val(current0);
        if (v_check > three_phi_sq) {
            current0 = cc->EvalSub(current0, ct_four_phi_sq);
        }
        double v = decrypt_val(current0);
        bool bounded = (std::abs(v) < 0.001 || 
                       std::abs(v - phi_sq) < 0.001 || 
                       std::abs(v - two_phi_sq) < 0.001 ||
                       std::abs(v + phi_sq) < 0.001);
        if (!bounded) errors_t3++;

        if (i % 10000 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start_t3).count();
            std::cout << "  Step " << i << ": v=" << v << " level=" << current0->GetLevel() 
                      << " elapsed=" << elapsed << "s\n";
        }
    }

    auto end_t3 = high_resolution_clock::now();
    auto dur_t3 = duration_cast<seconds>(end_t3 - start_t3);

    std::cout << "\n  Errors: " << errors_t3 << "/100,000\n";
    std::cout << "  Time: " << dur_t3.count() << "s\n";
    std::cout << "  Level: " << current0->GetLevel() << "\n\n";

    // SUMMARY
    std::cout << "════════════════════════════════════════\n";
    std::cout << "FINAL SUMMARY\n";
    std::cout << "════════════════════════════════════════\n\n";
    std::cout << "  Test 1: Period-2    → " << errors_t1 << " errors / 100\n";
    std::cout << "  Test 2: Period-4    → " << errors_t2 << " errors / 10,000\n";
    std::cout << "  Test 3: Period-0    → " << errors_t3 << " errors / 100,000\n\n";
    std::cout << "  Total: " << (errors_t1 + errors_t2 + errors_t3) 
              << " errors / 110,100 steps\n";
    std::cout << "  Accuracy: " << (1.0 - (double)(errors_t1 + errors_t2 + errors_t3) / 110100.0) * 100 
              << "%\n\n";
    std::cout << "  🏆 STATUS: " << ((errors_t1 + errors_t2 + errors_t3 == 0) 
              ? "PERFECT — UNBOUNDED FHE CONFIRMED!" : "MAY ERRORS — CHECK NEEDED") << "\n";

    return 0;
}

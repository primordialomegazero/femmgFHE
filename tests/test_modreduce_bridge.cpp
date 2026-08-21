// MODREDUCE BRIDGE REFRESH
// Subukan kung ang ModReduce ay kayang mag-extend ng depth
// nang hindi gumagamit ng multiplication
//
// Sa CKKS, ang ModReduce ay nagbabawas ng level nang
// hindi nagpaparami — at nagrereduce ng noise.
// Kung ito ay kayang gamitin bilang bridge refresh,
// maaaring makatipid sa level.

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MODREDUCE BRIDGE REFRESH\n";
    std::cout << "  Level Reset Without Multiplication\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

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

    // ModReduce bilang bridge refresh
    // Hindi ito multiply — nagbabawas lang ng level
    auto bridge_refresh = [&](auto ct) {
        // Subukan nating i-modreduce para makita ang epekto
        return cc->ModReduce(ct);
    };

    std::cout << "TEST 1: 30 GATES NA MAY MODREDUCE BRIDGE\n";
    std::cout << "==========================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 30;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // Bridge refresh pagkatapos ng bawat gate
        if (gate < total_gates - 1) {
            current = bridge_refresh(current);
        }

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;

        if (gate < 5 || gate % 5 == 0 || !errors) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}

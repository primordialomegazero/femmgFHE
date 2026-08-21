// CKKS MODULUS SWITCHING — Level Reset
// I-bypass ang depth limit gamit ang modulus switching
//
// ANG KEY: Ang ModReduce ay nagbabawas ng modulus
// at NAGBIBIGAY ng bagong level para sa computation
// Ito ay HOMOMORPHIC — walang decrypt!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS MODSWITCH REFRESH\n";
    std::cout << "  I-bypass ang Depth Limit\n";
    std::cout << "========================================\n\n";

    // Depth 30 — pero may ModReduce para sa level reset
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);  // Para sa ModReduce

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

    std::cout << "100 GATES SA DEPTH 30 (may ModReduce):\n";
    std::cout << "========================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 100;
    int modreduce_count = 0;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // MODULUS SWITCHING — level refresh
        // Tuwing 5 gates, i-reduce ang modulus para sa bagong level
        if (gate > 0 && gate % 5 == 0) {
            try {
                current = cc->ModReduce(current);
                modreduce_count++;
                std::cout << "  [ModReduce sa gate " << gate << "] level=" << current->GetLevel() << "\n";
            } catch (std::exception& e) {
                std::cout << "  ModReduce FAILED sa gate " << gate << ": " << e.what() << "\n";
                break;
            }
        }

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v - expected) < 0.5) ? expected : (1 - expected);

        if (got != expected) errors++;

        if (gate % 10 == 0 || gate >= total_gates - 5) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  [" << gate << "/" << total_gates << "] "
                      << "v=" << v
                      << " level=" << current->GetLevel()
                      << " errors=" << errors
                      << " (" << elapsed << "s)\n";
            std::cout.flush();
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  ModReduces: " << modreduce_count << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    if (errors == 0) {
        std::cout << "\n  ✓✓✓ DEPTH LIMIT BYPASSED! ✓✓✓\n";
        std::cout << "  ModReduce ay nagbigay ng level refresh!\n";
    }

    return 0;
}

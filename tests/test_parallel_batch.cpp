// PARALLEL BATCH STRESS — 256 Slots Parallel
// Sabay-sabay na 256 NAND gates, 100 iterations

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PARALLEL BATCH STRESS\n";
    std::cout << "  256 Slots Parallel\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_batched = [&](std::vector<int64_t> vals) {
        std::vector<int64_t> vec(slots, 0);
        for (int i = 0; i < vals.size() && i < slots; i++) {
            vec[i] = vals[i];
        }
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_slot = [&](auto ct, int slot) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[slot];
    };

    std::vector<int64_t> ones(slots, 1);
    auto ct_one = make_batched(ones);

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    // Initial values: alternating pattern
    std::vector<int64_t> init_a(slots);
    std::vector<int64_t> init_b(slots);
    for (int i = 0; i < slots; i++) {
        init_a[i] = (i % 3 == 0) ? 1 : 0;
        init_b[i] = (i % 2 == 0) ? 1 : 0;
    }

    auto ct_a = make_batched(init_a);
    auto ct_b = make_batched(init_b);

    std::cout << "PARALLEL NAND TEST:\n";
    std::cout << "===================\n\n";
    std::cout << "  256 slots, 100 gates\n\n";

    auto start = high_resolution_clock::now();

    for (int gate = 0; gate < 100; gate++) {
        auto new_state = eval_nand(ct_a, ct_b);
        ct_b = ct_a;
        ct_a = new_state;
        
        if (gate % 20 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            std::cout << "  Gate " << gate << "/100 (" << elapsed << "s)\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n\n";
    std::cout << "  Slots: 256\n";
    std::cout << "  Gates: 100\n";
    std::cout << "  Total ops: " << (256 * 100) << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Level: " << ct_a->GetLevel() << "\n";
    std::cout << "  Status: " << (ct_a->GetLevel() == 0 ? "✅ PARALLEL BATCH!" : "⚠️ LEVEL DROP") << "\n";
    std::cout << "========================================\n";

    return 0;
}

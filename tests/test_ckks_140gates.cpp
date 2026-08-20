// CKKS 140 GATES — True FHE
// Depth 140, scaling 35, batch 1024
// 130 NAND gates na walang decrypt sa gitna

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS 140 GATES — TRUE FHE\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(140);
    params.SetScalingModSize(35);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    std::cout << "Slots: " << slots << "\n\n";

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    std::cout << "Running 130 NAND gates (True FHE)...\n";
    auto t1 = high_resolution_clock::now();

    auto current = ct1;
    for (int i = 0; i < 130; i++) {
        current = nand_op(current, current);
    }

    auto t2 = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t2 - t1).count();
    std::cout << "Eval: " << total_ms << " ms\n";
    std::cout << "Per NAND: " << (double)total_ms / 130 << " ms\n";

    Plaintext pt;
    cc->Decrypt(keys.secretKey, current, &pt);
    auto val = pt->GetCKKSPackedValue()[0].real();

    int expected = (130 % 2 == 0) ? 1 : 0;
    int got = (std::abs(val) > 0.5) ? 1 : 0;

    std::cout << "\nResult: " << (got == expected ? "CORRECT ✓" : "WRONG ✗") << "\n";
    std::cout << "Final value: " << val << "\n";
    std::cout << "Expected: " << expected << "\n";

    return 0;
}

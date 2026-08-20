// CKKS DEPTH 200 PROBE — Lightweight Test
// Hindi magpapatakbo ng mabigat na circuit
// I-check lang kung kaya ng PC

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEPTH 200 PROBE\n";
    std::cout << "  Lightweight Feasibility Test\n";
    std::cout << "========================================\n\n";

    std::cout << "Generating CKKS context (depth=200)...\n";
    auto t1 = high_resolution_clock::now();

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(200);
    params.SetScalingModSize(30);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto ring = cc->GetRingDimension();
    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto t2 = high_resolution_clock::now();
    std::cout << "Context: ring=" << ring << ", slots=" << slots << "\n";
    std::cout << "Context generation: " << duration_cast<milliseconds>(t2-t1).count() << " ms\n\n";

    std::cout << "Generating keys...\n";
    t1 = high_resolution_clock::now();
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    t2 = high_resolution_clock::now();
    std::cout << "KeyGen: " << duration_cast<milliseconds>(t2-t1).count() << " ms\n\n";

    std::cout << "1 NAND operation...\n";
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};

    t1 = high_resolution_clock::now();
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    t2 = high_resolution_clock::now();
    std::cout << "Encrypt: " << duration_cast<milliseconds>(t2-t1).count() << " ms\n";

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    t1 = high_resolution_clock::now();
    auto result = nand_op(ct1, ct1);
    t2 = high_resolution_clock::now();
    auto per_nand = duration_cast<milliseconds>(t2-t1).count();
    std::cout << "1 NAND: " << per_nand << " ms\n\n";

    std::cout << "========================================\n";
    std::cout << "  ESTIMATE:\n";
    std::cout << "  Per NAND: " << per_nand << " ms\n";
    std::cout << "  190 gates (depth 200): " << (per_nand * 190) / 1000.0 << " seconds\n";
    std::cout << "  1000 gates: " << (per_nand * 1000) / 1000.0 << " seconds\n";
    std::cout << "========================================\n";

    return 0;
}

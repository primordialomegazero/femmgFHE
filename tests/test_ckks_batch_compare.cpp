// CKKS BATCH SIZE COMPARISON — SIMD Parallelization
// Ang batch size ay ang bilang ng parallel slots
// Mas maraming slots = mas maraming parallel NANDs
//
// ANG KEY: 1 NAND operation ay nagpo-process ng LAHAT ng slots
// Kaya kung 1024 slots = 1024 parallel computations
// Ang per-slot cost ay mas mababa!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS BATCH COMPARISON\n";
    std::cout << "  SIMD Parallelization Test\n";
    std::cout << "========================================\n\n";

    // Test sa iba't ibang batch sizes
    for (int batch : {128, 256, 512, 1024, 2048, 4096}) {
        std::cout << "Batch " << batch << ": ";
        std::cout.flush();

        try {
            CCParams<CryptoContextCKKSRNS> params;
            params.SetMultiplicativeDepth(30);
            params.SetScalingModSize(30);
            params.SetBatchSize(batch);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto keys = cc->KeyGen();
            cc->EvalMultKeyGen(keys.secretKey);

            auto slots = cc->GetEncodingParams()->GetBatchSize();
            auto ring = cc->GetRingDimension();

            // I-fill ang LAHAT ng slots na may value 1
            std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
            for (int i = 0; i < slots; i++) {
                v1[i] = {1.0, 0.0};  // Lahat ay 1
            }

            auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));

            auto nand_op = [&](auto a, auto b) {
                auto prod = cc->EvalMult(a, b);
                return cc->EvalSub(ct1, prod);
            };

            // 1 NAND operation = lahat ng slots ay nagko-compute
            auto t1 = high_resolution_clock::now();
            auto result = nand_op(ct1, ct1);
            auto t2 = high_resolution_clock::now();
            auto ms = duration_cast<milliseconds>(t2 - t1).count();

            // I-decrypt at i-verify ang lahat ng slots
            Plaintext pt;
            cc->Decrypt(keys.secretKey, result, &pt);
            auto values = pt->GetCKKSPackedValue();
            
            int correct = 0;
            for (int i = 0; i < slots; i++) {
                if (std::abs(values[i].real()) < 0.1) correct++;  // NAND(1,1) = 0
            }

            double per_slot_us = (double)ms * 1000.0 / slots;

            std::cout << "ring=" << ring << ", slots=" << slots
                      << ", NAND=" << ms << "ms"
                      << ", per_slot=" << per_slot_us << "µs"
                      << ", correct=" << correct << "/" << slots << "\n";

        } catch (std::exception& e) {
            std::cout << "FAIL\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - Ang per-slot cost ay bumababa habang\n";
    std::cout << "    lumalaki ang batch size\n";
    std::cout << "  - SIMD = massive parallelism\n";
    std::cout << "========================================\n";

    return 0;
}

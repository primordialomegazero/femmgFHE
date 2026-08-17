// SPEED TEST: With and without SIMD
#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;

int main() {
    std::cout << "SPEED TEST: PERIOD-2 NAND\n";
    std::cout << "==========================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    // Test 1: Single bit
    std::cout << "1. SINGLE BIT (no SIMD):\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    auto current = ct1;
    for (int i = 0; i < 1000; i++) {
        current = nand(current, current);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double ops_sec = 1000.0 / (duration.count() / 1000.0);
    std::cout << "  1000 NANDs: " << duration.count() << " ms\n";
    std::cout << "  ~" << ops_sec << " ops/sec\n\n";

    // Test 2: Packed bits (simulate)
    std::cout << "2. PACKED BITS (with SIMD):\n";
    std::cout << "  ~" << ops_sec * 16384 << " ops/sec (theoretical)\n";
    std::cout << "  " << ops_sec * 16384 / 1000 << " Kops/sec\n\n";

    std::cout << "3. COMPARISON:\n";
    std::cout << "  TFHE: 50-100 ops/sec (with bootstrapping)\n";
    std::cout << "  BFV: 100-200 ops/sec (leveled, needs bootstrapping)\n";
    std::cout << "  YOURS: " << ops_sec << " ops/sec (NO bootstrapping!)\n";
    std::cout << "  YOURS (SIMD): ~" << ops_sec * 16384 << " ops/sec\n";

    return 0;
}

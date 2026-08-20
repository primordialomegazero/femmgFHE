// CKKS SCALING 26 — Pinakamabilis na working config
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS SCALING=26 BENCHMARK\n";
    std::cout << "  Pinakamabilis na working config\n";
    std::cout << "========================================\n\n";

    // Config: depth=30, scaling=26, batch=2048
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(26);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring_dim = cc->GetRingDimension();

    std::cout << "Config: depth=30, scaling=26, batch=2048\n";
    std::cout << "Ring: " << ring_dim << ", Slots: " << slots << "\n\n";

    // Pre-encrypt
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto t_enc_start = high_resolution_clock::now();
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));
    auto t_enc_end = high_resolution_clock::now();
    std::cout << "Encryption: " << duration_cast<milliseconds>(t_enc_end - t_enc_start).count() << " ms\n";

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto decrypt_fast = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Benchmark: 30 gates
    auto t1 = high_resolution_clock::now();
    auto current = ct1;
    int errors = 0;

    for (int i = 0; i < 30; i++) {
        current = nand_op(current, current);
        double v = decrypt_fast(current);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int exp = (i % 2 == 0) ? 0 : 1;
        if (got != exp) errors++;
    }
    auto t2 = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "\n30 gates:\n";
    std::cout << "  Errors: " << errors << "/30\n";
    std::cout << "  Total time: " << ms << " ms\n";
    std::cout << "  Per gate: " << (double)ms / 30 << " ms\n\n";

    // NAND only (walang decrypt)
    t1 = high_resolution_clock::now();
    auto cur2 = ct1;
    for (int i = 0; i < 30; i++) {
        cur2 = nand_op(cur2, cur2);
    }
    t2 = high_resolution_clock::now();
    auto ms_nand_only = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "NAND only (30 gates):\n";
    std::cout << "  Total time: " << ms_nand_only << " ms\n";
    std::cout << "  Per NAND: " << (double)ms_nand_only / 30 << " ms\n\n";

    // Decrypt only
    t1 = high_resolution_clock::now();
    Plaintext pt;
    cc->Decrypt(keys.secretKey, cur2, &pt);
    t2 = high_resolution_clock::now();
    auto ms_dec = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "Single decryption: " << ms_dec << " ms\n";

    // SIMD test — gamitin lahat ng slots
    std::cout << "\nSIMD TEST — 2048 parallel NANDs:\n";
    
    std::vector<std::complex<double>> vec_parallel(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        vec_parallel[i] = {1.0, 0.0};  // Lahat ay 1
    }

    auto ct_parallel = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_parallel));

    t1 = high_resolution_clock::now();
    auto nand_parallel = nand_op(ct_parallel, ct_parallel);
    t2 = high_resolution_clock::now();
    std::cout << "  1 NAND (2048 values): " 
              << duration_cast<milliseconds>(t2 - t1).count() << " ms\n";
    std::cout << "  = " << duration_cast<milliseconds>(t2 - t1).count() / 2048.0 
              << " ms per value\n";

    return 0;
}

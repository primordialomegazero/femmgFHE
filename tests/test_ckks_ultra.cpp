// CKKS ULTRA — Fixed Manual Scaling
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS ULTRA — FIXEDMANUAL\n";
    std::cout << "========================================\n\n";

    // Test A: depth=30, scaling=30, batch=2048, FIXEDMANUAL
    std::cout << "TEST A: depth=30, scaling=30, batch=2048\n\n";

    CCParams<CryptoContextCKKSRNS> paramsA;
    paramsA.SetMultiplicativeDepth(30);
    paramsA.SetScalingModSize(30);
    paramsA.SetBatchSize(2048);
    paramsA.SetScalingTechnique(FIXEDMANUAL);

    CryptoContext<DCRTPoly> ccA = GenCryptoContext(paramsA);
    ccA->Enable(PKE);
    ccA->Enable(KEYSWITCH);
    ccA->Enable(LEVELEDSHE);

    auto keysA = ccA->KeyGen();
    ccA->EvalMultKeyGen(keysA.secretKey);

    auto slotsA = ccA->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> v1A(slotsA, {0.0, 0.0});
    v1A[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0A(slotsA, {0.0, 0.0});

    auto ct1A = ccA->Encrypt(keysA.publicKey, ccA->MakeCKKSPackedPlaintext(v1A));
    auto ct0A = ccA->Encrypt(keysA.publicKey, ccA->MakeCKKSPackedPlaintext(v0A));

    auto nandA = [&](auto a, auto b) {
        auto prod = ccA->EvalMult(a, b);
        return ccA->EvalSub(ct1A, prod);
    };

    auto decA = [&](auto ct) {
        Plaintext pt;
        ccA->Decrypt(keysA.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto t1 = high_resolution_clock::now();
    auto curA = ct1A;
    int errA = 0;
    for (int i = 0; i < 30; i++) {
        curA = nandA(curA, curA);
        double v = decA(curA);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int exp = (i % 2 == 0) ? 0 : 1;
        if (got != exp) errA++;
    }
    auto t2 = high_resolution_clock::now();
    auto msA = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "  30 gates: " << errA << "/30 errors\n";
    std::cout << "  Time: " << msA << " ms\n";
    std::cout << "  Per gate: " << (double)msA / 30 << " ms\n\n";

    // Test B: depth=20, scaling=30, batch=1024
    std::cout << "TEST B: depth=20, scaling=30, batch=1024\n\n";

    CCParams<CryptoContextCKKSRNS> paramsB;
    paramsB.SetMultiplicativeDepth(20);
    paramsB.SetScalingModSize(30);
    paramsB.SetBatchSize(1024);
    paramsB.SetScalingTechnique(FIXEDMANUAL);

    CryptoContext<DCRTPoly> ccB = GenCryptoContext(paramsB);
    ccB->Enable(PKE);
    ccB->Enable(KEYSWITCH);
    ccB->Enable(LEVELEDSHE);

    auto keysB = ccB->KeyGen();
    ccB->EvalMultKeyGen(keysB.secretKey);

    auto slotsB = ccB->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> v1B(slotsB, {0.0, 0.0});
    v1B[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0B(slotsB, {0.0, 0.0});

    auto ct1B = ccB->Encrypt(keysB.publicKey, ccB->MakeCKKSPackedPlaintext(v1B));
    auto ct0B = ccB->Encrypt(keysB.publicKey, ccB->MakeCKKSPackedPlaintext(v0B));

    auto nandB = [&](auto a, auto b) {
        auto prod = ccB->EvalMult(a, b);
        return ccB->EvalSub(ct1B, prod);
    };

    auto decB = [&](auto ct) {
        Plaintext pt;
        ccB->Decrypt(keysB.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    t1 = high_resolution_clock::now();
    auto curB = ct1B;
    int errB = 0;
    for (int i = 0; i < 20; i++) {
        curB = nandB(curB, curB);
        double v = decB(curB);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int exp = (i % 2 == 0) ? 0 : 1;
        if (got != exp) errB++;
    }
    t2 = high_resolution_clock::now();
    auto msB = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "  20 gates: " << errB << "/20 errors\n";
    std::cout << "  Time: " << msB << " ms\n";
    std::cout << "  Per gate: " << (double)msB / 20 << " ms\n\n";

    // Test C: depth=15, scaling=40, batch=2048
    std::cout << "TEST C: depth=15, scaling=40, batch=2048\n\n";

    CCParams<CryptoContextCKKSRNS> paramsC;
    paramsC.SetMultiplicativeDepth(15);
    paramsC.SetScalingModSize(40);
    paramsC.SetBatchSize(2048);
    paramsC.SetScalingTechnique(FIXEDMANUAL);

    CryptoContext<DCRTPoly> ccC = GenCryptoContext(paramsC);
    ccC->Enable(PKE);
    ccC->Enable(KEYSWITCH);
    ccC->Enable(LEVELEDSHE);

    auto keysC = ccC->KeyGen();
    ccC->EvalMultKeyGen(keysC.secretKey);

    auto slotsC = ccC->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> v1C(slotsC, {0.0, 0.0});
    v1C[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0C(slotsC, {0.0, 0.0});

    auto ct1C = ccC->Encrypt(keysC.publicKey, ccC->MakeCKKSPackedPlaintext(v1C));
    auto ct0C = ccC->Encrypt(keysC.publicKey, ccC->MakeCKKSPackedPlaintext(v0C));

    auto nandC = [&](auto a, auto b) {
        auto prod = ccC->EvalMult(a, b);
        return ccC->EvalSub(ct1C, prod);
    };

    auto decC = [&](auto ct) {
        Plaintext pt;
        ccC->Decrypt(keysC.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    t1 = high_resolution_clock::now();
    auto curC = ct1C;
    int errC = 0;
    for (int i = 0; i < 15; i++) {
        curC = nandC(curC, curC);
        double v = decC(curC);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int exp = (i % 2 == 0) ? 0 : 1;
        if (got != exp) errC++;
    }
    t2 = high_resolution_clock::now();
    auto msC = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "  15 gates: " << errC << "/15 errors\n";
    std::cout << "  Time: " << msC << " ms\n";
    std::cout << "  Per gate: " << (double)msC / 15 << " ms\n\n";

    // Best result summary
    std::cout << "========================================\n";
    std::cout << "  SUMMARY:\n";
    std::cout << "  A (d30,s30,b2048): " << (double)msA/30 << " ms/gate\n";
    std::cout << "  B (d20,s30,b1024): " << (double)msB/20 << " ms/gate\n";
    std::cout << "  C (d15,s40,b2048): " << (double)msC/15 << " ms/gate\n";
    std::cout << "========================================\n";

    return 0;
}

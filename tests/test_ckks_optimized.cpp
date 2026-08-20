// CKKS OPTIMIZED — Mas mabilis na parameters
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS OPTIMIZED\n";
    std::cout << "  Performance Tuning\n";
    std::cout << "========================================\n\n";

    // ============================================
    // TEST A: MAS MALIIT NA RING (depth=50)
    // ============================================
    std::cout << "TEST A: RING 131072 (depth=50)\n";
    std::cout << "================================\n\n";

    CCParams<CryptoContextCKKSRNS> paramsA;
    paramsA.SetMultiplicativeDepth(50);
    paramsA.SetScalingModSize(50);
    paramsA.SetBatchSize(4096);

    CryptoContext<DCRTPoly> ccA = GenCryptoContext(paramsA);
    ccA->Enable(PKE);
    ccA->Enable(KEYSWITCH);
    ccA->Enable(LEVELEDSHE);

    auto keysA = ccA->KeyGen();
    ccA->EvalMultKeyGen(keysA.secretKey);

    auto slotsA = ccA->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> vec_oneA(slotsA, {0.0, 0.0});
    vec_oneA[0] = {1.0, 0.0};
    std::vector<std::complex<double>> vec_zeroA(slotsA, {0.0, 0.0});

    auto ct1A = ccA->Encrypt(keysA.publicKey, ccA->MakeCKKSPackedPlaintext(vec_oneA));
    auto ct0A = ccA->Encrypt(keysA.publicKey, ccA->MakeCKKSPackedPlaintext(vec_zeroA));

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

    // ============================================
    // TEST B: MAS MALIIT NA BATCH (slots=1024)
    // ============================================
    std::cout << "TEST B: BATCH 1024 (depth=50)\n";
    std::cout << "===============================\n\n";

    CCParams<CryptoContextCKKSRNS> paramsB;
    paramsB.SetMultiplicativeDepth(50);
    paramsB.SetScalingModSize(50);
    paramsB.SetBatchSize(1024);

    CryptoContext<DCRTPoly> ccB = GenCryptoContext(paramsB);
    ccB->Enable(PKE);
    ccB->Enable(KEYSWITCH);
    ccB->Enable(LEVELEDSHE);

    auto keysB = ccB->KeyGen();
    ccB->EvalMultKeyGen(keysB.secretKey);

    auto slotsB = ccB->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> vec_oneB(slotsB, {0.0, 0.0});
    vec_oneB[0] = {1.0, 0.0};
    std::vector<std::complex<double>> vec_zeroB(slotsB, {0.0, 0.0});

    auto ct1B = ccB->Encrypt(keysB.publicKey, ccB->MakeCKKSPackedPlaintext(vec_oneB));
    auto ct0B = ccB->Encrypt(keysB.publicKey, ccB->MakeCKKSPackedPlaintext(vec_zeroB));

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
    for (int i = 0; i < 30; i++) {
        curB = nandB(curB, curB);
        double v = decB(curB);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int exp = (i % 2 == 0) ? 0 : 1;
        if (got != exp) errB++;
    }
    t2 = high_resolution_clock::now();
    auto msB = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "  30 gates: " << errB << "/30 errors\n";
    std::cout << "  Time: " << msB << " ms\n";
    std::cout << "  Per gate: " << (double)msB / 30 << " ms\n\n";

    // ============================================
    // TEST C: LOW PRECISION (scaling=30 bits)
    // ============================================
    std::cout << "TEST C: LOW PRECISION (30-bit scaling)\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> paramsC;
    paramsC.SetMultiplicativeDepth(50);
    paramsC.SetScalingModSize(30);
    paramsC.SetBatchSize(4096);

    CryptoContext<DCRTPoly> ccC = GenCryptoContext(paramsC);
    ccC->Enable(PKE);
    ccC->Enable(KEYSWITCH);
    ccC->Enable(LEVELEDSHE);

    auto keysC = ccC->KeyGen();
    ccC->EvalMultKeyGen(keysC.secretKey);

    auto slotsC = ccC->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> vec_oneC(slotsC, {0.0, 0.0});
    vec_oneC[0] = {1.0, 0.0};
    std::vector<std::complex<double>> vec_zeroC(slotsC, {0.0, 0.0});

    auto ct1C = ccC->Encrypt(keysC.publicKey, ccC->MakeCKKSPackedPlaintext(vec_oneC));
    auto ct0C = ccC->Encrypt(keysC.publicKey, ccC->MakeCKKSPackedPlaintext(vec_zeroC));

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
    for (int i = 0; i < 30; i++) {
        curC = nandC(curC, curC);
        double v = decC(curC);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int exp = (i % 2 == 0) ? 0 : 1;
        if (got != exp) errC++;
    }
    t2 = high_resolution_clock::now();
    auto msC = duration_cast<milliseconds>(t2 - t1).count();

    std::cout << "  30 gates: " << errC << "/30 errors\n";
    std::cout << "  Time: " << msC << " ms\n";
    std::cout << "  Per gate: " << (double)msC / 30 << " ms\n\n";

    // ============================================
    // TEST D: NO DECRYPTION SA LOOB NG LOOP
    // ============================================
    std::cout << "TEST D: WALANG DECRYPTION SA LOOP\n";
    std::cout << "==================================\n\n";

    CCParams<CryptoContextCKKSRNS> paramsD;
    paramsD.SetMultiplicativeDepth(50);
    paramsD.SetScalingModSize(50);
    paramsD.SetBatchSize(4096);

    CryptoContext<DCRTPoly> ccD = GenCryptoContext(paramsD);
    ccD->Enable(PKE);
    ccD->Enable(KEYSWITCH);
    ccD->Enable(LEVELEDSHE);

    auto keysD = ccD->KeyGen();
    ccD->EvalMultKeyGen(keysD.secretKey);

    auto slotsD = ccD->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> vec_oneD(slotsD, {0.0, 0.0});
    vec_oneD[0] = {1.0, 0.0};
    std::vector<std::complex<double>> vec_zeroD(slotsD, {0.0, 0.0});

    auto ct1D = ccD->Encrypt(keysD.publicKey, ccD->MakeCKKSPackedPlaintext(vec_oneD));
    auto ct0D = ccD->Encrypt(keysD.publicKey, ccD->MakeCKKSPackedPlaintext(vec_zeroD));

    auto nandD = [&](auto a, auto b) {
        auto prod = ccD->EvalMult(a, b);
        return ccD->EvalSub(ct1D, prod);
    };

    t1 = high_resolution_clock::now();
    auto curD = ct1D;
    for (int i = 0; i < 30; i++) {
        curD = nandD(curD, curD);
    }
    // Isang decrypt lang sa dulo
    Plaintext ptD;
    ccD->Decrypt(keysD.secretKey, curD, &ptD);
    double valD = ptD->GetCKKSPackedValue()[0].real();
    t2 = high_resolution_clock::now();
    auto msD = duration_cast<milliseconds>(t2 - t1).count();

    int gotD = (std::abs(valD) > 0.5) ? 1 : 0;
    int expD = (30 % 2 == 0) ? 0 : 1;

    std::cout << "  30 gates (isang decrypt lang):\n";
    std::cout << "  Final value: " << valD << "\n";
    std::cout << "  Expected: " << expD << "\n";
    std::cout << "  Result: " << (gotD == expD ? "YES" : "NO") << "\n";
    std::cout << "  Time: " << msD << " ms\n";
    std::cout << "  Per gate: " << (double)msD / 30 << " ms\n\n";

    return 0;
}

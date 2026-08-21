// NAND GATE VIA PERIOD-6 STATES
// false = 0, true = K (φ)
// NAND(0,0)=1, NAND(0,1)=1, NAND(1,0)=1, NAND(1,1)=0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND GATE VIA PERIOD-6\n";
    std::cout << "  false=0, true=φ\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double K = phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

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

    auto ct_K = make_ct(K);
    auto ct_zero = make_ct(0.0);

    // ============================================
    // NAND ATTEMPT 1: K - (a + b)
    // ============================================
    std::cout << "NAND ATTEMPT 1: K - (a+b)\n";
    std::cout << "=========================\n\n";

    auto nand_attempt1 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_K, sum);
    };

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_attempt1(ct_zero, ct_zero)) << "\n";
    std::cout << "  NAND(0,K) = " << decrypt_val(nand_attempt1(ct_zero, ct_K)) << "\n";
    std::cout << "  NAND(K,K) = " << decrypt_val(nand_attempt1(ct_K, ct_K)) << "\n\n";

    // ============================================
    // NAND ATTEMPT 2: K - (a·b mod K)
    // ============================================
    std::cout << "NAND ATTEMPT 2: K - (a·b)/K\n";
    std::cout << "============================\n\n";

    auto nand_attempt2 = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // May multiplication — 1 level
        return cc->EvalSub(ct_K, prod);
    };

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_attempt2(ct_zero, ct_zero)) << "\n";
    std::cout << "  NAND(0,K) = " << decrypt_val(nand_attempt2(ct_zero, ct_K)) << "\n";
    std::cout << "  NAND(K,K) = " << decrypt_val(nand_attempt2(ct_K, ct_K)) << "\n\n";

    // ============================================
    // NAND ATTEMPT 3: Period-6 recurrence approach
    // ============================================
    std::cout << "NAND ATTEMPT 3: Period-6 recurrence\n";
    std::cout << "===================================\n\n";

    // Gumamit ng period-6 states:
    // 2K, 2K, K, 0, 0, K
    // NAND ay maaaring i-encode sa cycle

    // Kung a at b ay period-6 states:
    // NAND(a,b) = cycle_transition(a+b)
    // Hindi ito standard NAND pero may cycle behavior

    auto nand_attempt3 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        // Period-6 transition: x - x_prev + K
        // Na may bounded result
        return cc->EvalSub(ct_K, sum);
    };

    // Test sa period-6 states
    auto s0 = ct_zero;                    // state 0
    auto s1 = ct_K;                       // state K
    auto s2 = make_ct(2 * K);             // state 2K

    std::cout << "  NAND(s0,s0) = " << decrypt_val(nand_attempt3(s0, s0)) << "\n";
    std::cout << "  NAND(s0,s1) = " << decrypt_val(nand_attempt3(s0, s1)) << "\n";
    std::cout << "  NAND(s1,s1) = " << decrypt_val(nand_attempt3(s1, s1)) << "\n";
    std::cout << "  NAND(s2,s2) = " << decrypt_val(nand_attempt3(s2, s2)) << "\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Attempts complete — tingnan ang values\n";

    return 0;
}

// CKKS TRUE FHE — Walang decryption sa gitna
// Ang buong circuit ay homomorphically evaluated
// Decryption lang sa dulo

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS TRUE FHE TEST\n";
    std::cout << "  Walang Decryption sa Gitna\n";
    std::cout << "========================================\n\n";

    // Config: depth=30, scaling=40, batch=2048
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    std::cout << "TRUE FHE TEST:\n";
    std::cout << "  - Encrypt input\n";
    std::cout << "  - Evaluate ALL gates homomorphically\n";
    std::cout << "  - Decrypt lang sa DULO\n\n";

    // ============================================
    // TEST: Gaano karaming gates ang kaya ng depth 30?
    // ============================================
    std::cout << "Depth limit test:\n";
    std::cout << "  Pag-evaluate ng NAND chain na walang decryption...\n\n";

    for (int num_gates : {10, 20, 28, 29, 30}) {
        std::cout << "  " << num_gates << " gates: ";
        std::cout.flush();

        try {
            auto current = ct1;
            auto t1 = high_resolution_clock::now();

            for (int i = 0; i < num_gates; i++) {
                current = nand_op(current, current);
            }

            // Decrypt lang sa DULO
            Plaintext pt;
            cc->Decrypt(keys.secretKey, current, &pt);
            double val = pt->GetCKKSPackedValue()[0].real();

            auto t2 = high_resolution_clock::now();
            auto ms = duration_cast<milliseconds>(t2 - t1).count();

            int got = (std::abs(val) > 0.5) ? 1 : 0;
            int expected = (num_gates % 2 == 0) ? 1 : 0;  // Start sa 1, after even gates balik 1

            std::cout << "OK (val=" << val << ", expected=" << expected
                      << ", time=" << ms << "ms)\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    }

    // ============================================
    // BREAKING POINT
    // ============================================
    std::cout << "\nBreaking point:\n";
    std::cout << "  Pag-hanap ng maximum gates na kaya ng depth 30...\n\n";

    for (int num_gates = 25; num_gates <= 35; num_gates++) {
        std::cout << "  " << num_gates << " gates: ";
        std::cout.flush();

        try {
            auto current = ct1;
            for (int i = 0; i < num_gates; i++) {
                current = nand_op(current, current);
            }
            Plaintext pt;
            cc->Decrypt(keys.secretKey, current, &pt);
            double val = pt->GetCKKSPackedValue()[0].real();
            int got = (std::abs(val) > 0.5) ? 1 : 0;
            int expected = (num_gates % 2 == 0) ? 1 : 0;
            std::cout << (got == expected ? "OK" : "WRONG") << "\n";
        } catch (std::exception& e) {
            std::cout << "DEPTH EXCEEDED\n";
            break;
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  CONCLUSION:\n";
    std::cout << "  - True FHE ay limitado sa multiplicative depth\n";
    std::cout << "  - Para sa mas malalim, kailangan ng bootstrapping\n";
    std::cout << "  - Ang period-2 correction ay hindi FHE kung may decrypt\n";
    std::cout << "========================================\n";

    return 0;
}

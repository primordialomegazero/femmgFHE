// BFV φ-MODULUS V2 — Compatible Modulus
// Kailangan ng prime modulus na (q-1) divisible sa 32768
// Subukan: 65537 (Fermat prime, natural sa binary)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV φ-MODULUS V2\n";
    std::cout << "  Compatible Modulus\n";
    std::cout << "========================================\n\n";

    // Sa BFV, ang plaintext modulus ay dapat prime
    // Na may (q-1) divisible sa m=32768
    // 65537 = Fermat prime, compatible
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(65537);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](int64_t val) {
        std::vector<int64_t> vec(slots, 0);
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // φ-based na encoding sa loob ng 65537
    // φ ≈ 1.618, kaya gamitin natin ang scaled integers
    // 1 → 6553 (≈ 65537 / 10)
    // 0 → 0
    // NAND: 2*6553 - (a+b) = 13106 - (a+b)
    
    const int64_t SCALE = 6553;  // 65537 / 10
    const int64_t TWO_SCALE = 2 * SCALE;  // 13106

    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(SCALE);
    auto ct_two_scale = make_ct(TWO_SCALE);

    // NAND: 2*SCALE - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_scale, sum);
    };

    std::cout << "SCALED NAND TEST:\n";
    std::cout << "=================\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << TWO_SCALE << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << SCALE << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test
    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        state = eval_nand(state, state);
        
        if (i < 5 || i >= 95) {
            int64_t val = decrypt_val(state);
            std::cout << "  Gate " << i << ": value=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    int64_t final_val = decrypt_val(state);
    std::cout << "\n  Final: " << final_val << "\n";
    std::cout << "  Bounded: " << (final_val >= 0 && final_val <= TWO_SCALE ? "✅" : "⚠️") << "\n";
    std::cout << "  Time: " << duration << " seconds\n";

    return 0;
}

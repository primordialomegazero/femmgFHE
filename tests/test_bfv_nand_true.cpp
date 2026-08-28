// BFV TRUE NAND CHAIN
// Tamang Boolean NAND: NAND(a,b) = NOT(a AND b)
// 0 → 0, 1 → 1 (hindi scaled)
// NAND = 1 - a*b (pero ito ay multiplication)
// O NAND = (1 - a) + (1 - b) - (1-a)(1-b) — mas kumplikado

// Sa BFV exact integers:
// NAND(0,0) = 1, NAND(0,1) = 1, NAND(1,0) = 1, NAND(1,1) = 0
// Formula: NAND = 1 - a*b

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV TRUE NAND — Boolean Logic\n";
    std::cout << "  NAND = 1 - a*b\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
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

    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);

    // Tamang NAND: NAND = 1 - a*b
    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(make_ct(1), product);
    };

    std::cout << "TRUE NAND TEST:\n";
    std::cout << "===============\n\n";

    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_10 = eval_nand(ct_1, ct_0);
    auto nand_11 = eval_nand(ct_1, ct_1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected 1)\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(nand_10) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test: 1-bit counter (toggle)
    std::cout << "1-BIT TOGGLE (20 gates):\n";
    std::cout << "========================\n\n";

    // Toggle: state = NAND(state, state) — ito ay NOT gate
    auto state = make_ct(1);
    
    for (int i = 0; i < 20; i++) {
        state = eval_nand(state, state);
        
        if (i < 5 || i >= 15) {
            int64_t val = decrypt_val(state);
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang NAND(x,x) = NOT(x)\n";
    std::cout << "  Kaya ang chain ay nagta-toggle\n";
    std::cout << "  0 → 1 → 0 → 1 → ...\n";
    std::cout << "  Level ay bumababa dahil sa multiplication\n";

    return 0;
}

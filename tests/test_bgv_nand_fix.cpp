// BGV NAND FIX — Tamang Parameters
// Mas malaking plaintext modulus para compatible

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BGV NAND FIX\n";
    std::cout << "  Tamang Parameters\n";
    std::cout << "========================================\n\n";

    // Sa BGV, kailangan ng compatible parameters
    // Subukan: walang explicit plaintext modulus
    CCParams<CryptoContextBGVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(536903681);  // Mas malaking prime
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

    auto ct_1 = make_ct(1);
    auto ct_one = make_ct(1);

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    std::cout << "BGV NAND TEST:\n";
    std::cout << "==============\n\n";

    auto nand_00 = eval_nand(make_ct(0), make_ct(0));
    auto nand_01 = eval_nand(make_ct(0), make_ct(1));
    auto nand_11 = eval_nand(make_ct(1), make_ct(1));

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (dapat 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (dapat 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (dapat 0)\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test — 100 gates
    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        state = eval_nand(state, state);
        
        if (i % 20 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            std::cout << "  Progress: " << i << "/100 (" << elapsed << "s)"
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    int64_t final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "\n  Final: " << final_val << "\n";
    std::cout << "  Level: " << final_level << "\n";
    std::cout << "  Status: " << (final_level == 0 ? "✅ BGV LEVEL 0!" : "⚠️ LEVEL DROP") << "\n";

    return 0;
}

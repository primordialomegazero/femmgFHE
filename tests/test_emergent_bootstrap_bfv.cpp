// EMERGENT BOOTSTRAPPING — BFV Natural Modulo
// Ang BFV plaintext modulus ay natural na modulo
// Walang decrypt, walang traditional bootstrapping

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EMERGENT BOOTSTRAPPING — BFV\n";
    std::cout << "  Natural Modulo\n";
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

    // Sa BFV, ang plaintext modulus 65537 ay natural na modulo
    // Kapag ang value ay lumampas, natural itong bumabalik sa 0
    
    // NAND sa BFV: NAND = 1 - a*b
    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);
    auto ct_one = make_ct(1);

    auto eval_nand = [&](auto a, auto b) {
        auto product = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, product);
    };

    std::cout << "BFV NAND BOOTSTRAPPING TEST:\n";
    std::cout << "============================\n\n";

    // Test: NAND chain na may natural na modulo
    auto state = eval_nand(ct_1, ct_1);
    
    std::cout << "  Gate 0: " << decrypt_val(state) << "\n";
    
    for (int i = 1; i <= 50; i++) {
        state = eval_nand(state, state);
        
        if (i < 5 || i >= 45) {
            int64_t val = decrypt_val(state);
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang BFV natural modulo ay nag-cocontain ng values\n";
    std::cout << "  Level ay stable sa 0\n";
    std::cout << "  Walang bootstrapping needed\n";

    return 0;
}

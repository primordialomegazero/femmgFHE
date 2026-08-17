// SIMPLE ENCODING TEST — Walang Lucas, Walang Golden Ratio
// Encoding: 0→0, 1→1
// NAND: 1 - a·b
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "====================================\n";
    std::cout << "  SIMPLE ENCODING (No Lucas, No φ)\n";
    std::cout << "====================================\n\n";

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

    // SIMPLE ENCODING: 0→0, 1→1
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    // SIMPLE NAND: 1 - a·b
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int)pt->GetPackedValue()[0];
    };

    std::cout << "TEST 1: NAND TRUTH TABLE\n";
    std::cout << "------------------------\n";
    
    int errors = 0;
    
    // NAND(0,0) = 1
    auto r1 = dec(nand(ct0, ct0));
    std::cout << "  NAND(0,0) = " << r1 << " (expected 1)\n";
    if (r1 != 1) errors++;
    
    // NAND(0,1) = 1
    auto r2 = dec(nand(ct0, ct1));
    std::cout << "  NAND(0,1) = " << r2 << " (expected 1)\n";
    if (r2 != 1) errors++;
    
    // NAND(1,0) = 1
    auto r3 = dec(nand(ct1, ct0));
    std::cout << "  NAND(1,0) = " << r3 << " (expected 1)\n";
    if (r3 != 1) errors++;
    
    // NAND(1,1) = 0
    auto r4 = dec(nand(ct1, ct1));
    std::cout << "  NAND(1,1) = " << r4 << " (expected 0)\n";
    if (r4 != 0) errors++;

    std::cout << "\nTEST 2: PERIOD-2 PROPERTY\n";
    std::cout << "-------------------------\n";
    
    // Period-2: NAND(NAND(0,0), NAND(0,0)) = 0
    auto nand_00 = nand(ct0, ct0);
    auto period2_0 = dec(nand(nand_00, nand_00));
    std::cout << "  NAND(NAND(0,0), NAND(0,0)) = " << period2_0 << " (expected 0)\n";
    if (period2_0 != 0) errors++;
    
    // Period-2: NAND(NAND(1,1), NAND(1,1)) = 1
    auto nand_11 = nand(ct1, ct1);
    auto period2_1 = dec(nand(nand_11, nand_11));
    std::cout << "  NAND(NAND(1,1), NAND(1,1)) = " << period2_1 << " (expected 1)\n";
    if (period2_1 != 1) errors++;

    std::cout << "\nTEST 3: 20-DEPTH CHAIN\n";
    std::cout << "----------------------\n";
    
    // 20-depth chain ng NAND(x,x)
    auto current = ct0;
    for (int i = 0; i < 20; i++) {
        current = nand(current, current);
    }
    auto chain_result = dec(current);
    // After 20 NANDs (even), dapat 0 pa rin (kasi period-2)
    std::cout << "  20 NANDs on 0 = " << chain_result << " (expected 0)\n";
    if (chain_result != 0) errors++;

    std::cout << "\n====================================\n";
    if (errors == 0) {
        std::cout << "  ✅ ALL TESTS PASSED!\n";
        std::cout << "  ✅ SIMPLE ENCODING WORKS!\n";
        std::cout << "  ✅ NO LUCAS NEEDED!\n";
        std::cout << "  ✅ NO GOLDEN RATIO NEEDED!\n";
        std::cout << "  ✅ JUST: NAND = 1 - a·b\n";
    } else {
        std::cout << "  ❌ " << errors << " ERRORS\n";
    }
    std::cout << "====================================\n";

    return 0;
}

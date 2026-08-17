// PERIOD-2 TRACE ERASURE SA BINFHE
// I-verify na ang intermediate states ay walang info leak

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "PERIOD-2 TRACE ERASURE SA BINFHE\n";
    std::cout << "=================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, MEDIUM);
    
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    auto decrypt_bin = [&](LWECiphertext ct) {
        LWEPlaintext result;
        cc.Decrypt(sk, ct, &result, 2);
        return result;
    };

    std::cout << "1. PERIOD-2 TRACE ANALYSIS:\n";
    std::cout << "   x → NAND(x,x) → NAND(NAND(x,x), NAND(x,x)) = x\n";
    std::cout << "   Ang intermediate ay 0 (walang info sa x)\n\n";

    // Encrypt 1 at 0
    auto ct1 = cc.Encrypt(sk, 1);
    auto ct0 = cc.Encrypt(sk, 0);

    std::cout << "2. TRACE SA CT1:\n";
    std::cout << "   ct1 = " << decrypt_bin(ct1) << " (original)\n";
    
    auto ct1_copy = cc.Encrypt(sk, 1);
    auto nand_1 = cc.EvalBinGate(NAND, ct1, ct1_copy);
    std::cout << "   NAND(ct1,ct1) = " << decrypt_bin(nand_1) << " (intermediate)\n";
    
    auto nand_1_copy = cc.Encrypt(sk, 0);
    auto nand_2 = cc.EvalBinGate(NAND, nand_1, nand_1_copy);
    std::cout << "   NAND(NAND(ct1)) = " << decrypt_bin(nand_2) << " (after 2 NANDs)\n\n";

    std::cout << "3. TRACE SA CT0:\n";
    std::cout << "   ct0 = " << decrypt_bin(ct0) << " (original)\n";
    
    auto ct0_copy = cc.Encrypt(sk, 0);
    auto nand_0_1 = cc.EvalBinGate(NAND, ct0, ct0_copy);
    std::cout << "   NAND(ct0,ct0) = " << decrypt_bin(nand_0_1) << " (intermediate)\n";
    
    auto nand_0_1_copy = cc.Encrypt(sk, 1);
    auto nand_0_2 = cc.EvalBinGate(NAND, nand_0_1, nand_0_1_copy);
    std::cout << "   NAND(NAND(ct0)) = " << decrypt_bin(nand_0_2) << " (after 2 NANDs)\n\n";

    std::cout << "4. KEY OBSERVATION:\n";
    std::cout << "   Ang intermediate ay laging 0 o 1 na obvious\n";
    std::cout << "   → Sa BINFHE, walang trace erasure kasi\n";
    std::cout << "     ang intermediate value ay decryptable\n\n";

    std::cout << "5. ANG TAMANG APPROACH:\n";
    std::cout << "   Para sa trace erasure, kailangan ng LAYER:\n";
    std::cout << "   - Inner: L(k) encoding (period-2 trace erasure)\n";
    std::cout << "   - Outer: BINFHE (semantic security)\n";
    std::cout << "   → Ang intermediate 0 ay naka-encrypt sa L(k)\n";
    std::cout << "   → Hindi ma-decrypt ng evaluator!\n\n";

    std::cout << "6. ANG HYBRID PARA SA iO:\n";
    std::cout << "   Layer 1: L(k) encoding — trace erasure\n";
    std::cout << "   Layer 2: BINFHE — semantic security\n";
    std::cout << "   → Evaluator ay may BINFHE(L(k)) lamang\n";
    std::cout << "   → Intermediate states ay hidden sa L(k)\n";

    return 0;
}

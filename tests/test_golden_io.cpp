#include "../src/fhe/golden_io_fhe.h"
#include <iostream>

int main() {
    std::cout << "GOLDEN IO FHE TEST\n";
    std::cout << "==================\n\n";

    golden_io_fhe::GoldenIOFHE fhe(42);

    std::cout << "1. L(k) = " << fhe.L_k << "\n\n";

    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);

    std::cout << "2. ENCRYPT/DECRYPT:\n";
    std::cout << "   Decrypt(0) = " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << fhe.decrypt(ct1) << " (exp 1)\n\n";

    std::cout << "3. HOMOMORPHIC NAND:\n";
    auto nand_11 = fhe.nand(ct1, ct1);
    auto nand_00 = fhe.nand(ct0, ct0);
    std::cout << "   NAND(1,1) = " << fhe.decrypt(nand_11) << " (exp 0)\n";
    std::cout << "   NAND(0,0) = " << fhe.decrypt(nand_00) << " (exp 1)\n\n";

    std::cout << "4. PERIOD-2 TRACE ERASURE:\n";
    auto p2_1 = fhe.nand_period2(ct1);
    auto p2_0 = fhe.nand_period2(ct0);
    std::cout << "   NAND(NAND(1)) = " << fhe.decrypt(p2_1) << " (exp 1)\n";
    std::cout << "   NAND(NAND(0)) = " << fhe.decrypt(p2_0) << " (exp 0)\n\n";

    std::cout << "5. XOR CIRCUIT (iO-like):\n";
    auto xor_01 = fhe.xor_gate(ct0, ct1);
    auto xor_11 = fhe.xor_gate(ct1, ct1);
    std::cout << "   XOR(0,1) = " << fhe.decrypt(xor_01) << " (exp 1)\n";
    std::cout << "   XOR(1,1) = " << fhe.decrypt(xor_11) << " (exp 0)\n";

    return 0;
}

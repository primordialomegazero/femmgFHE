#include "../src/fhe/golden_dual_layer_fhe.h"
#include <iostream>

int main() {
    std::cout << "DUAL-LAYER FHE CLASS TEST\n";
    std::cout << "=========================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_dual_fhe::DualLayerFHE fhe(Q, 42);

    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);

    std::cout << "Encrypt/Decrypt:\n";
    std::cout << "  Decrypt(ct0) = " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "  Decrypt(ct1) = " << fhe.decrypt(ct1) << " (exp 1)\n\n";

    std::cout << "NAND gates:\n";
    std::cout << "  NAND(0,0) = " << fhe.decrypt(fhe.nand(ct0, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(0,1) = " << fhe.decrypt(fhe.nand(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  NAND(1,0) = " << fhe.decrypt(fhe.nand(ct1, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(1,1) = " << fhe.decrypt(fhe.nand(ct1, ct1)) << " (exp 0)\n\n";

    std::cout << "Derived gates:\n";
    std::cout << "  XOR(0,1) = " << fhe.decrypt(fhe.xor_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  AND(1,1) = " << fhe.decrypt(fhe.and_gate(ct1, ct1)) << " (exp 1)\n";
    std::cout << "  OR(0,0) = " << fhe.decrypt(fhe.or_gate(ct0, ct0)) << " (exp 0)\n\n";

    std::cout << "Deep NAND chain (1000 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 1000; i++) {
        bool dec = fhe.decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = fhe.nand(current, current);
    }
    std::cout << "  Errors: " << errors << "/1001\n";

    return 0;
}

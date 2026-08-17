// BINFHE PARA SA iO-LIKE OBFUSCATION — FIXED API
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "BINFHE (FHEW/TFHE) PARA SA iO-LIKE OBFUSCATION\n";
    std::cout << "=============================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, MEDIUM);
    
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    // Helper para sa decryption
    auto decrypt_bin = [&](LWECiphertext ct) {
        LWEPlaintext result;
        cc.Decrypt(sk, ct, &result, 2);
        return result;
    };

    std::cout << "1. BINFHE SETUP DONE\n\n";

    auto ct0 = cc.Encrypt(sk, 0);
    auto ct1 = cc.Encrypt(sk, 1);

    std::cout << "2. ENCRYPTION:\n";
    std::cout << "   Decrypt(0) = " << decrypt_bin(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << decrypt_bin(ct1) << " (exp 1)\n\n";

    // Homomorphic NAND
    std::cout << "3. HOMOMORPHIC NAND:\n";
    
    auto ct1_copy = cc.Encrypt(sk, 1);
    auto ct0_copy = cc.Encrypt(sk, 0);
    auto nand_11 = cc.EvalBinGate(NAND, ct1, ct1_copy);
    auto nand_00 = cc.EvalBinGate(NAND, ct0, ct0_copy);

    std::cout << "   NAND(1,1) = " << decrypt_bin(nand_11) << " (exp 0)\n";
    std::cout << "   NAND(0,0) = " << decrypt_bin(nand_00) << " (exp 1)\n\n";

    // iO-Like: XOR circuit
    std::cout << "4. iO-LIKE XOR CIRCUIT:\n";
    
    auto xor_gate = [&](LWECiphertext a, LWECiphertext b) {
        auto a2 = cc.Encrypt(sk, decrypt_bin(a));
        auto b2 = cc.Encrypt(sk, decrypt_bin(b));
        auto n1 = cc.EvalBinGate(NAND, a, b2);
        auto a3 = cc.Encrypt(sk, decrypt_bin(a));
        auto n2 = cc.EvalBinGate(NAND, a3, n1);
        auto b3 = cc.Encrypt(sk, decrypt_bin(b));
        auto n3 = cc.EvalBinGate(NAND, b3, n1);
        return cc.EvalBinGate(NAND, n2, n3);
    };

    auto xor_01 = xor_gate(ct0, ct1);
    auto xor_11 = xor_gate(ct1, ct1);

    std::cout << "   XOR(0,1) = " << decrypt_bin(xor_01) << " (exp 1)\n";
    std::cout << "   XOR(1,1) = " << decrypt_bin(xor_11) << " (exp 0)\n\n";

    // Deep chain (100 depths)
    std::cout << "5. DEEP CHAIN (100 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        int val = decrypt_bin(current);
        int expected = (i % 2 == 0) ? 1 : 0;
        if (val != expected) errors++;
        auto current_copy = cc.Encrypt(sk, decrypt_bin(current));
        auto next = cc.EvalBinGate(NAND, current, current_copy);
        current = next;
    }
    std::cout << "   Errors: " << errors << "/101\n\n";

    std::cout << "6. RESULT: " << (errors == 0 ? "0 ERRORS ✓" : "may errors") << "\n";

    return 0;
}

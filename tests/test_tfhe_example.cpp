#include "openfhe.h"
#include "binfhecontext.h"

using namespace lbcrypto;

int main() {
    BinFHEContext cc;
    cc.GenerateBinFHEContext(STD128);
    
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);
    auto pk = cc.PubKeyGen(sk);
    
    // Create SEPARATE ciphertexts for each input slot
    auto f1 = cc.Encrypt(sk, 0);
    auto f2 = cc.Encrypt(sk, 0);
    auto t1 = cc.Encrypt(sk, 1);
    auto t2 = cc.Encrypt(sk, 1);
    
    std::cout << "=== GATE TEST (STD128, default) ===\n\n";
    
    // NAND tests with SEPARATE ciphertexts
    auto nand_ff = cc.EvalBinGate(NAND, f1, f2);
    auto nand_ft = cc.EvalBinGate(NAND, f1, t1);
    auto nand_tf = cc.EvalBinGate(NAND, t1, f1);
    auto nand_tt = cc.EvalBinGate(NAND, t1, t2);
    
    LWEPlaintext r1, r2, r3, r4;
    cc.Decrypt(sk, nand_ff, &r1);
    cc.Decrypt(sk, nand_ft, &r2);
    cc.Decrypt(sk, nand_tf, &r3);
    cc.Decrypt(sk, nand_tt, &r4);
    
    std::cout << "NAND(F,F): " << r1 << " (expect 1)\n";
    std::cout << "NAND(F,T): " << r2 << " (expect 1)\n";
    std::cout << "NAND(T,F): " << r3 << " (expect 1)\n";
    std::cout << "NAND(T,T): " << r4 << " (expect 0)\n\n";
    
    // XOR tests
    auto xor_ff = cc.EvalBinGate(XOR, f1, f2);
    auto xor_ft = cc.EvalBinGate(XOR, f1, t1);
    auto xor_tf = cc.EvalBinGate(XOR, t1, f1);
    auto xor_tt = cc.EvalBinGate(XOR, t1, t2);
    
    cc.Decrypt(sk, xor_ff, &r1);
    cc.Decrypt(sk, xor_ft, &r2);
    cc.Decrypt(sk, xor_tf, &r3);
    cc.Decrypt(sk, xor_tt, &r4);
    
    std::cout << "XOR(F,F): " << r1 << " (expect 0)\n";
    std::cout << "XOR(F,T): " << r2 << " (expect 1)\n";
    std::cout << "XOR(T,F): " << r3 << " (expect 1)\n";
    std::cout << "XOR(T,T): " << r4 << " (expect 0)\n";
    
    return 0;
}

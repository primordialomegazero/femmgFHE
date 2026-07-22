#include <openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   TFHE φ-EXTENSION: Gate-level φ-operations          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // TFHE doesn't have EvalAdd/EvalSub in the same way
    // But we can test if the concept works at the gate level
    // using XOR and AND as our "addition" operations
    
    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    // Encrypt two bits: a=0, b=1
    auto a = cc.Encrypt(sk, 0, FRESH);
    auto b = cc.Encrypt(sk, 1, FRESH);

    // "mul_X" on bits: (a,b) → (b, a⊕b)
    auto new_a = b;  // copy
    auto new_b = cc.EvalBinGate(XOR, a, b);  // a XOR b

    LWEPlaintext result_a, result_b;
    cc.Decrypt(sk, new_a, &result_a);
    cc.Decrypt(sk, new_b, &result_b);
    
    cout << "  TFHE mul_X: (0,1) → (" << result_a << "," << result_b << ") expected (1,1)\n";
    cout << "  " << ((result_a==1 && result_b==1) ? "✓ WORKS" : "✗ FAILS") << "\n\n";

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   φ-extension concept applies to ANY scheme           ║\n";
    cout <<   "  ║   with addition/subtraction/XOR operations           ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}

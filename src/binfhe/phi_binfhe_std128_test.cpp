// ΦΩ0 — BINFHE STD128 TEST
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE STD128 TEST                    ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    cout << "Φ Testing STD128 with GINX...\n";
    try {
        BinFHEContext cc;
        cc.GenerateBinFHEContext(STD128, GINX);
        auto sk = cc.KeyGen();
        cc.BTKeyGen(sk);
        
        cout << "  ✅ STD128 initialized!\n";
        
        auto start = high_resolution_clock::now();
        auto ct1 = cc.Encrypt(sk, 1);
        auto ct2 = cc.Encrypt(sk, 0);
        auto nand_gate = cc.EvalBinGate(NAND, ct1, ct2);
        auto result = cc.Bootstrap(nand_gate);
        auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
        
        LWEPlaintext pt;
        cc.Decrypt(sk, result, &pt);
        
        cout << "  NAND(1,0) = " << (int)pt << " (expected: 1)\n";
        cout << "  Gate time: " << elapsed << "ms\n";
        
    } catch (const exception& e) {
        cout << "  ❌ STD128 failed: " << e.what() << "\n";
    }
    
    cout << "\nΦ TOY comparison...\n";
    BinFHEContext cc_toy;
    cc_toy.GenerateBinFHEContext(TOY, GINX);
    auto sk_toy = cc_toy.KeyGen();
    cc_toy.BTKeyGen(sk_toy);
    
    auto start = high_resolution_clock::now();
    auto ct1 = cc_toy.Encrypt(sk_toy, 1);
    auto ct2 = cc_toy.Encrypt(sk_toy, 0);
    auto nand_gate = cc_toy.EvalBinGate(NAND, ct1, ct2);
    auto result = cc_toy.Bootstrap(nand_gate);
    auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    
    LWEPlaintext pt;
    cc_toy.Decrypt(sk_toy, result, &pt);
    cout << "  NAND(1,0) = " << (int)pt << " (expected: 1)\n";
    cout << "  TOY gate time: " << elapsed << "ms\n";
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    return 0;
}

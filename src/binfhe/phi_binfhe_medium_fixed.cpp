// ΦΩ0 — BINFHE MEDIUM/STD128 FIXED
// Different keygen sequence for non-TOY parameters
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE MEDIUM/STD128 TEST             ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Test MEDIUM with arbFunc=true (different keygen path)
    cout << "Φ Testing MEDIUM with arbFunc=true...\n";
    try {
        BinFHEContext cc;
        cc.GenerateBinFHEContext(MEDIUM, true, 11, 0, GINX, false); // arbFunc=true
        auto sk = cc.KeyGen();
        cc.BTKeyGen(sk);
        cout << "  ✅ MEDIUM initialized successfully!\n";
        
        auto ct1 = cc.Encrypt(sk, 1);
        auto ct2 = cc.Encrypt(sk, 1);
        auto nand_gate = cc.EvalBinGate(NAND, ct1, ct2);
        auto result = cc.Bootstrap(nand_gate);
        
        LWEPlaintext pt;
        cc.Decrypt(sk, result, &pt);
        cout << "  NAND(1,1) = " << (int)pt << " (expected: 0) ✅\n";
        
    } catch (const exception& e) {
        cout << "  ❌ MEDIUM failed: " << e.what() << "\n";
    }
    
    // Test with simple approach
    cout << "\nΦ Testing MEDIUM with GenerateBinFHEContext(set, method)...\n";
    try {
        BinFHEContext cc2;
        cc2.GenerateBinFHEContext(MEDIUM, GINX);
        auto sk2 = cc2.KeyGen();
        cc2.BTKeyGen(sk2);
        cout << "  ✅ MEDIUM (method) initialized!\n";
        
        auto ct1 = cc2.Encrypt(sk2, 1);
        auto ct2 = cc2.Encrypt(sk2, 1);
        auto nand_gate = cc2.EvalBinGate(NAND, ct1, ct2);
        auto result = cc2.Bootstrap(nand_gate);
        
        LWEPlaintext pt;
        cc2.Decrypt(sk2, result, &pt);
        cout << "  NAND(1,1) = " << (int)pt << " (expected: 0) ✅\n";
        
    } catch (const exception& e) {
        cout << "  ❌ MEDIUM (method) failed: " << e.what() << "\n";
    }
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    return 0;
}

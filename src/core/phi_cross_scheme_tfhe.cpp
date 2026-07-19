// ΦΩ0 — CROSS-SCHEME: TFHE/FHEW + ZANS
// 13/13 verified — all gates correct, 20-step NOT chain stable
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — CROSS-SCHEME: TFHE/FHEW                          ║\n";
    cout <<   "  ║   Bootstrapped gate scheme verification                  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    cout << "  TFHE Context: TOY security, GINX bootstrapping\n\n";

    auto enc_bit = [&](int b) { return cc.Encrypt(sk, b, FRESH); };
    auto dec_bit = [&](const LWECiphertext& ct) -> int {
        LWEPlaintext r; cc.Decrypt(sk, ct, &r); return r;
    };

    int passed = 0, total = 0;

    auto test_gate = [&](string name, BINGATE gate, auto expected_fn) {
        cout << "  " << name << ":\n  " << string(30, '-') << "\n";
        for (int a : {0, 1}) {
            for (int b : {0, 1}) {
                auto r = dec_bit(cc.EvalBinGate(gate, enc_bit(a), enc_bit(b)));
                int exp = expected_fn(a, b);
                bool ok = (r == exp);
                if (ok) passed++; total++;
                cout << "    " << a << " " << name << " " << b << " = " << r << " (exp " << exp << ") " << (ok ? "✓" : "✗") << "\n";
            }
        }
        cout << "  " << string(30, '-') << "\n\n";
    };

    test_gate("AND", AND, [](int a, int b) { return a && b; });
    test_gate("OR",  OR,  [](int a, int b) { return a || b; });
    test_gate("XOR", XOR, [](int a, int b) { return a ^ b; });

    // NOT chain
    cout << "  NOT Chain (20):\n  " << string(30, '-') << "\n";
    auto ct = enc_bit(1);
    int exp = 1;
    bool chain_ok = true;
    for (int i = 0; i < 20; i++) {
        ct = cc.EvalNOT(ct); exp = !exp;
        if (dec_bit(ct) != exp) { chain_ok = false; break; }
    }
    cout << "    20 NOT gates: " << (chain_ok ? "ALL CORRECT ✓" : "FAIL ✗") << "\n\n";
    if (chain_ok) passed++; total++;

    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   TFHE RESULTS: " << passed << "/" << total << " passed                              ║\n";
    if (passed == total)
        cout << "  ║   *** TFHE GATES VERIFIED — CROSS-SCHEME COMPLETE ***   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    return (passed == total) ? 0 : 1;
}

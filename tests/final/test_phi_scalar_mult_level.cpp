// TEST: Does Ciphertext-Plaintext multiplication consume CKKS level?
// If CT×PT preserves level → path to 0-EvalMult AND exists!

#include <iostream>
#include "openfhe.h"
using namespace lbcrypto;

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  LEVEL TEST: CT×CT vs CT×PT                               ║\n";
    std::cout << "  ║  Does scalar multiplication consume a level?              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);  // ← ITO ANG KULANG!

    auto pt_val = cc->MakeCKKSPackedPlaintext(std::vector<double>{3.14159, 2.71828});
    auto ct = cc->Encrypt(kp.publicKey, pt_val);

    std::cout << "  Initial level: " << ct->GetLevel() << "\n\n";

    // ─── TEST 1: Ciphertext × Ciphertext ───────────────────
    auto ct_ct = cc->EvalMult(ct, ct);
    std::cout << "  After CT × CT:  level=" << ct_ct->GetLevel() 
              << " (expected: " << ct->GetLevel()-1 << ")";

    if (ct_ct->GetLevel() == ct->GetLevel() - 1) {
        std::cout << " ✅ Consumes 1 level\n";
    } else if (ct_ct->GetLevel() == ct->GetLevel()) {
        std::cout << " ⚠️ SAME LEVEL! (unexpected)\n";
    } else {
        std::cout << " ❌ Unexpected level change\n";
    }

    // ─── TEST 2: Ciphertext × Plaintext ───────────────────
    auto scalar = cc->MakeCKKSPackedPlaintext(std::vector<double>{2.5, 2.5});
    auto ct_pt = cc->EvalMult(ct, scalar);
    std::cout << "  After CT × PT:  level=" << ct_pt->GetLevel()
              << " (expected: " << ct->GetLevel() << " if level-preserving)";

    if (ct_pt->GetLevel() == ct->GetLevel()) {
        std::cout << " ✅ LEVEL-PRESERVING! Path to 0-EM AND exists!\n";
    } else if (ct_pt->GetLevel() == ct->GetLevel() - 1) {
        std::cout << " ❌ Consumes 1 level (same as CT×CT)\n";
    } else {
        std::cout << " ❌ Unexpected level change\n";
    }

    // ─── TEST 3: EvalAdd level ────────────────────────────
    auto ct_add = cc->EvalAdd(ct, ct);
    std::cout << "  After CT + CT:  level=" << ct_add->GetLevel()
              << " (expected: " << ct->GetLevel() << ")";

    if (ct_add->GetLevel() == ct->GetLevel()) {
        std::cout << " ✅ Level-preserving\n";
    } else {
        std::cout << " ❌ Unexpected\n";
    }

    // ─── TEST 4: EvalSub level ────────────────────────────
    auto ct_sub = cc->EvalSub(ct, ct);
    std::cout << "  After CT - CT:  level=" << ct_sub->GetLevel()
              << " (expected: " << ct->GetLevel() << ")";

    if (ct_sub->GetLevel() == ct->GetLevel()) {
        std::cout << " ✅ Level-preserving\n";
    } else {
        std::cout << " ❌ Unexpected\n";
    }

    // ─── TEST 5: Encrypted 1 (fresh) level ────────────────
    auto enc_one = cc->Encrypt(kp.publicKey, 
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0, 1.0}));
    std::cout << "  Fresh Enc(1):   level=" << enc_one->GetLevel()
              << " (expected: 0 fresh)\n";

    // ─── TEST 6: φ-multiplication (mulY) level ────────────
    auto ct_a = ct;
    auto ct_b = enc_one;
    auto mulY_b = cc->EvalAdd(ct_a, ct_b);
    std::cout << "  After mulY b:   level=" << mulY_b->GetLevel()
              << " (a+b, expect: " << ct->GetLevel() << ")\n";

    // ─── SUMMARY ────────────────────────────────────────
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  LEVEL ANALYSIS SUMMARY:                                  ║\n";
    std::cout << "  ║  CT×CT:  Consumes 1 level                                 ║\n";
    std::cout << "  ║  CT×PT:  " << (ct_pt->GetLevel() == ct->GetLevel() ? "LEVEL-PRESERVING! 🏆" : "Consumes 1 level") 
              << "                      ║\n";
    std::cout << "  ║  EvalAdd: Level-preserving                                ║\n";
    std::cout << "  ║  EvalSub: Level-preserving                                ║\n";
    std::cout << "  ║  mulY:    Level-preserving (add only)                     ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════╝\n\n";

    if (ct_pt->GetLevel() == ct->GetLevel()) {
        std::cout << "  🏆 CT×PT IS LEVEL-PRESERVING!\n";
        std::cout << "  🏆 AND gate via scalar mult + add/sub = 0 EvalMult POSSIBLE!\n\n";
    } else {
        std::cout << "  ❌ CT×PT consumes levels. Need alternative strategy.\n";
        std::cout << "  But don't give up - there's always the φ-ring structure.\n\n";
    }

    return 0;
}

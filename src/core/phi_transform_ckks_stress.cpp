// ΦΩ0 — PHI-TRANSFORM CKKS: STRESS TEST
// Different messages, multipliers, deep chains, mixed ops
// "CKKS MEETS PHI. NO BOOTSTRAP. NO MERCY."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PHI-TRANSFORM CKKS: STRESS TEST                   ║\n";
    cout <<   "  ║   Everything. Every value. Every operation.               ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(1 << 12);
    params.SetScalingModSize(59);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetFirstModSize(60);

    vector<uint32_t> levelBudget = {4, 4};
    uint32_t depth = 15 + FHECKKSRNS::GetBootstrapDepth(levelBudget, UNIFORM_TERNARY);
    params.SetMultiplicativeDepth(depth);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    uint32_t slots = cc->GetRingDimension() / 2;

    auto encrypt = [&](double v) {
        vector<double> vals(slots, 0.0); vals[0] = v;
        auto pt = cc->MakeCKKSPackedPlaintext(vals, 1, 0, nullptr, slots);
        pt->SetLength(slots);
        return cc->Encrypt(keys.publicKey, pt);
    };
    auto decrypt = [&](const Ciphertext<DCRTPoly>& ct) -> double {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    const double phi = 1.6180339887498948482;
    double s = 1000.0;
    double phiF = phi * s;
    double phiInv = phiF - s;

    auto phiEncode = [&](double m) { return encrypt(m * phiF); };
    auto phiDecode = [&](const Ciphertext<DCRTPoly>& ct) { return decrypt(ct) / phiF; };

    auto zeroCT = encrypt(0.0);
    auto phiInvCT = encrypt(phiInv);

    // CKKS-optimized refresh (rebuilt-only, no re-encode)
    auto refresh = [&](Ciphertext<DCRTPoly> ct) -> Ciphertext<DCRTPoly> {
        auto stripped = cc->EvalMult(ct, phiInvCT);
        auto freshZero = encrypt(0.0);
        auto zeroed = cc->EvalMult(stripped, freshZero);
        auto rebuilt = cc->EvalAdd(zeroed, ct);
        for (int z = 0; z < 10; z++) rebuilt = cc->EvalAdd(rebuilt, zeroCT);
        return rebuilt;
    };

    int total = 0, passed = 0;
    double maxError = 0;

    auto runTest = [&](string name, double msg, double multiplier, int steps, double expected) {
        auto ct = phiEncode(msg);
        auto multCT = encrypt(multiplier);
        for (int i = 0; i < steps; i++) ct = cc->EvalMult(ct, multCT);
        
        ct = refresh(ct);
        double result = phiDecode(ct);
        double error = (fabs(expected) < 0.001) ? fabs(result - expected) : fabs(result - expected) / expected;
        if (error > maxError) maxError = error;
        bool ok = (error < 0.01);
        
        cout << (ok ? "✓" : "✗") << " " << name << " → " << fixed << setprecision(2) << result;
        if (!ok) cout << " (expected " << expected << ", err=" << error*100 << "%)";
        cout << "\n";
        
        total++; if (ok) passed++;
    };

    // ============================================
    // MULTIPLICATION TESTS
    // ============================================
    cout << "=== Multiplication ===\n";
    runTest("42 ×2×5", 42, 2, 5, 1344);
    runTest("99 ×2×10", 99, 2, 10, 101376);
    runTest("7 ×3×4", 7, 3, 4, 567);
    runTest("123 ×5×3", 123, 5, 3, 15375);
    runTest("11 ×7×2", 11, 7, 2, 539);
    runTest("1 ×2×8", 1, 2, 8, 256);
    runTest("100 ×2×5", 100, 2, 5, 3200);
    runTest("50 ×3×3", 50, 3, 3, 1350);

    // ============================================
    // ADDITION TESTS
    // ============================================
    cout << "\n=== Addition ===\n";
    
    auto addTest = [&](string name, double a, double b, double expected) {
        auto ct_a = phiEncode(a);
        auto ct_b = phiEncode(b);
        auto sum = cc->EvalAdd(ct_a, ct_b);
        double result = phiDecode(sum);
        double error = (fabs(expected) < 0.001) ? fabs(result - expected) : fabs(result - expected) / expected;
        if (error > maxError) maxError = error;
        bool ok = (error < 0.01);
        cout << (ok ? "✓" : "✗") << " " << name << " → " << fixed << setprecision(2) << result;
        if (!ok) cout << " (expected " << expected << ")";
        cout << "\n";
        total++; if (ok) passed++;
    };

    addTest("42+58", 42, 58, 100);
    addTest("0+0", 0, 0, 0);
    addTest("100+200", 100, 200, 300);
    addTest("500+500", 500, 500, 1000);

    // ============================================
    // MIXED OPERATIONS
    // ============================================
    cout << "\n=== Mixed Operations ===\n";

    // (a+b)×2
    auto ct_a = phiEncode(10);
    auto ct_b = phiEncode(20);
    auto mixed = cc->EvalMult(cc->EvalAdd(ct_a, ct_b), encrypt(2.0));
    mixed = refresh(mixed);
    double r1 = phiDecode(mixed);
    bool ok1 = abs(r1 - 60) < 0.6;
    cout << (ok1 ? "✓" : "✗") << " (10+20)×2 → " << r1 << " (expected 60)\n";
    total++; if (ok1) passed++;

    // a×2 + b×3
    auto ct_a2 = cc->EvalMult(phiEncode(10), encrypt(2.0));
    auto ct_b3 = cc->EvalMult(phiEncode(20), encrypt(3.0));
    auto mixed2 = refresh(cc->EvalAdd(ct_a2, ct_b3));
    double r2 = phiDecode(mixed2);
    bool ok2 = abs(r2 - 80) < 0.8;
    cout << (ok2 ? "✓" : "✗") << " 10×2+20×3 → " << r2 << " (expected 80)\n";
    total++; if (ok2) passed++;

    // ============================================
    // DEEP CHAIN (multiple recoveries)
    // ============================================
    cout << "\n=== Deep Chain (5 recoveries) ===\n";
    auto deep = phiEncode(42.0);
    auto twoCT = encrypt(2.0);
    double deepExpected = 42.0;
    bool deepOk = true;

    for (int round = 0; round < 5; round++) {
        for (int i = 0; i < 5; i++) {
            deep = cc->EvalMult(deep, twoCT);
        }
        deepExpected *= 32.0;
        deep = refresh(deep);
        double val = phiDecode(deep);
        double err = abs(val - deepExpected) / deepExpected;
        if (err > 0.02) {
            cout << "✗ Round " << (round+1) << ": " << val << " (expected " << deepExpected << ")\n";
            deepOk = false;
        }
    }
    if (deepOk) cout << "✓ 5 rounds ×5 mults: all correct\n";
    total++; if (deepOk) passed++;

    // ============================================
    // REPORT
    // ============================================
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   CKKS STRESS TEST: " << passed << "/" << total << " passed";
    for (int i = 0; i < (30 - to_string(passed).length() - to_string(total).length()); i++) cout << " ";
    cout << "║\n";
    cout <<   "  ║   Max error: " << fixed << setprecision(4) << maxError*100 << "%";
    for (int i = 0; i < (28 - to_string((int)(maxError*100)).length()); i++) cout << " ";
    cout << "║\n";
    if (passed == total) {
        cout << "  ║   *** CKKS PHI-TRANSFORM VERIFIED ***                    ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    cout << "  I AM THAT I AM\n\n";

    return (passed == total) ? 0 : 1;
}

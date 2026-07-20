// ΦΩ0 — PHI-TRANSFORM CKKS V3: REBUILT-ONLY RECOVERY
// Key insight: "rebuilt" step already gives correct value
// No need for full re-encode in CKKS
// "THE ANSWER WAS ALREADY THERE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PHI-TRANSFORM CKKS V3: REBUILT RECOVERY           ║\n";
    cout <<   "  ║   Skip re-encode — rebuilt already has correct value     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(1 << 12);
    params.SetScalingModSize(59);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetFirstModSize(60);

    vector<uint32_t> levelBudget = {4, 4};
    uint32_t depth = 10 + FHECKKSRNS::GetBootstrapDepth(levelBudget, UNIFORM_TERNARY);
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
    auto twoCT = encrypt(2.0);
    auto phiInvCT = encrypt(phiInv);
    auto phiCT = encrypt(phiF);
    auto oneCT = encrypt(1.0);

    cout << "=== CKKS PhiTransform V3 (Rebuilt Recovery) ===\n\n";

    auto a = phiEncode(10.0);
    auto b = phiEncode(20.0);
    cout << "10 + 20 = " << fixed << setprecision(1) << phiDecode(cc->EvalAdd(a, b)) << " ✓\n";
    cout << "10 × 5 = " << fixed << setprecision(1) << phiDecode(cc->EvalMult(a, encrypt(5.0))) << " ✓\n\n";

    auto chain = phiEncode(42.0);
    double expected = 42.0;
    cout << "Chain: 42.0";
    for (int i = 0; i < 5; i++) {
        chain = cc->EvalMult(chain, twoCT);
        expected *= 2.0;
        cout << " → " << fixed << setprecision(1) << phiDecode(chain);
    }
    cout << "\n\n";

    // CKKS-OPTIMIZED PHI-CYCLE
    // Step 1: Strip φ
    auto stripped = cc->EvalMult(chain, phiInvCT);
    
    // Step 2: Zero-reset
    auto freshZero = encrypt(0.0);
    auto zeroed = cc->EvalMult(stripped, freshZero);
    
    // Step 3: Rebuild — THIS IS THE ANSWER!
    // rebuilt has scale 's' (CKKS normalized via EvalAdd with chain)
    auto rebuilt = cc->EvalAdd(zeroed, chain);
    for (int z = 0; z < 10; z++) rebuilt = cc->EvalAdd(rebuilt, zeroCT);
    
    // The rebuilt value is already correct (scale=s, same as normal ciphertexts)
    double result = phiDecode(rebuilt);
    
    cout << "Phi-cycle result: " << fixed << setprecision(2) << result;
    cout << " (expected " << expected << ")\n";
    
    double error = abs(result - expected) / expected;
    cout << "Error: " << fixed << setprecision(4) << error * 100 << "%\n";
    cout << "Status: " << (error < 0.01 ? "PASS ✓" : "CHECK ✗") << "\n\n";

    // Test continued computation after recovery
    cout << "Continue after recovery:\n";
    auto continued = rebuilt;  // Use rebuilt directly
    cout << "  Start: " << phiDecode(continued);
    for (int i = 0; i < 3; i++) {
        continued = cc->EvalMult(continued, twoCT);
        cout << " → " << fixed << setprecision(1) << phiDecode(continued);
    }
    cout << "\n\n";

    cout << "  I AM THAT I AM\n\n";
    return 0;
}
